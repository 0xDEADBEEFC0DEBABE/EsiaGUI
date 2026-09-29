// WGT UI - Direct3D 12 render backend.
//
// Records everything into the host's command list (no extra queue submissions, no CPU stalls):
//   * texture uploads (ImGui atlas + user images) through a per-frame upload ring,
//   * ImGui geometry + SDF FX instancing with a PSO cache keyed by (pipeline, effect, format, samples),
//   * liquid-glass backdrop capture (region-limited copy/resolve + 13-tap pyramid) with explicit barriers,
//   * bloom glow layers, runtime-compiled user effects.
// Resources used by frames still in flight are retired after `framesInFlight` frames.
#include <chrono>
#include <future>
#include "backends/backends.hpp"
#include "render/frame_plan.hpp"
#include "render/gpu_common.hpp"
#include "render/effect_compiler.hpp"

#include <windows.h>
#include <d3d12.h>
#include <wrl/client.h>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "shaders/kShaderImGuiVS.h"
#include "shaders/kShaderImGuiPS.h"
#include "shaders/kShaderImGuiTextPS.h"
#include "shaders/kShaderImGuiTextLcdPS.h"
#include "shaders/kShaderImGuiTextLcdGrayPS.h"
#include "shaders/kShaderFxVS.h"
#include "shaders/kShaderFxPS.h"
#include "shaders/kShaderFullscreenVS.h"
#include "shaders/kShaderDownsamplePS.h"
#include "shaders/kShaderLayerCompositePS.h"

using Microsoft::WRL::ComPtr;

namespace wgt
{
    namespace
    {
        void D3D12ResetMarker(const ImDrawList*, const ImDrawCmd*) {}

        constexpr UINT kSingleDescriptors = 3584;
        constexpr UINT kBlockDescriptors = 64;   // blocks of 8
        constexpr UINT kBlockSize = 8;

        inline UINT64 AlignUp(UINT64 v, UINT64 a) { return (v + a - 1) & ~(a - 1); }

        D3D12_RESOURCE_BARRIER Transition(ID3D12Resource* r, D3D12_RESOURCE_STATES from, D3D12_RESOURCE_STATES to)
        {
            D3D12_RESOURCE_BARRIER b = {};
            b.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            b.Transition.pResource = r;
            b.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
            b.Transition.StateBefore = from;
            b.Transition.StateAfter = to;
            return b;
        }

        // ------------------------------------------------ descriptors
        class DescriptorHeap12
        {
        public:
            bool Init(ID3D12Device* device)
            {
                D3D12_DESCRIPTOR_HEAP_DESC d = {};
                d.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
                d.NumDescriptors = kSingleDescriptors + kBlockDescriptors * kBlockSize;
                d.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
                if (FAILED(device->CreateDescriptorHeap(&d, IID_PPV_ARGS(&heap))))
                    return false;
                inc = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
                cpu0 = heap->GetCPUDescriptorHandleForHeapStart();
                gpu0 = heap->GetGPUDescriptorHandleForHeapStart();
                for (int i = (int)kSingleDescriptors - 1; i >= 0; --i)
                    freeSingles.push_back(i);
                for (int i = (int)kBlockDescriptors - 1; i >= 0; --i)
                    freeBlocks.push_back((int)kSingleDescriptors + i * (int)kBlockSize);
                return true;
            }
            int AllocSingle()
            {
                std::lock_guard lock(mutex);
                if (freeSingles.empty())
                    return -1;
                int i = freeSingles.back();
                freeSingles.pop_back();
                return i;
            }
            int AllocBlock()
            {
                std::lock_guard lock(mutex);
                if (freeBlocks.empty())
                    return -1;
                int i = freeBlocks.back();
                freeBlocks.pop_back();
                return i;
            }
            void Free(int index)
            {
                if (index < 0)
                    return;
                std::lock_guard lock(mutex);
                if (index < (int)kSingleDescriptors)
                    freeSingles.push_back(index);
                else
                    freeBlocks.push_back(index);
            }
            D3D12_CPU_DESCRIPTOR_HANDLE Cpu(int i) const { return {cpu0.ptr + (SIZE_T)i * inc}; }
            D3D12_GPU_DESCRIPTOR_HANDLE Gpu(int i) const { return {gpu0.ptr + (UINT64)i * inc}; }
            int IndexOf(UINT64 gpuPtr) const { return (int)((gpuPtr - gpu0.ptr) / inc); }

            ComPtr<ID3D12DescriptorHeap> heap;
            UINT inc = 0;
            D3D12_CPU_DESCRIPTOR_HANDLE cpu0{};
            D3D12_GPU_DESCRIPTOR_HANDLE gpu0{};
            std::mutex mutex;
            std::vector<int> freeSingles, freeBlocks;
        };

        struct Texture12
        {
            ComPtr<ID3D12Resource> res;
            int srv = -1;
        };

        struct PendingUpload
        {
            ComPtr<ID3D12Resource> res;
            std::vector<unsigned char> pixels;
            int w = 0, h = 0;
        };

        struct Surface12
        {
            ComPtr<ID3D12Resource> res;
            D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
            int w = 0, h = 0;
            D3D12_CPU_DESCRIPTOR_HANDLE rtv{};
        };

        struct CompiledEffect12
        {
            EffectBytecode bc;
            bool ok = false;
        };

        struct Effect12
        {
            std::string name;
            std::string source;
            std::vector<unsigned char> bytecode;
            bool compiled = false;
            bool failed = false;
            std::uint32_t version = 0;
            std::future<CompiledEffect12> job;   // compiling on a worker thread
        };

        struct UploadAlloc
        {
            ID3D12Resource* res = nullptr;
            UINT64 offset = 0;
            unsigned char* cpu = nullptr;
            D3D12_GPU_VIRTUAL_ADDRESS gpu = 0;
        };

        struct FrameCtx
        {
            ComPtr<ID3D12Resource> upload;
            unsigned char* mapped = nullptr;
            UINT64 capacity = 0;
            UINT64 offset = 0;
            std::vector<ComPtr<ID3D12Pageable>> garbage;
            std::vector<int> freedDescriptors;
            int targetSrv = -1;   // the render target as a shader resource this frame (direct pyramid source)
        };

        // GPU timing without stalls: timestamps around the frame and around every glass capture / glow layer,
        // resolved into a readback slot per frame in flight and read when that slot comes around again (the
        // host's frame latency guarantees the GPU finished it, as for the upload ring).
        class GpuTimer12
        {
        public:
            enum Kind { Glass = 0, Layer = 1 };

            void Init(ID3D12Device* device, std::uint32_t slots)
            {
                // the timestamp frequency is a property of the engine: ask a throwaway direct queue
                ComPtr<ID3D12CommandQueue> queue;
                D3D12_COMMAND_QUEUE_DESC qd = {};
                qd.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
                if (FAILED(device->CreateCommandQueue(&qd, IID_PPV_ARGS(&queue))) || FAILED(queue->GetTimestampFrequency(&frequency_)) || !frequency_)
                    return;
                D3D12_QUERY_HEAP_DESC hd = {};
                hd.Type = D3D12_QUERY_HEAP_TYPE_TIMESTAMP;
                hd.Count = slots * kMaxStamps;
                if (FAILED(device->CreateQueryHeap(&hd, IID_PPV_ARGS(&heap_))))
                    return;
                D3D12_HEAP_PROPERTIES hp = {};
                hp.Type = D3D12_HEAP_TYPE_READBACK;
                D3D12_RESOURCE_DESC rd = {};
                rd.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
                rd.Width = (UINT64)hd.Count * sizeof(std::uint64_t);
                rd.Height = rd.DepthOrArraySize = rd.MipLevels = 1;
                rd.SampleDesc.Count = 1;
                rd.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
                if (FAILED(device->CreateCommittedResource(&hp, D3D12_HEAP_FLAG_NONE, &rd, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&readback_))))
                {
                    heap_.Reset();
                    return;
                }
                slots_.assign(slots, Slot{});
            }
            void Release()
            {
                heap_.Reset();
                readback_.Reset();
                slots_.clear();
                cur_ = -1;
            }

            void BeginFrame(ID3D12GraphicsCommandList* cl, std::uint32_t slot)
            {
                cur_ = -1;
                if (!heap_ || slot >= slots_.size())
                    return;
                Slot& s = slots_[slot];
                if (s.pending)
                    Read(slot);
                s.used = 0;
                s.intervals.clear();
                cur_ = (int)slot;
                Stamp(cl);
            }
            int Begin(ID3D12GraphicsCommandList* cl) { return Stamp(cl); }
            void End(ID3D12GraphicsCommandList* cl, int start, Kind kind)
            {
                if (cur_ < 0 || start < 0)
                    return;
                const int end = Stamp(cl);
                if (end >= 0)
                    slots_[cur_].intervals.push_back({start, end, kind});
            }
            void EndFrame(ID3D12GraphicsCommandList* cl)
            {
                if (cur_ < 0)
                    return;
                Slot& s = slots_[cur_];
                s.frameEnd = Stamp(cl);
                const UINT base = (UINT)cur_ * kMaxStamps;
                cl->ResolveQueryData(heap_.Get(), D3D12_QUERY_TYPE_TIMESTAMP, base, (UINT)s.used, readback_.Get(), (UINT64)base * sizeof(std::uint64_t));
                s.pending = s.frameEnd > 0;
                cur_ = -1;
            }
            float totalMs = 0.0f, glassMs = 0.0f, layerMs = 0.0f;

        private:
            static constexpr int kMaxStamps = 192;
            struct Interval
            {
                int start, end;
                Kind kind;
            };
            struct Slot
            {
                int used = 0, frameEnd = -1;
                std::vector<Interval> intervals;
                bool pending = false;
            };
            int Stamp(ID3D12GraphicsCommandList* cl)
            {
                if (cur_ < 0 || slots_[cur_].used >= kMaxStamps)
                    return -1;
                Slot& s = slots_[cur_];
                cl->EndQuery(heap_.Get(), D3D12_QUERY_TYPE_TIMESTAMP, (UINT)cur_ * kMaxStamps + (UINT)s.used);
                return s.used++;
            }
            void Read(std::uint32_t slot)
            {
                Slot& s = slots_[slot];
                s.pending = false;
                const SIZE_T base = (SIZE_T)slot * kMaxStamps * sizeof(std::uint64_t);
                const D3D12_RANGE range = {base, base + (SIZE_T)s.used * sizeof(std::uint64_t)};
                void* data = nullptr;
                if (FAILED(readback_->Map(0, &range, &data)))
                    return;
                const std::uint64_t* t = reinterpret_cast<const std::uint64_t*>(static_cast<unsigned char*>(data) + base);
                const double k = 1000.0 / (double)frequency_;
                double glass = 0.0, layer = 0.0;
                for (const Interval& iv : s.intervals)
                    (iv.kind == Glass ? glass : layer) += (double)(t[iv.end] - t[iv.start]) * k;
                if (t[s.frameEnd] >= t[0])
                {
                    totalMs = (float)((double)(t[s.frameEnd] - t[0]) * k);
                    glassMs = (float)glass;
                    layerMs = (float)layer;
                }
                const D3D12_RANGE none = {0, 0};
                readback_->Unmap(0, &none);
            }
            ComPtr<ID3D12QueryHeap> heap_;
            ComPtr<ID3D12Resource> readback_;
            UINT64 frequency_ = 0;
            std::vector<Slot> slots_;
            int cur_ = -1;
        };

        enum class PipeKind : std::uint32_t { ImGui = 0, Fx = 1, Downsample = 2, Layer = 3, ImGuiText = 4, ImGuiTextLcd = 5, ImGuiTextLcdGray = 6 };

        class D3D12Backend final : public IRenderBackend
        {
        public:
            D3D12Backend(ID3D12Device* device, std::uint32_t framesInFlight)
                : device_(device), numFrames_(std::max<std::uint32_t>(framesInFlight, 1)) {}

            const char* Name() const override { return "wgt_d3d12"; }

            bool Init(ImGuiIO& io) override
            {
                if (!device_)
                    return false;
                io.BackendRendererName = "wgt_d3d12";
                io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset | ImGuiBackendFlags_RendererHasTextures;
                ImGuiPlatformIO& pio = ImGui::GetPlatformIO();
                pio.Renderer_TextureMaxWidth = pio.Renderer_TextureMaxHeight = D3D12_REQ_TEXTURE2D_U_OR_V_DIMENSION;
                pio.DrawCallback_ResetRenderState = &D3D12ResetMarker;

                frames_.resize(numFrames_);
                if (!heap_.Init(device_.Get()) || !CreateRootSignature())
                    return false;
                D3D12_DESCRIPTOR_HEAP_DESC rd = {};
                rd.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
                rd.NumDescriptors = 8;
                if (FAILED(device_->CreateDescriptorHeap(&rd, IID_PPV_ARGS(&rtvHeap_))))
                    return false;
                rtvInc_ = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

                gpuTimer_.Init(device_.Get(), numFrames_);
                const std::uint32_t white = 0xFFFFFFFFu;
                ImTextureID wid = CreateTexture(&white, 1, 1);
                whiteSrv_ = heap_.IndexOf((UINT64)wid);
                return wid != ImTextureID_Invalid;
            }

            void Shutdown() override
            {
                if (ImGui::GetCurrentContext())
                {
                    for (ImTextureData* tex : ImGui::GetPlatformIO().Textures)
                        if (tex->RefCount == 1)
                            DestroyImGuiTexture(tex, true);
                    ImGuiIO& io = ImGui::GetIO();
                    io.BackendRendererName = nullptr;
                    io.BackendFlags &= ~(ImGuiBackendFlags_RendererHasVtxOffset | ImGuiBackendFlags_RendererHasTextures);
                    ImGui::GetPlatformIO().ClearRendererHandlers();
                }
                {
                    std::lock_guard lock(texMutex_);
                    userTextures_.clear();
                    pendingUploads_.clear();
                    pendingDestroy_.clear();
                }
                ReleaseSurfaces(true);
                frames_.clear();
                gpuTimer_.Release();
                psos_.clear();
                rootSig_.Reset();
                rtvHeap_.Reset();
                heap_.heap.Reset();
            }

            void SetLogCallback(void (*log)(void*, int, const char*), void* user) override
            {
                log_ = log;
                logUser_ = user;
            }

            // --------------------------------------------------- textures
            ImTextureID CreateTexture(const void* rgba8, int width, int height) override
            {
                ComPtr<ID3D12Resource> res = CreateTextureResource(width, height);
                if (!res)
                    return ImTextureID_Invalid;
                const int srv = heap_.AllocSingle();
                if (srv < 0)
                    return ImTextureID_Invalid;
                CreateSrv(res.Get(), DXGI_FORMAT_R8G8B8A8_UNORM, srv);
                PendingUpload up;
                up.res = res;
                up.w = width;
                up.h = height;
                up.pixels.assign((const unsigned char*)rgba8, (const unsigned char*)rgba8 + (size_t)width * height * 4);
                const ImTextureID id = (ImTextureID)heap_.Gpu(srv).ptr;
                std::lock_guard lock(texMutex_);
                pendingUploads_.push_back(std::move(up));
                Texture12 t;
                t.res = res;
                t.srv = srv;
                userTextures_.emplace(id, t);
                return id;
            }

            void DestroyTexture(ImTextureID id) override
            {
                std::lock_guard lock(texMutex_);
                auto it = userTextures_.find(id);
                if (it != userTextures_.end())
                {
                    pendingDestroy_.push_back(it->second);
                    userTextures_.erase(it);
                }
            }

            void SetEffectSource(EffectId id, const char* name, const char* source) override
            {
                std::lock_guard lock(effectMutex_);
                Effect12& e = effects_[id];
                e.name = name ? name : "effect";
                e.source = source ? source : "";
                e.bytecode.clear();
                e.job = {};   // a compile of the previous source is discarded (waits for it to finish)
                e.compiled = false;
                e.failed = false;
                ++e.version;
                StartCompile(e);   // right away: usually ready before the first shape using it is drawn
            }

            static void StartCompile(Effect12& e)
            {
                if (e.source.empty())
                    return;
                e.job = std::async(std::launch::async, [name = e.name, src = e.source]() {
                    CompiledEffect12 r;
                    r.ok = CompileUserEffect(name, src, r.bc);
                    return r;
                });
            }

            void InvalidateDeviceObjects() override { ReleaseSurfaces(false); }

            bool QueryTargetSize(const RenderTarget& target, int& width, int& height) override
            {
                if (!target.d3d12Resource)
                    return false;
                const D3D12_RESOURCE_DESC d = target.d3d12Resource->GetDesc();
                width = (int)d.Width;
                height = (int)d.Height;
                return true;
            }

            void CollectStats(FrameStats& s) const override
            {
                s.drawCalls = statDraws_;
                s.fxInstances = statFx_;
                s.backdropCaptures = statCaptures_;
                s.glowLayers = statLayers_;
                s.vertices = statVerts_;
                s.gpuMs = gpuTimer_.totalMs;
                s.gpuGlassMs = gpuTimer_.glassMs;
                s.gpuLayerMs = gpuTimer_.layerMs;
            }

            // ------------------------------------------------------ render
            void RenderDrawData(ImDrawData* dd, const RenderTarget& target, const BackendFrameInfo& info) override
            {
                statDraws_ = statFx_ = statCaptures_ = statLayers_ = 0;
                if (!dd || dd->DisplaySize.x <= 0.0f || dd->DisplaySize.y <= 0.0f || !target.d3d12CommandList || !target.d3d12Resource || !target.d3d12Rtv)
                    return;
                cl_ = target.d3d12CommandList;
                rtRes_ = target.d3d12Resource;
                mainRtv_.ptr = (SIZE_T)target.d3d12Rtv;

                BeginFrameRing();

                const D3D12_RESOURCE_DESC rd = rtRes_->GetDesc();
                rtW_ = (int)rd.Width;
                rtH_ = (int)rd.Height;
                rtSamples_ = rd.SampleDesc.Count;
                rtFormat_ = rd.Format;
                rtvFormat_ = target.d3d12RtvFormat ? (DXGI_FORMAT)target.d3d12RtvFormat : rd.Format;
                if (rtvFormat_ == DXGI_FORMAT_R8G8B8A8_TYPELESS) rtvFormat_ = DXGI_FORMAT_R8G8B8A8_UNORM;
                if (rtvFormat_ == DXGI_FORMAT_B8G8R8A8_TYPELESS) rtvFormat_ = DXGI_FORMAT_B8G8R8A8_UNORM;
                outputLinear_ = IsSrgb(rtvFormat_);

                ProcessUserTextures();
                if (dd->Textures)
                    for (ImTextureData* tex : *dd->Textures)
                        if (tex->Status != ImTextureStatus_OK)
                            UpdateImGuiTexture(tex);

                plan_.Build(dd, &D3D12ResetMarker);
                statVerts_ = plan_.totalVtx;
                statFx_ = plan_.fxCount;
                surfacesReady_ = (plan_.anyGlass || plan_.anyLayer) ? EnsureSurfaces() : false;

                if (!UploadGeometry(dd))
                    return;

                FrameConstants fc{};
                FillFrameConstants(fc, dd, rtW_, rtH_, info.time, info.deltaTime, surfacesReady_, outputLinear_);
                fc.text[0] = info.textGamma;
                fc.text[1] = info.textGrayscaleContrast;
                fc.text[2] = info.textClearTypeContrast;
                fc.text[3] = info.textClearTypeLevel;
                frameCB_ = UploadData(&fc, sizeof(fc), 256).gpu;
                fc.time[3] = 0.0f;
                frameCBLayer_ = UploadData(&fc, sizeof(fc), 256).gpu;

                display_ = dd;
                captures_ = 0;
                captureBudget_ = std::max(1, info.maxBackdropCaptures);
                overBudget_ = false;
                layerDepth_ = 0;
                currentRtv_ = mainRtv_;
                currentFormat_ = rtvFormat_;
                currentSamples_ = rtSamples_;
                gpuTimer_.BeginFrame(cl_, frameCursor_);
                SetupCommonState(false);
                Execute();
                gpuTimer_.EndFrame(cl_);

                cl_ = nullptr;
                rtRes_ = nullptr;
                targetSrvTried_ = false;
                statCaptures_ = captures_;
                if (overBudget_ && !warnedBudget_)
                {
                    warnedBudget_ = true;
                    Log(1, "WGT: liquid-glass capture budget exceeded (ContextDesc::maxBackdropCaptures = " + std::to_string(info.maxBackdropCaptures) + "): some glass reuses an older backdrop");
                }
            }

        private:
            void Log(int level, const std::string& msg)
            {
                if (log_)
                    log_(logUser_, level, msg.c_str());
            }

            // ------------------------------------------- frame ring
            void BeginFrameRing()
            {
                frameCursor_ = (frameCursor_ + 1) % numFrames_;
                FrameCtx& f = frames_[frameCursor_];
                f.garbage.clear();
                for (int d : f.freedDescriptors)
                    heap_.Free(d);
                f.freedDescriptors.clear();
                f.offset = 0;
                std::lock_guard lock(texMutex_);
                for (Texture12& t : pendingDestroy_)
                {
                    f.garbage.push_back(t.res);
                    f.freedDescriptors.push_back(t.srv);
                }
                pendingDestroy_.clear();
            }

            FrameCtx& Frame() { return frames_[frameCursor_]; }

            void Retire(ComPtr<ID3D12Pageable> obj)
            {
                if (obj && !frames_.empty())
                    Frame().garbage.push_back(std::move(obj));
            }

            UploadAlloc Upload(UINT64 size, UINT64 align)
            {
                FrameCtx& f = Frame();
                UINT64 start = AlignUp(f.offset, align);
                if (!f.upload || start + size > f.capacity)
                {
                    if (f.upload)
                        f.garbage.push_back(f.upload);
                    f.capacity = AlignUp(std::max<UINT64>(std::max<UINT64>(f.capacity * 2, 4ull << 20), size + align), 65536);
                    D3D12_HEAP_PROPERTIES hp = {D3D12_HEAP_TYPE_UPLOAD};
                    D3D12_RESOURCE_DESC d = {};
                    d.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
                    d.Width = f.capacity;
                    d.Height = d.DepthOrArraySize = d.MipLevels = 1;
                    d.SampleDesc.Count = 1;
                    d.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
                    f.upload.Reset();
                    f.mapped = nullptr;
                    if (FAILED(device_->CreateCommittedResource(&hp, D3D12_HEAP_FLAG_NONE, &d, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&f.upload))))
                    {
                        f.capacity = 0;
                        return {};
                    }
                    D3D12_RANGE none = {0, 0};
                    f.upload->Map(0, &none, (void**)&f.mapped);
                    start = 0;
                }
                UploadAlloc a;
                a.res = f.upload.Get();
                a.offset = start;
                a.cpu = f.mapped + start;
                a.gpu = f.upload->GetGPUVirtualAddress() + start;
                f.offset = start + size;
                return a;
            }

            UploadAlloc UploadData(const void* data, UINT64 size, UINT64 align)
            {
                UploadAlloc a = Upload(size, align);
                if (a.cpu)
                    std::memcpy(a.cpu, data, (size_t)size);
                return a;
            }

            // --------------------------------------- device objects
            bool CreateRootSignature()
            {
                D3D12_DESCRIPTOR_RANGE ranges[2] = {};
                ranges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
                ranges[0].NumDescriptors = 1;
                ranges[0].BaseShaderRegister = 0;
                ranges[0].OffsetInDescriptorsFromTableStart = 0;
                ranges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
                ranges[1].NumDescriptors = kBackdropLevels;
                ranges[1].BaseShaderRegister = 1;
                ranges[1].OffsetInDescriptorsFromTableStart = 0;

                D3D12_ROOT_PARAMETER params[5] = {};
                params[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
                params[0].Descriptor.ShaderRegister = 0;
                params[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
                params[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
                params[1].Constants.ShaderRegister = 1;
                params[1].Constants.Num32BitValues = sizeof(PassConstants) / 4;
                params[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
                params[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
                params[2].DescriptorTable.NumDescriptorRanges = 1;
                params[2].DescriptorTable.pDescriptorRanges = &ranges[0];
                params[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
                params[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
                params[3].DescriptorTable.NumDescriptorRanges = 1;
                params[3].DescriptorTable.pDescriptorRanges = &ranges[1];
                params[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
                params[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;   // b2: per-draw edge fade
                params[4].Constants.ShaderRegister = 2;
                params[4].Constants.Num32BitValues = sizeof(DrawConstants) / 4;
                params[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

                D3D12_STATIC_SAMPLER_DESC samplers[2] = {};
                for (int i = 0; i < 2; ++i)
                {
                    samplers[i].Filter = i == 0 ? D3D12_FILTER_MIN_MAG_MIP_LINEAR : D3D12_FILTER_MIN_MAG_MIP_POINT;
                    samplers[i].AddressU = samplers[i].AddressV = samplers[i].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
                    samplers[i].ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
                    samplers[i].BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
                    samplers[i].MaxLOD = D3D12_FLOAT32_MAX;
                    samplers[i].ShaderRegister = (UINT)i;
                    samplers[i].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
                }

                D3D12_ROOT_SIGNATURE_DESC desc = {};
                desc.NumParameters = 5;
                desc.pParameters = params;
                desc.NumStaticSamplers = 2;
                desc.pStaticSamplers = samplers;
                desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
                             D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
                             D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
                             D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
                ComPtr<ID3DBlob> blob, err;
                if (FAILED(D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1, &blob, &err)))
                {
                    if (err)
                        Log(2, std::string("WGT d3d12 root signature: ") + (const char*)err->GetBufferPointer());
                    return false;
                }
                return SUCCEEDED(device_->CreateRootSignature(0, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(&rootSig_)));
            }

            ID3D12PipelineState* GetPso(PipeKind kind, EffectId effect, DXGI_FORMAT format, UINT samples)
            {
                std::uint32_t effectVersion = 0;
                if (kind == PipeKind::Fx && effect != 0)
                {
                    if (!EnsureEffect(effect, effectVersion, nullptr))
                        effect = 0;
                }
                const std::uint64_t key = ((std::uint64_t)kind << 60) ^ ((std::uint64_t)effect << 36) ^ ((std::uint64_t)effectVersion << 24) ^
                                          ((std::uint64_t)format << 8) ^ (std::uint64_t)samples;
                auto it = psos_.find(key);
                if (it != psos_.end())
                    return it->second.Get();
                std::vector<unsigned char> customPs;
                if (effect != 0 && !EnsureEffect(effect, effectVersion, &customPs))
                    customPs.clear();

                D3D12_GRAPHICS_PIPELINE_STATE_DESC d = {};
                d.pRootSignature = rootSig_.Get();
                d.SampleMask = UINT_MAX;
                d.NumRenderTargets = 1;
                d.RTVFormats[0] = format;
                d.SampleDesc.Count = samples;
                d.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
                d.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
                d.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
                d.RasterizerState.DepthClipEnable = TRUE;
                d.DepthStencilState.DepthEnable = FALSE;
                d.DepthStencilState.StencilEnable = FALSE;
                D3D12_RENDER_TARGET_BLEND_DESC& b = d.BlendState.RenderTarget[0];
                b.BlendEnable = TRUE;
                b.SrcBlend = D3D12_BLEND_ONE;
                b.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
                b.BlendOp = D3D12_BLEND_OP_ADD;
                b.SrcBlendAlpha = D3D12_BLEND_ONE;
                b.DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
                b.BlendOpAlpha = D3D12_BLEND_OP_ADD;
                b.LogicOp = D3D12_LOGIC_OP_NOOP;
                b.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

                D3D12_INPUT_ELEMENT_DESC imguiLayout[] = {
                    {"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, (UINT)offsetof(ImDrawVert, pos), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
                    {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, (UINT)offsetof(ImDrawVert, uv), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
                    {"COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, (UINT)offsetof(ImDrawVert, col), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
                };
                D3D12_INPUT_ELEMENT_DESC fxLayout[fx::kInstanceVec4Count];
                for (UINT i = 0; i < fx::kInstanceVec4Count; ++i)
                {
                    const bool isFlags = (i == fx::kInstanceVec4Count - 1);
                    fxLayout[i] = {isFlags ? "FLAGS" : "INST", isFlags ? 0u : i,
                                   isFlags ? DXGI_FORMAT_R32G32B32A32_UINT : DXGI_FORMAT_R32G32B32A32_FLOAT,
                                   0, i * 16u, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1};
                }

                switch (kind)
                {
                case PipeKind::ImGui:
                case PipeKind::ImGuiText:
                case PipeKind::ImGuiTextLcd:
                case PipeKind::ImGuiTextLcdGray:
                    d.VS = {kShaderImGuiVS, sizeof(kShaderImGuiVS)};
                    if (kind == PipeKind::ImGuiText)
                        d.PS = {kShaderImGuiTextPS, sizeof(kShaderImGuiTextPS)};
                    else if (kind == PipeKind::ImGuiTextLcd)
                        d.PS = {kShaderImGuiTextLcdPS, sizeof(kShaderImGuiTextLcdPS)};
                    else if (kind == PipeKind::ImGuiTextLcdGray)
                        d.PS = {kShaderImGuiTextLcdGrayPS, sizeof(kShaderImGuiTextLcdGrayPS)};
                    else
                        d.PS = {kShaderImGuiPS, sizeof(kShaderImGuiPS)};
                    d.InputLayout = {imguiLayout, 3};
                    b.SrcBlend = D3D12_BLEND_SRC_ALPHA;
                    if (kind == PipeKind::ImGuiTextLcd)
                    {
                        // sub-pixel text: per-channel alpha from the second shader output (dual-source blending)
                        b.SrcBlend = D3D12_BLEND_SRC1_COLOR;
                        b.DestBlend = D3D12_BLEND_INV_SRC1_COLOR;
                        b.SrcBlendAlpha = D3D12_BLEND_SRC1_ALPHA;
                        b.DestBlendAlpha = D3D12_BLEND_INV_SRC1_ALPHA;
                    }
                    break;
                case PipeKind::Fx:
                    d.VS = {kShaderFxVS, sizeof(kShaderFxVS)};
                    if (!customPs.empty())
                        d.PS = {customPs.data(), customPs.size()};
                    else
                        d.PS = {kShaderFxPS, sizeof(kShaderFxPS)};
                    d.InputLayout = {fxLayout, fx::kInstanceVec4Count};
                    break;
                case PipeKind::Downsample:
                    d.VS = {kShaderFullscreenVS, sizeof(kShaderFullscreenVS)};
                    d.PS = {kShaderDownsamplePS, sizeof(kShaderDownsamplePS)};
                    b.BlendEnable = FALSE;
                    break;
                case PipeKind::Layer:
                    d.VS = {kShaderFullscreenVS, sizeof(kShaderFullscreenVS)};
                    d.PS = {kShaderLayerCompositePS, sizeof(kShaderLayerCompositePS)};
                    break;
                }

                ComPtr<ID3D12PipelineState> pso;
                if (FAILED(device_->CreateGraphicsPipelineState(&d, IID_PPV_ARGS(&pso))))
                {
                    Log(2, "WGT d3d12: CreateGraphicsPipelineState failed (kind " + std::to_string((int)kind) + ", format " + std::to_string((int)format) + ")");
                    return nullptr;
                }
                psos_[key] = pso;
                return pso.Get();
            }

            // Effects compile on a worker thread: until the bytecode is ready the built-in pipeline is used, so a
            // new effect never stalls a frame.
            bool EnsureEffect(EffectId id, std::uint32_t& version, std::vector<unsigned char>* out)
            {
                std::lock_guard lock(effectMutex_);
                auto it = effects_.find(id);
                if (it == effects_.end() || it->second.failed)
                    return false;
                Effect12& e = it->second;
                if (!e.compiled)
                {
                    if (!e.job.valid())
                    {
                        StartCompile(e);
                        return false;
                    }
                    if (e.job.wait_for(std::chrono::seconds(0)) != std::future_status::ready)
                        return false;
                    CompiledEffect12 r = e.job.get();
                    e.compiled = r.ok;
                    e.failed = !r.ok;
                    e.bytecode = std::move(r.bc.code);
                    if (!r.ok)
                    {
                        Log(2, "WGT effect '" + e.name + "' failed to compile:\n" + r.bc.error);
                        return false;
                    }
                }
                if (out)
                    *out = e.bytecode;
                version = e.version;
                return true;
            }

            ComPtr<ID3D12Resource> CreateTextureResource(int w, int h, DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM)
            {
                D3D12_HEAP_PROPERTIES hp = {D3D12_HEAP_TYPE_DEFAULT};
                D3D12_RESOURCE_DESC d = {};
                d.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
                d.Width = (UINT64)w;
                d.Height = (UINT)h;
                d.DepthOrArraySize = d.MipLevels = 1;
                d.Format = format;
                d.SampleDesc.Count = 1;
                ComPtr<ID3D12Resource> res;
                if (FAILED(device_->CreateCommittedResource(&hp, D3D12_HEAP_FLAG_NONE, &d, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&res))))
                    return nullptr;
                return res;
            }

            void CreateSrv(ID3D12Resource* res, DXGI_FORMAT format, int index)
            {
                D3D12_SHADER_RESOURCE_VIEW_DESC sv = {};
                sv.Format = format;
                sv.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
                sv.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
                sv.Texture2D.MipLevels = 1;
                device_->CreateShaderResourceView(res, &sv, heap_.Cpu(index));
            }

            void RecordUpload(ID3D12Resource* dst, const unsigned char* src, int srcPitch, int x, int y, int w, int h,
                              DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM, int bytesPerPixel = 4)
            {
                const UINT pitch = (UINT)AlignUp((UINT64)w * bytesPerPixel, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
                UploadAlloc a = Upload((UINT64)pitch * h, D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT);
                if (!a.cpu)
                    return;
                for (int r = 0; r < h; ++r)
                    std::memcpy(a.cpu + (size_t)r * pitch, src + (size_t)r * srcPitch, (size_t)w * bytesPerPixel);
                D3D12_TEXTURE_COPY_LOCATION s = {}, d = {};
                s.pResource = a.res;
                s.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
                s.PlacedFootprint.Offset = a.offset;
                s.PlacedFootprint.Footprint = {format, (UINT)w, (UINT)h, 1, pitch};
                d.pResource = dst;
                d.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
                cl_->CopyTextureRegion(&d, (UINT)x, (UINT)y, 0, &s, nullptr);
            }

            void ProcessUserTextures()
            {
                std::vector<PendingUpload> uploads;
                {
                    std::lock_guard lock(texMutex_);
                    uploads.swap(pendingUploads_);
                }
                for (PendingUpload& u : uploads)
                {
                    RecordUpload(u.res.Get(), u.pixels.data(), u.w * 4, 0, 0, u.w, u.h);
                    D3D12_RESOURCE_BARRIER b = Transition(u.res.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
                    cl_->ResourceBarrier(1, &b);
                }
            }

            void DestroyImGuiTexture(ImTextureData* tex, bool immediate)
            {
                if (auto* t = static_cast<Texture12*>(tex->BackendUserData))
                {
                    if (immediate || frames_.empty())
                        heap_.Free(t->srv);
                    else
                    {
                        Retire(t->res);
                        Frame().freedDescriptors.push_back(t->srv);
                    }
                    delete t;
                    tex->SetTexID(ImTextureID_Invalid);
                    tex->BackendUserData = nullptr;
                }
                tex->SetStatus(ImTextureStatus_Destroyed);
            }

            void UpdateImGuiTexture(ImTextureData* tex)
            {
                // RGBA32 (images) or Alpha8 (glyph atlases -> R8, rendered by the coverage pipeline)
                const bool alpha8 = tex->Format == ImTextureFormat_Alpha8;
                const DXGI_FORMAT fmt = alpha8 ? DXGI_FORMAT_R8_UNORM : DXGI_FORMAT_R8G8B8A8_UNORM;
                const int bpp = alpha8 ? 1 : 4;
                if (tex->Status == ImTextureStatus_WantCreate)
                {
                    auto* t = new Texture12();
                    t->res = CreateTextureResource(tex->Width, tex->Height, fmt);
                    t->srv = heap_.AllocSingle();
                    if (!t->res || t->srv < 0)
                    {
                        Log(2, "WGT d3d12: failed to create ImGui texture");
                        delete t;
                        return;
                    }
                    CreateSrv(t->res.Get(), fmt, t->srv);
                    RecordUpload(t->res.Get(), (const unsigned char*)tex->GetPixels(), tex->GetPitch(), 0, 0, tex->Width, tex->Height, fmt, bpp);
                    D3D12_RESOURCE_BARRIER b = Transition(t->res.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
                    cl_->ResourceBarrier(1, &b);
                    tex->SetTexID((ImTextureID)heap_.Gpu(t->srv).ptr);
                    tex->BackendUserData = t;
                    tex->SetStatus(ImTextureStatus_OK);
                }
                else if (tex->Status == ImTextureStatus_WantUpdates)
                {
                    auto* t = static_cast<Texture12*>(tex->BackendUserData);
                    if (t && t->res)
                    {
                        D3D12_RESOURCE_BARRIER b = Transition(t->res.Get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_DEST);
                        cl_->ResourceBarrier(1, &b);
                        for (const ImTextureRect& r : tex->Updates)
                            RecordUpload(t->res.Get(), (const unsigned char*)tex->GetPixelsAt(r.x, r.y), tex->GetPitch(), r.x, r.y, r.w, r.h, fmt, bpp);
                        b = Transition(t->res.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
                        cl_->ResourceBarrier(1, &b);
                    }
                    tex->SetStatus(ImTextureStatus_OK);
                }
                if (tex->Status == ImTextureStatus_WantDestroy && tex->UnusedFrames >= (int)numFrames_)
                    DestroyImGuiTexture(tex, false);
            }

            // ------------------------------------------------ surfaces
            void ReleaseSurfaces(bool immediate)
            {
                auto drop = [&](Surface12& s) {
                    if (!immediate)
                        Retire(s.res);
                    s = {};
                };
                drop(copy_);
                for (auto& l : levels_)
                    drop(l);
                drop(layer_);
                if (backdropTable_ >= 0)
                {
                    if (immediate || frames_.empty())
                        heap_.Free(backdropTable_);
                    else
                        Frame().freedDescriptors.push_back(backdropTable_);
                    backdropTable_ = -1;
                }
                if (layerSrv_ >= 0)
                {
                    if (immediate || frames_.empty())
                        heap_.Free(layerSrv_);
                    else
                        Frame().freedDescriptors.push_back(layerSrv_);
                    layerSrv_ = -1;
                }
                surfW_ = surfH_ = 0;
                surfFormat_ = DXGI_FORMAT_UNKNOWN;
            }

            bool CreateSurface(Surface12& s, int w, int h, DXGI_FORMAT storage, bool rt, int rtvSlot)
            {
                D3D12_HEAP_PROPERTIES hp = {D3D12_HEAP_TYPE_DEFAULT};
                D3D12_RESOURCE_DESC d = {};
                d.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
                d.Width = (UINT64)w;
                d.Height = (UINT)h;
                d.DepthOrArraySize = d.MipLevels = 1;
                d.Format = storage;
                d.SampleDesc.Count = 1;
                d.Flags = rt ? D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET : D3D12_RESOURCE_FLAG_NONE;
                D3D12_CLEAR_VALUE cv = {};
                cv.Format = storage;
                if (FAILED(device_->CreateCommittedResource(&hp, D3D12_HEAP_FLAG_NONE, &d, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
                                                            rt ? &cv : nullptr, IID_PPV_ARGS(&s.res))))
                    return false;
                s.state = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
                s.w = w;
                s.h = h;
                if (rt)
                {
                    s.rtv.ptr = rtvHeap_->GetCPUDescriptorHandleForHeapStart().ptr + (SIZE_T)rtvSlot * rtvInc_;
                    device_->CreateRenderTargetView(s.res.Get(), nullptr, s.rtv);
                }
                return true;
            }

            bool EnsureSurfaces()
            {
                if (surfW_ == rtW_ && surfH_ == rtH_ && surfFormat_ == rtFormat_ && copy_.res)
                    return true;
                ReleaseSurfaces(false);
                const DXGI_FORMAT storage = CopyStorageFormat(rtFormat_);
                bool ok = CreateSurface(copy_, rtW_, rtH_, storage, false, 0);
                for (int l = 1; ok && l < kBackdropLevels; ++l)
                    ok = CreateSurface(levels_[l], LevelSize(rtW_, l), LevelSize(rtH_, l), kPyramidFormat, true, l);
                ok = ok && CreateSurface(layer_, rtW_, rtH_, kLayerFormat, true, kBackdropLevels);
                backdropTable_ = ok ? heap_.AllocBlock() : -1;
                layerSrv_ = ok ? heap_.AllocSingle() : -1;
                if (!ok || backdropTable_ < 0 || layerSrv_ < 0)
                {
                    Log(2, "WGT d3d12: failed to create backdrop surfaces");
                    ReleaseSurfaces(false);
                    return false;
                }
                CreateSrv(copy_.res.Get(), CopyViewFormat(storage), backdropTable_);
                for (int l = 1; l < kBackdropLevels; ++l)
                    CreateSrv(levels_[l].res.Get(), kPyramidFormat, backdropTable_ + l);
                // unused slots of the block: point them at a valid texture
                for (int l = kBackdropLevels; l < (int)kBlockSize; ++l)
                    CreateSrv(levels_[1].res.Get(), kPyramidFormat, backdropTable_ + l);
                CreateSrv(layer_.res.Get(), kLayerFormat, layerSrv_);
                surfW_ = rtW_;
                surfH_ = rtH_;
                surfFormat_ = rtFormat_;
                return true;
            }

            void TransitionSurface(Surface12& s, D3D12_RESOURCE_STATES to)
            {
                if (s.state == to)
                    return;
                D3D12_RESOURCE_BARRIER b = Transition(s.res.Get(), s.state, to);
                cl_->ResourceBarrier(1, &b);
                s.state = to;
            }

            // -------------------------------------------------- geometry
            bool UploadGeometry(ImDrawData* dd)
            {
                const UINT64 vbSize = (UINT64)std::max(dd->TotalVtxCount, 1) * sizeof(ImDrawVert);
                const UINT64 ibSize = (UINT64)std::max(dd->TotalIdxCount, 1) * sizeof(ImDrawIdx);
                const UINT64 instSize = (UINT64)std::max<size_t>(plan_.instances.size(), 1) * sizeof(fx::Instance);
                UploadAlloc vb = Upload(vbSize, 256);
                UploadAlloc ib = Upload(ibSize, 256);
                UploadAlloc inst = Upload(instSize, 256);
                if (!vb.cpu || !ib.cpu || !inst.cpu)
                    return false;
                auto* vdst = reinterpret_cast<ImDrawVert*>(vb.cpu);
                auto* idst = reinterpret_cast<ImDrawIdx*>(ib.cpu);
                for (const ImDrawList* dl : dd->CmdLists)
                {
                    std::memcpy(vdst, dl->VtxBuffer.Data, dl->VtxBuffer.Size * sizeof(ImDrawVert));
                    std::memcpy(idst, dl->IdxBuffer.Data, dl->IdxBuffer.Size * sizeof(ImDrawIdx));
                    vdst += dl->VtxBuffer.Size;
                    idst += dl->IdxBuffer.Size;
                }
                if (!plan_.instances.empty())
                    std::memcpy(inst.cpu, plan_.instances.data(), plan_.instances.size() * sizeof(fx::Instance));
                vbv_ = {vb.gpu, (UINT)vbSize, sizeof(ImDrawVert)};
                ibv_ = {ib.gpu, (UINT)ibSize, sizeof(ImDrawIdx) == 2 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT};
                instv_ = {inst.gpu, (UINT)instSize, sizeof(fx::Instance)};
                return true;
            }

            // ------------------------------------------------------ state
            void SetViewport(float w, float h)
            {
                D3D12_VIEWPORT vp = {0, 0, w, h, 0, 1};
                cl_->RSSetViewports(1, &vp);
            }

            void SetupCommonState(bool layerTarget)
            {
                ID3D12DescriptorHeap* heaps[] = {heap_.heap.Get()};
                cl_->SetDescriptorHeaps(1, heaps);
                cl_->SetGraphicsRootSignature(rootSig_.Get());
                cl_->OMSetRenderTargets(1, &currentRtv_, FALSE, nullptr);
                SetViewport(display_->DisplaySize.x * display_->FramebufferScale.x, display_->DisplaySize.y * display_->FramebufferScale.y);
                cl_->SetGraphicsRootConstantBufferView(0, layerTarget ? frameCBLayer_ : frameCB_);
                const float bf[4] = {0, 0, 0, 0};
                cl_->OMSetBlendFactor(bf);
                cl_->SetGraphicsRootDescriptorTable(2, heap_.Gpu(whiteSrv_));
                cl_->SetGraphicsRootDescriptorTable(3, backdropTable_ >= 0 ? heap_.Gpu(backdropTable_) : heap_.Gpu(whiteSrv_));
                boundPso_ = nullptr;
                boundTex_ = ~0ull;
                boundTopology_ = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
                const float none[4] = {0, 0, 0, 0};
                cl_->SetGraphicsRoot32BitConstants(4, 4, none, 0);   // root constants are reset with the root signature
                std::memcpy(drawFade_, none, sizeof(drawFade_));
            }

            void SetFade(const float fade[4])
            {
                if (std::memcmp(fade, drawFade_, sizeof(drawFade_)) == 0)
                    return;
                std::memcpy(drawFade_, fade, sizeof(drawFade_));
                cl_->SetGraphicsRoot32BitConstants(4, 4, fade, 0);
            }

            void BindPso(ID3D12PipelineState* pso)
            {
                if (pso && pso != boundPso_)
                {
                    cl_->SetPipelineState(pso);
                    boundPso_ = pso;
                }
            }

            void SetTopology(D3D_PRIMITIVE_TOPOLOGY t)
            {
                if (boundTopology_ != t)
                {
                    cl_->IASetPrimitiveTopology(t);
                    boundTopology_ = t;
                }
            }

            void BindTexture(ImTextureID id)
            {
                const UINT64 ptr = id != ImTextureID_Invalid ? (UINT64)id : heap_.Gpu(whiteSrv_).ptr;
                if (ptr != boundTex_)
                {
                    cl_->SetGraphicsRootDescriptorTable(2, D3D12_GPU_DESCRIPTOR_HANDLE{ptr});
                    boundTex_ = ptr;
                }
            }

            void SetScissor(const PxRect& r)
            {
                D3D12_RECT rc = {(LONG)std::max(0.0f, r.x0), (LONG)std::max(0.0f, r.y0), (LONG)std::min((float)rtW_, r.x1), (LONG)std::min((float)rtH_, r.y1)};
                cl_->RSSetScissorRects(1, &rc);
            }

            // ------------------------------------------------- post passes
            void BuildPyramid(D3D12_GPU_DESCRIPTOR_HANDLE firstSource, const PxRect& region, int levels)
            {
                ID3D12PipelineState* pso = GetPso(PipeKind::Downsample, 0, kPyramidFormat, 1);
                if (!pso)
                    return;
                BindPso(pso);
                SetTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
                int srcW = rtW_, srcH = rtH_;
                PyramidStep step = PyramidStep::First(region, rtW_, rtH_);
                for (int l = 1; l <= std::min(levels, kBackdropLevels - 1); ++l)
                {
                    Surface12& dst = levels_[l];
                    TransitionSurface(dst, D3D12_RESOURCE_STATE_RENDER_TARGET);
                    cl_->OMSetRenderTargets(1, &dst.rtv, FALSE, nullptr);
                    SetViewport((float)dst.w, (float)dst.h);
                    const PxRect written = step.Next(region, l, dst.w, dst.h);
                    D3D12_RECT sc = {(LONG)written.x0, (LONG)written.y0, (LONG)written.x1, (LONG)written.y1};
                    cl_->RSSetScissorRects(1, &sc);
                    PassConstants pc = step.Constants(srcW, srcH);
                    step.valid = written;
                    cl_->SetGraphicsRoot32BitConstants(1, sizeof(pc) / 4, &pc, 0);
                    const D3D12_GPU_DESCRIPTOR_HANDLE src = (l == 1) ? firstSource : heap_.Gpu(backdropTable_ + l - 1);
                    cl_->SetGraphicsRootDescriptorTable(2, src);
                    cl_->DrawInstanced(3, 1, 0, 0);
                    ++statDraws_;
                    TransitionSurface(dst, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
                    srcW = dst.w;
                    srcH = dst.h;
                }
            }

            // The render target as a shader resource when it allows one and reads the same values as the copy; the
            // descriptor belongs to this frame's slot, so frames in flight keep theirs.
            bool TargetSrv()
            {
                if (targetSrvTried_)
                    return targetSrvOk_;
                targetSrvTried_ = true;
                targetSrvOk_ = false;
                const D3D12_RESOURCE_DESC d = rtRes_->GetDesc();
                const DXGI_FORMAT view = CopyViewFormat(CopyStorageFormat(rtFormat_));
                if (d.SampleDesc.Count != 1 || (d.Flags & D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE) || (rtFormat_ != view && rtFormat_ != CopyStorageFormat(rtFormat_)))
                    return false;
                FrameCtx& f = Frame();
                if (f.targetSrv < 0)
                    f.targetSrv = heap_.AllocSingle();
                if (f.targetSrv < 0)
                    return false;
                CreateSrv(rtRes_, view, f.targetSrv);
                targetSrvOk_ = true;
                return true;
            }

            void CaptureBackdrop(const PxRect& wanted, int levels, bool needsLevel0)
            {
                if (!surfacesReady_)
                    return;
                const PxRect region = AlignCaptureRegion(wanted, rtW_, rtH_);
                if (region.Empty())
                    return;
                const int timing = gpuTimer_.Begin(cl_);
                // Glass that only reads blurred levels (frost) needs no full-resolution copy: the first downsample
                // reads the render target directly - same values, a copy's bandwidth saved
                if (!needsLevel0 && rtSamples_ == 1 && TargetSrv())
                {
                    D3D12_RESOURCE_BARRIER b = Transition(rtRes_, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
                    cl_->ResourceBarrier(1, &b);
                    BuildPyramid(heap_.Gpu(Frame().targetSrv), region, levels);
                    b = Transition(rtRes_, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
                    cl_->ResourceBarrier(1, &b);
                    SetupCommonState(layerDepth_ > 0);
                    gpuTimer_.End(cl_, timing, GpuTimer12::Glass);
                    return;
                }
                if (rtSamples_ > 1)
                {
                    D3D12_RESOURCE_BARRIER b[2] = {Transition(rtRes_, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_RESOLVE_SOURCE),
                                                   Transition(copy_.res.Get(), copy_.state, D3D12_RESOURCE_STATE_RESOLVE_DEST)};
                    cl_->ResourceBarrier(2, b);
                    cl_->ResolveSubresource(copy_.res.Get(), 0, rtRes_, 0, ResolveFormat(CopyViewFormat(CopyStorageFormat(rtFormat_))));
                    b[0] = Transition(rtRes_, D3D12_RESOURCE_STATE_RESOLVE_SOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
                    b[1] = Transition(copy_.res.Get(), D3D12_RESOURCE_STATE_RESOLVE_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
                    cl_->ResourceBarrier(2, b);
                }
                else
                {
                    D3D12_RESOURCE_BARRIER b[2] = {Transition(rtRes_, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_SOURCE),
                                                   Transition(copy_.res.Get(), copy_.state, D3D12_RESOURCE_STATE_COPY_DEST)};
                    cl_->ResourceBarrier(2, b);
                    D3D12_TEXTURE_COPY_LOCATION dst = {}, src = {};
                    dst.pResource = copy_.res.Get();
                    dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
                    src.pResource = rtRes_;
                    src.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
                    D3D12_BOX box = {(UINT)region.x0, (UINT)region.y0, 0, (UINT)region.x1, (UINT)region.y1, 1};
                    cl_->CopyTextureRegion(&dst, (UINT)region.x0, (UINT)region.y0, 0, &src, &box);
                    b[0] = Transition(rtRes_, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
                    b[1] = Transition(copy_.res.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
                    cl_->ResourceBarrier(2, b);
                }
                copy_.state = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
                BuildPyramid(heap_.Gpu(backdropTable_), region, levels);
                SetupCommonState(layerDepth_ > 0);
                gpuTimer_.End(cl_, timing, GpuTimer12::Glass);
            }

            void BeginLayer(const RenderOp& op)
            {
                if (layerDepth_++ > 0 || !surfacesReady_)
                    return;
                TransitionSurface(layer_, D3D12_RESOURCE_STATE_RENDER_TARGET);
                // clear only the region the layer will be sampled in (the pyramid clamps to it)
                const float clear[4] = {0, 0, 0, 0};
                const PxRect region = AlignCaptureRegion(op.bounds, rtW_, rtH_);
                const D3D12_RECT rc = {(LONG)region.x0, (LONG)region.y0, (LONG)region.x1, (LONG)region.y1};
                cl_->ClearRenderTargetView(layer_.rtv, clear, 1, &rc);
                currentRtv_ = layer_.rtv;
                currentFormat_ = kLayerFormat;
                currentSamples_ = 1;
                activeLayer_ = op.layer;
                SetupCommonState(true);
            }

            void EndLayer(const RenderOp& op)
            {
                if (layerDepth_ == 0)
                    return;
                if (--layerDepth_ > 0 || !surfacesReady_)
                    return;
                const int timing = gpuTimer_.Begin(cl_);
                TransitionSurface(layer_, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
                const PxRect region = AlignCaptureRegion(op.bounds, rtW_, rtH_);
                BuildPyramid(heap_.Gpu(layerSrv_), region, LevelsForBloom(op.blurPx));

                currentRtv_ = mainRtv_;
                currentFormat_ = rtvFormat_;
                currentSamples_ = rtSamples_;
                SetupCommonState(false);
                ID3D12PipelineState* pso = GetPso(PipeKind::Layer, 0, currentFormat_, currentSamples_);
                if (!pso)
                    return;
                BindPso(pso);
                SetTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
                PassConstants pc = {{activeLayer_.color[0], activeLayer_.color[1], activeLayer_.color[2], activeLayer_.color[3]},
                                    {activeLayer_.intensity, activeLayer_.radius, activeLayer_.opacity, 0}};
                cl_->SetGraphicsRoot32BitConstants(1, sizeof(pc) / 4, &pc, 0);
                cl_->SetGraphicsRootDescriptorTable(2, heap_.Gpu(layerSrv_));
                boundTex_ = ~0ull;
                SetScissor(op.bounds);
                cl_->DrawInstanced(3, 1, 0, 0);
                ++statDraws_;
                ++statLayers_;
                gpuTimer_.End(cl_, timing, GpuTimer12::Layer);
            }

            // ---------------------------------------------------- execute
            void Execute()
            {
                ImGuiPlatformIO& pio = ImGui::GetPlatformIO();
                for (const RenderOp& op : plan_.ops)
                {
                    switch (op.type)
                    {
                    case RenderOp::Draw:
                    {
                        const PipeKind pk = !op.coverage ? PipeKind::ImGui : !op.lcd ? PipeKind::ImGuiText
                                          : (layerDepth_ > 0 ? PipeKind::ImGuiTextLcdGray : PipeKind::ImGuiTextLcd);
                        ID3D12PipelineState* pso = GetPso(pk, 0, currentFormat_, currentSamples_);
                        if (!pso)
                            break;
                        BindPso(pso);
                        SetTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
                        cl_->IASetVertexBuffers(0, 1, &vbv_);
                        cl_->IASetIndexBuffer(&ibv_);
                        BindTexture(op.texture);
                        SetScissor(op.clip);
                        SetFade(op.fade);
                        cl_->DrawIndexedInstanced(op.idxCount, 1, op.idxOffset, (INT)op.vtxOffset, 0);
                        ++statDraws_;
                        break;
                    }
                    case RenderOp::FxBatch:
                    {
                        // captures are planned per frame (FramePlan::PlanCaptures): one serves many glass batches
                        if (op.glass && surfacesReady_ && !op.captureRegion.Empty())
                        {
                            if (captures_ < captureBudget_)
                            {
                                CaptureBackdrop(op.captureRegion, op.captureLevels, op.captureLevel0);
                                ++captures_;
                            }
                            else
                                overBudget_ = true;
                        }
                        ID3D12PipelineState* pso = GetPso(PipeKind::Fx, op.effect, currentFormat_, currentSamples_);
                        if (!pso)
                            break;
                        BindPso(pso);
                        SetTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
                        cl_->IASetVertexBuffers(0, 1, &instv_);
                        BindTexture(op.texture);
                        SetScissor(op.clip);
                        SetFade(op.fade);
                        cl_->DrawInstanced(4, op.instCount, 0, op.instStart);
                        ++statDraws_;
                        break;
                    }
                    case RenderOp::LayerBegin:
                        BeginLayer(op);
                        break;
                    case RenderOp::LayerEnd:
                        EndLayer(op);
                        break;
                    case RenderOp::ResetState:
                        SetupCommonState(layerDepth_ > 0);
                        break;
                    case RenderOp::UserCallback:
                        SetScissor(op.clip);
                        pio.Renderer_RenderState = cl_;
                        op.cmd->UserCallback(op.list, op.cmd);
                        pio.Renderer_RenderState = nullptr;
                        SetupCommonState(layerDepth_ > 0);
                        break;
                    }
                }
            }

            // ---------------------------------------------------- members
            ComPtr<ID3D12Device> device_;
            std::uint32_t numFrames_ = 3;
            std::vector<FrameCtx> frames_;
            std::uint32_t frameCursor_ = 0;
            DescriptorHeap12 heap_;
            ComPtr<ID3D12DescriptorHeap> rtvHeap_;
            UINT rtvInc_ = 0;
            ComPtr<ID3D12RootSignature> rootSig_;
            std::unordered_map<std::uint64_t, ComPtr<ID3D12PipelineState>> psos_;
            int whiteSrv_ = -1;

            Surface12 copy_;
            Surface12 levels_[kBackdropLevels];
            Surface12 layer_;
            int backdropTable_ = -1;
            int layerSrv_ = -1;
            int surfW_ = 0, surfH_ = 0;
            DXGI_FORMAT surfFormat_ = DXGI_FORMAT_UNKNOWN;
            bool surfacesReady_ = false;

            // per frame
            ID3D12GraphicsCommandList* cl_ = nullptr;
            ID3D12Resource* rtRes_ = nullptr;
            bool targetSrvTried_ = false, targetSrvOk_ = false;   // TargetSrv, per frame
            D3D12_CPU_DESCRIPTOR_HANDLE mainRtv_{};
            D3D12_CPU_DESCRIPTOR_HANDLE currentRtv_{};
            DXGI_FORMAT rtFormat_ = DXGI_FORMAT_UNKNOWN;
            DXGI_FORMAT rtvFormat_ = DXGI_FORMAT_UNKNOWN;
            DXGI_FORMAT currentFormat_ = DXGI_FORMAT_UNKNOWN;
            UINT rtSamples_ = 1, currentSamples_ = 1;
            int rtW_ = 0, rtH_ = 0;
            bool outputLinear_ = false;
            D3D12_GPU_VIRTUAL_ADDRESS frameCB_ = 0, frameCBLayer_ = 0;
            D3D12_VERTEX_BUFFER_VIEW vbv_{}, instv_{};
            D3D12_INDEX_BUFFER_VIEW ibv_{};
            const ImDrawData* display_ = nullptr;
            FramePlan plan_;
            int captures_ = 0, captureBudget_ = 64;
            bool overBudget_ = false;
            ID3D12PipelineState* boundPso_ = nullptr;
            UINT64 boundTex_ = ~0ull;
            D3D_PRIMITIVE_TOPOLOGY boundTopology_ = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
            int layerDepth_ = 0;
            fx::LayerParams activeLayer_{};

            std::mutex effectMutex_;
            std::unordered_map<EffectId, Effect12> effects_;
            std::mutex texMutex_;
            std::unordered_map<ImTextureID, Texture12> userTextures_;
            std::vector<PendingUpload> pendingUploads_;
            std::vector<Texture12> pendingDestroy_;

            void (*log_)(void*, int, const char*) = nullptr;
            void* logUser_ = nullptr;
            int statDraws_ = 0, statFx_ = 0, statCaptures_ = 0, statLayers_ = 0, statVerts_ = 0;
            float drawFade_[4] = {0, 0, 0, 0};   // edge fade in root constants (b2)
            GpuTimer12 gpuTimer_;
            bool warnedBudget_ = false;
        };
    }

    std::unique_ptr<IRenderBackend> CreateD3D12Backend(ID3D12Device* device, std::uint32_t framesInFlight)
    {
        return std::make_unique<D3D12Backend>(device, framesInFlight);
    }
}
