// WGT UI - Direct3D 11 render backend.
//
// Replaces imgui_impl_dx11 entirely: own shaders, ImGui texture protocol, SDF FX pipeline,
// liquid-glass backdrop capture (region-limited copy + 13-tap pyramid), bloom glow layers,
// runtime-compiled user effects and full host pipeline-state backup / restore.
#include <chrono>
#include <future>
#include "backends/backends.hpp"
#include "render/frame_plan.hpp"
#include "render/gpu_common.hpp"
#include "render/effect_compiler.hpp"

#include <windows.h>
#include <d3d11_1.h>
#include <wrl/client.h>
#include <mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>
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
        void D3D11ResetMarker(const ImDrawList*, const ImDrawCmd*) {}

        struct ImGuiTex11
        {
            ComPtr<ID3D11Texture2D> tex;
            ComPtr<ID3D11ShaderResourceView> srv;
        };

        struct Surface11
        {
            ComPtr<ID3D11Texture2D> tex;
            ComPtr<ID3D11RenderTargetView> rtv;
            ComPtr<ID3D11ShaderResourceView> srv;
            int w = 0, h = 0;
        };

        // GPU timing without stalls: timestamps around the frame and around every glass capture / glow layer,
        // read back kSlots frames later.
        class GpuTimer11
        {
        public:
            // Glass = backdrop capture (copy + blur pyramid), Layer = glow layer; the others only in the
            // WGT_GPU_PROFILE breakdown (runs of SDF batches with / without glass, ImGui geometry incl. text)
            enum Kind { Glass = 0, Layer = 1, Fx = 2, FxGlass = 3, Imgui = 4, KindCount = 5 };

            void Init(ID3D11Device* device)
            {
                for (Slot& s : slots_)
                {
                    D3D11_QUERY_DESC qd = {D3D11_QUERY_TIMESTAMP_DISJOINT, 0};
                    device->CreateQuery(&qd, &s.disjoint);
                    qd.Query = D3D11_QUERY_TIMESTAMP;
                    for (auto& q : s.stamps)
                        device->CreateQuery(&qd, &q);
                }
            }
            void Release()
            {
                for (Slot& s : slots_)
                {
                    s.disjoint.Reset();
                    for (auto& q : s.stamps)
                        q.Reset();
                    s.pending = false;
                }
            }

            void BeginFrame(ID3D11DeviceContext* ctx)
            {
                cur_ = &slots_[frame_++ % kSlots];
                if (cur_->pending)
                    Read(ctx, *cur_);
                cur_->used = 0;
                cur_->intervals.clear();
                if (!cur_->disjoint)
                {
                    cur_ = nullptr;
                    return;
                }
                ctx->Begin(cur_->disjoint.Get());
                Stamp(ctx);
            }
            int Begin(ID3D11DeviceContext* ctx) { return cur_ ? Stamp(ctx) : -1; }
            void End(ID3D11DeviceContext* ctx, int start, Kind kind)
            {
                if (cur_ && start >= 0)
                {
                    const int end = Stamp(ctx);
                    if (end >= 0)
                        cur_->intervals.push_back({start, end, kind});
                }
            }
            void EndFrame(ID3D11DeviceContext* ctx)
            {
                if (!cur_)
                    return;
                cur_->frameEnd = Stamp(ctx);
                ctx->End(cur_->disjoint.Get());
                cur_->pending = cur_->frameEnd > 0;
                cur_ = nullptr;
            }
            float totalMs = 0.0f, glassMs = 0.0f, layerMs = 0.0f;
            float kindMs[KindCount] = {};

        private:
            static constexpr int kSlots = 4, kMaxStamps = 1024;
            struct Interval
            {
                int start, end;
                Kind kind;
            };
            struct Slot
            {
                ComPtr<ID3D11Query> disjoint;
                ComPtr<ID3D11Query> stamps[kMaxStamps];
                int used = 0, frameEnd = -1;
                std::vector<Interval> intervals;
                bool pending = false;
            };
            int Stamp(ID3D11DeviceContext* ctx)
            {
                if (!cur_ || cur_->used >= kMaxStamps)
                    return -1;
                ctx->End(cur_->stamps[cur_->used].Get());
                return cur_->used++;
            }
            void Read(ID3D11DeviceContext* ctx, Slot& s)
            {
                s.pending = false;
                D3D11_QUERY_DATA_TIMESTAMP_DISJOINT dj = {};
                if (ctx->GetData(s.disjoint.Get(), &dj, sizeof(dj), D3D11_ASYNC_GETDATA_DONOTFLUSH) != S_OK || dj.Disjoint || dj.Frequency == 0)
                    return;
                std::uint64_t t[kMaxStamps] = {};
                for (int i = 0; i < s.used; ++i)
                    if (ctx->GetData(s.stamps[i].Get(), &t[i], sizeof(std::uint64_t), D3D11_ASYNC_GETDATA_DONOTFLUSH) != S_OK)
                        return;
                const double k = 1000.0 / (double)dj.Frequency;
                double sums[KindCount] = {};
                for (const Interval& iv : s.intervals)
                    sums[iv.kind] += (double)(t[iv.end] - t[iv.start]) * k;
                totalMs = (float)((double)(t[s.frameEnd] - t[0]) * k);
                glassMs = (float)sums[Glass];
                layerMs = (float)sums[Layer];
                for (int i = 0; i < KindCount; ++i)
                    kindMs[i] = (float)sums[i];
            }
            Slot slots_[kSlots];
            Slot* cur_ = nullptr;
            std::uint64_t frame_ = 0;
        };

        struct CompiledEffect11
        {
            ComPtr<ID3D11PixelShader> ps;
            std::string error;
            bool ok = false;
        };

        struct Effect11
        {
            std::string name;
            std::string source;
            ComPtr<ID3D11PixelShader> ps;
            bool compiled = false;
            bool failed = false;
            std::future<CompiledEffect11> job;   // compiling on a worker thread
        };

        // Host pipeline state saved around Render().
        struct StateBackup11
        {
            UINT scissorCount = 0, viewportCount = 0;
            D3D11_RECT scissors[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
            D3D11_VIEWPORT viewports[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
            ComPtr<ID3D11RasterizerState> rs;
            ComPtr<ID3D11BlendState> blend;
            FLOAT blendFactor[4];
            UINT sampleMask;
            UINT stencilRef;
            ComPtr<ID3D11DepthStencilState> depth;
            ID3D11ShaderResourceView* psSrv[kBackdropLevels + 1] = {};
            ID3D11SamplerState* psSamplers[2] = {};
            ComPtr<ID3D11PixelShader> ps;
            ComPtr<ID3D11VertexShader> vs;
            ComPtr<ID3D11GeometryShader> gs;
            ID3D11Buffer* vsCb[2] = {};
            ID3D11Buffer* psCb[3] = {};
            D3D11_PRIMITIVE_TOPOLOGY topology;
            ComPtr<ID3D11Buffer> ib;
            DXGI_FORMAT ibFormat;
            UINT ibOffset;
            ComPtr<ID3D11Buffer> vb;
            UINT vbStride, vbOffset;
            ComPtr<ID3D11InputLayout> layout;
            ID3D11RenderTargetView* rtvs[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT] = {};
            ComPtr<ID3D11DepthStencilView> dsv;

            void Capture(ID3D11DeviceContext* c)
            {
                scissorCount = viewportCount = D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
                c->RSGetScissorRects(&scissorCount, scissors);
                c->RSGetViewports(&viewportCount, viewports);
                c->RSGetState(&rs);
                c->OMGetBlendState(&blend, blendFactor, &sampleMask);
                c->OMGetDepthStencilState(&depth, &stencilRef);
                c->PSGetShaderResources(0, kBackdropLevels + 1, psSrv);
                c->PSGetSamplers(0, 2, psSamplers);
                c->PSGetShader(&ps, nullptr, nullptr);
                c->VSGetShader(&vs, nullptr, nullptr);
                c->GSGetShader(&gs, nullptr, nullptr);
                c->VSGetConstantBuffers(0, 2, vsCb);
                c->PSGetConstantBuffers(0, 3, psCb);
                c->IAGetPrimitiveTopology(&topology);
                c->IAGetIndexBuffer(&ib, &ibFormat, &ibOffset);
                c->IAGetVertexBuffers(0, 1, &vb, &vbStride, &vbOffset);
                c->IAGetInputLayout(&layout);
                c->OMGetRenderTargets(D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT, rtvs, &dsv);
            }

            void Restore(ID3D11DeviceContext* c)
            {
                c->OMSetRenderTargets(D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT, rtvs, dsv.Get());
                c->RSSetScissorRects(scissorCount, scissors);
                c->RSSetViewports(viewportCount, viewports);
                c->RSSetState(rs.Get());
                c->OMSetBlendState(blend.Get(), blendFactor, sampleMask);
                c->OMSetDepthStencilState(depth.Get(), stencilRef);
                c->PSSetShaderResources(0, kBackdropLevels + 1, psSrv);
                c->PSSetSamplers(0, 2, psSamplers);
                c->PSSetShader(ps.Get(), nullptr, 0);
                c->VSSetShader(vs.Get(), nullptr, 0);
                c->GSSetShader(gs.Get(), nullptr, 0);
                c->VSSetConstantBuffers(0, 2, vsCb);
                c->PSSetConstantBuffers(0, 3, psCb);
                c->IASetPrimitiveTopology(topology);
                c->IASetIndexBuffer(ib.Get(), ibFormat, ibOffset);
                ID3D11Buffer* vbp = vb.Get();
                c->IASetVertexBuffers(0, 1, &vbp, &vbStride, &vbOffset);
                c->IASetInputLayout(layout.Get());
                for (auto*& p : psSrv) if (p) { p->Release(); p = nullptr; }
                for (auto*& p : psSamplers) if (p) { p->Release(); p = nullptr; }
                for (auto*& p : vsCb) if (p) { p->Release(); p = nullptr; }
                for (auto*& p : psCb) if (p) { p->Release(); p = nullptr; }
                for (auto*& p : rtvs) if (p) { p->Release(); p = nullptr; }
            }
        };

        class D3D11Backend final : public IRenderBackend
        {
        public:
            D3D11Backend(ID3D11Device* device, ID3D11DeviceContext* context, bool restore)
                : device_(device), ctx_(context), restoreState_(restore) {}

            const char* Name() const override { return "wgt_d3d11"; }

            bool Init(ImGuiIO& io) override
            {
                if (!device_ || !ctx_)
                    return false;
                io.BackendRendererName = "wgt_d3d11";
                io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset | ImGuiBackendFlags_RendererHasTextures;
                ImGuiPlatformIO& pio = ImGui::GetPlatformIO();
                pio.Renderer_TextureMaxWidth = pio.Renderer_TextureMaxHeight = D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION;
                pio.DrawCallback_ResetRenderState = &D3D11ResetMarker;
                return CreateDeviceObjects();
            }

            void Shutdown() override
            {
                if (ImGui::GetCurrentContext())
                {
                    for (ImTextureData* tex : ImGui::GetPlatformIO().Textures)
                        if (tex->RefCount == 1)
                            DestroyImGuiTexture(tex);
                    ImGuiIO& io = ImGui::GetIO();
                    io.BackendRendererName = nullptr;
                    io.BackendFlags &= ~(ImGuiBackendFlags_RendererHasVtxOffset | ImGuiBackendFlags_RendererHasTextures);
                    ImGui::GetPlatformIO().ClearRendererHandlers();
                }
                {
                    std::lock_guard lock(texMutex_);
                    userTextures_.clear();
                    pendingRelease_.clear();
                }
                ReleaseSurfaces();
                ReleaseDeviceObjects();
            }

            void SetLogCallback(void (*log)(void*, int, const char*), void* user) override
            {
                log_ = log;
                logUser_ = user;
            }

            // --------------------------------------------------- textures
            ImTextureID CreateTexture(const void* rgba8, int width, int height) override
            {
                D3D11_TEXTURE2D_DESC desc = {};
                desc.Width = (UINT)width;
                desc.Height = (UINT)height;
                desc.MipLevels = 1;
                desc.ArraySize = 1;
                desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
                desc.SampleDesc.Count = 1;
                desc.Usage = D3D11_USAGE_DEFAULT;
                desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
                D3D11_SUBRESOURCE_DATA init = {rgba8, (UINT)width * 4, 0};
                ComPtr<ID3D11Texture2D> tex;
                if (FAILED(device_->CreateTexture2D(&desc, &init, &tex)))
                    return ImTextureID_Invalid;
                ComPtr<ID3D11ShaderResourceView> srv;
                if (FAILED(device_->CreateShaderResourceView(tex.Get(), nullptr, &srv)))
                    return ImTextureID_Invalid;
                ImTextureID id = (ImTextureID)(intptr_t)srv.Get();
                std::lock_guard lock(texMutex_);
                userTextures_.emplace(id, srv);
                return id;
            }

            void DestroyTexture(ImTextureID id) override
            {
                std::lock_guard lock(texMutex_);
                auto it = userTextures_.find(id);
                if (it != userTextures_.end())
                {
                    pendingRelease_.push_back(it->second);
                    userTextures_.erase(it);
                }
            }

            void SetEffectSource(EffectId id, const char* name, const char* source) override
            {
                std::lock_guard lock(effectMutex_);
                Effect11& e = effects_[id];
                e.name = name ? name : "effect";
                e.source = source ? source : "";
                e.ps.Reset();
                e.job = {};   // a compile of the previous source is discarded (waits for it to finish)
                e.compiled = false;
                e.failed = false;
                StartCompile(e);   // right away: usually ready before the first shape using it is drawn
            }

            void InvalidateDeviceObjects() override { ReleaseSurfaces(); }

            bool QueryTargetSize(const RenderTarget& target, int& width, int& height) override
            {
                if (!target.d3d11Rtv)
                    return false;
                ComPtr<ID3D11Resource> res;
                target.d3d11Rtv->GetResource(&res);
                ComPtr<ID3D11Texture2D> tex;
                if (!res || FAILED(res.As(&tex)))
                    return false;
                D3D11_TEXTURE2D_DESC d;
                tex->GetDesc(&d);
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
                if (!dd || dd->DisplaySize.x <= 0.0f || dd->DisplaySize.y <= 0.0f || !target.d3d11Rtv)
                    return;

                if (dd->Textures)
                    for (ImTextureData* tex : *dd->Textures)
                        if (tex->Status != ImTextureStatus_OK)
                            UpdateImGuiTexture(tex);

                plan_.Build(dd, &D3D11ResetMarker);
                statVerts_ = plan_.totalVtx;
                statFx_ = plan_.fxCount;
                // Inspect the render target.
                mainRtv_ = target.d3d11Rtv;
                ComPtr<ID3D11Resource> rtRes;
                mainRtv_->GetResource(&rtRes);
                ComPtr<ID3D11Texture2D> rtTex;
                if (FAILED(rtRes.As(&rtTex)))
                    return;
                D3D11_TEXTURE2D_DESC rtDesc;
                rtTex->GetDesc(&rtDesc);
                D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
                mainRtv_->GetDesc(&rtvDesc);
                rtTex_ = rtTex.Get();
                rtW_ = (int)rtDesc.Width;
                rtH_ = (int)rtDesc.Height;
                rtSamples_ = rtDesc.SampleDesc.Count;
                rtFormat_ = rtDesc.Format;
                outputLinear_ = IsSrgb(rtvDesc.Format);

                const bool needSurfaces = plan_.anyGlass || plan_.anyLayer;
                surfacesReady_ = needSurfaces ? EnsureSurfaces() : false;

                if (!UploadBuffers(dd))
                    return;

                StateBackup11 backup;
                if (restoreState_)
                    backup.Capture(ctx_.Get());

                FrameConstants fc{};
                FillFrameConstants(fc, dd, rtW_, rtH_, info.time, info.deltaTime, surfacesReady_, outputLinear_);
                fc.text[0] = info.textGamma;
                fc.text[1] = info.textGrayscaleContrast;
                fc.text[2] = info.textClearTypeContrast;
                fc.text[3] = info.textClearTypeLevel;
                UpdateCB(frameCB_.Get(), &fc, sizeof(fc));
                fc.time[3] = 0.0f;   // layers are composed in gamma space and converted on composite
                UpdateCB(frameCBLayer_.Get(), &fc, sizeof(fc));

                display_ = dd;
                captures_ = 0;
                captureBudget_ = std::max(1, info.maxBackdropCaptures);
                overBudget_ = false;
                layerDepth_ = 0;
                currentRtv_ = mainRtv_;
                gpuTimer_.BeginFrame(ctx_.Get());
                SetupCommonState(false);
                Execute();
                gpuTimer_.EndFrame(ctx_.Get());
                if (profile_)
                {
                    // average over 240 frames, logged (level 2) for tuning
                    for (int i = 0; i < GpuTimer11::KindCount; ++i)
                        profAcc_[i] += gpuTimer_.kindMs[i];
                    profAcc_[GpuTimer11::KindCount] += gpuTimer_.totalMs;
                    if (++profFrames_ == 240)
                    {
                        char line[256];
                        const double n = (double)profFrames_;
                        std::snprintf(line, sizeof(line), "WGT gpu profile (ms/frame): total %.3f | capture %.3f | fx-glass %.3f | fx %.3f | imgui/text %.3f | layers %.3f",
                                      profAcc_[GpuTimer11::KindCount] / n, profAcc_[GpuTimer11::Glass] / n, profAcc_[GpuTimer11::FxGlass] / n,
                                      profAcc_[GpuTimer11::Fx] / n, profAcc_[GpuTimer11::Imgui] / n, profAcc_[GpuTimer11::Layer] / n);
                        Log(2, line);
                        profFrames_ = 0;
                        for (double& v : profAcc_)
                            v = 0.0;
                    }
                }

                if (restoreState_)
                    backup.Restore(ctx_.Get());
                else
                {
                    ID3D11ShaderResourceView* nulls[kBackdropLevels + 1] = {};
                    ctx_->PSSetShaderResources(0, kBackdropLevels + 1, nulls);
                }
                statCaptures_ = captures_;
                if (overBudget_ && !warnedBudget_)
                {
                    warnedBudget_ = true;
                    Log(1, "WGT: liquid-glass capture budget exceeded (ContextDesc::maxBackdropCaptures = " + std::to_string(info.maxBackdropCaptures) + "): some glass reuses an older backdrop");
                }
                mainRtv_ = nullptr;
                rtTex_ = nullptr;
                targetSrv_.Reset();
                targetSrvTried_ = false;

                std::lock_guard lock(texMutex_);
                pendingRelease_.clear();
            }

        private:
            enum class Pipe { None, ImGui, Fx };

            void Log(int level, const std::string& msg)
            {
                if (log_)
                    log_(logUser_, level, msg.c_str());
            }

            // -------------------------------------------- device objects
            bool CreateDeviceObjects()
            {
                gpuTimer_.Init(device_.Get());
                ctx_.As(&ctx1_);   // D3D11.1: region clears (optional)
                HRESULT hr = S_OK;
                hr |= device_->CreateVertexShader(kShaderImGuiVS, sizeof(kShaderImGuiVS), nullptr, &imguiVS_);
                hr |= device_->CreatePixelShader(kShaderImGuiPS, sizeof(kShaderImGuiPS), nullptr, &imguiPS_);
                hr |= device_->CreatePixelShader(kShaderImGuiTextPS, sizeof(kShaderImGuiTextPS), nullptr, &imguiTextPS_);
                hr |= device_->CreatePixelShader(kShaderImGuiTextLcdPS, sizeof(kShaderImGuiTextLcdPS), nullptr, &imguiTextLcdPS_);
                hr |= device_->CreatePixelShader(kShaderImGuiTextLcdGrayPS, sizeof(kShaderImGuiTextLcdGrayPS), nullptr, &imguiTextLcdGrayPS_);
                hr |= device_->CreateVertexShader(kShaderFxVS, sizeof(kShaderFxVS), nullptr, &fxVS_);
                hr |= device_->CreatePixelShader(kShaderFxPS, sizeof(kShaderFxPS), nullptr, &fxPS_);
                hr |= device_->CreateVertexShader(kShaderFullscreenVS, sizeof(kShaderFullscreenVS), nullptr, &fullscreenVS_);
                hr |= device_->CreatePixelShader(kShaderDownsamplePS, sizeof(kShaderDownsamplePS), nullptr, &downsamplePS_);
                hr |= device_->CreatePixelShader(kShaderLayerCompositePS, sizeof(kShaderLayerCompositePS), nullptr, &layerPS_);
                if (FAILED(hr))
                    return false;

                D3D11_INPUT_ELEMENT_DESC imguiLayout[] = {
                    {"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, (UINT)offsetof(ImDrawVert, pos), D3D11_INPUT_PER_VERTEX_DATA, 0},
                    {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, (UINT)offsetof(ImDrawVert, uv), D3D11_INPUT_PER_VERTEX_DATA, 0},
                    {"COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, (UINT)offsetof(ImDrawVert, col), D3D11_INPUT_PER_VERTEX_DATA, 0},
                };
                if (FAILED(device_->CreateInputLayout(imguiLayout, 3, kShaderImGuiVS, sizeof(kShaderImGuiVS), &imguiLayout_)))
                    return false;

                D3D11_INPUT_ELEMENT_DESC fxLayout[fx::kInstanceVec4Count];
                for (UINT i = 0; i < fx::kInstanceVec4Count; ++i)
                {
                    const bool isFlags = (i == fx::kInstanceVec4Count - 1);
                    fxLayout[i] = {isFlags ? "FLAGS" : "INST", isFlags ? 0u : i,
                                   isFlags ? DXGI_FORMAT_R32G32B32A32_UINT : DXGI_FORMAT_R32G32B32A32_FLOAT,
                                   0, i * 16u, D3D11_INPUT_PER_INSTANCE_DATA, 1};
                }
                if (FAILED(device_->CreateInputLayout(fxLayout, fx::kInstanceVec4Count, kShaderFxVS, sizeof(kShaderFxVS), &fxLayout_)))
                    return false;

                auto makeCB = [&](UINT size, ComPtr<ID3D11Buffer>& out) {
                    D3D11_BUFFER_DESC d = {};
                    d.ByteWidth = size;
                    d.Usage = D3D11_USAGE_DYNAMIC;
                    d.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
                    d.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
                    return SUCCEEDED(device_->CreateBuffer(&d, nullptr, &out));
                };
                if (!makeCB(sizeof(FrameConstants), frameCB_) || !makeCB(sizeof(FrameConstants), frameCBLayer_) || !makeCB(sizeof(PassConstants), passCB_) ||
                    !makeCB(sizeof(DrawConstants), drawCB_))
                    return false;

                auto makeBlend = [&](D3D11_BLEND src, D3D11_BLEND dst, bool enable, ComPtr<ID3D11BlendState>& out) {
                    D3D11_BLEND_DESC d = {};
                    d.RenderTarget[0].BlendEnable = enable;
                    d.RenderTarget[0].SrcBlend = src;
                    d.RenderTarget[0].DestBlend = dst;
                    d.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
                    d.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
                    d.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
                    d.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
                    d.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
                    device_->CreateBlendState(&d, &out);
                };
                makeBlend(D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA, true, blendStraight_);
                makeBlend(D3D11_BLEND_ONE, D3D11_BLEND_INV_SRC_ALPHA, true, blendPremul_);
                makeBlend(D3D11_BLEND_ONE, D3D11_BLEND_ZERO, false, blendOpaque_);
                {
                    // sub-pixel text: per-channel alpha from the second shader output (dual-source blending)
                    D3D11_BLEND_DESC bd = {};
                    bd.RenderTarget[0].BlendEnable = TRUE;
                    bd.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC1_COLOR;
                    bd.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC1_COLOR;
                    bd.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
                    bd.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_SRC1_ALPHA;
                    bd.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC1_ALPHA;
                    bd.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
                    bd.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
                    device_->CreateBlendState(&bd, &blendLcd_);
                }

                D3D11_RASTERIZER_DESC rd = {};
                rd.FillMode = D3D11_FILL_SOLID;
                rd.CullMode = D3D11_CULL_NONE;
                rd.ScissorEnable = TRUE;
                rd.DepthClipEnable = TRUE;
                device_->CreateRasterizerState(&rd, &raster_);

                D3D11_DEPTH_STENCIL_DESC dsd = {};
                dsd.DepthEnable = FALSE;
                dsd.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
                dsd.DepthFunc = D3D11_COMPARISON_ALWAYS;
                dsd.FrontFace.StencilFailOp = dsd.FrontFace.StencilDepthFailOp = dsd.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
                dsd.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
                dsd.BackFace = dsd.FrontFace;
                device_->CreateDepthStencilState(&dsd, &depth_);

                D3D11_SAMPLER_DESC sd = {};
                sd.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
                sd.AddressU = sd.AddressV = sd.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
                sd.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
                sd.MaxLOD = D3D11_FLOAT32_MAX;
                device_->CreateSamplerState(&sd, &samplerLinear_);
                sd.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
                device_->CreateSamplerState(&sd, &samplerPoint_);

                const UINT white = 0xFFFFFFFFu;
                D3D11_TEXTURE2D_DESC td = {};
                td.Width = td.Height = 1;
                td.MipLevels = td.ArraySize = 1;
                td.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
                td.SampleDesc.Count = 1;
                td.Usage = D3D11_USAGE_IMMUTABLE;
                td.BindFlags = D3D11_BIND_SHADER_RESOURCE;
                D3D11_SUBRESOURCE_DATA init = {&white, 4, 0};
                ComPtr<ID3D11Texture2D> wt;
                device_->CreateTexture2D(&td, &init, &wt);
                if (wt)
                    device_->CreateShaderResourceView(wt.Get(), nullptr, &whiteSrv_);
                return true;
            }

            void ReleaseDeviceObjects()
            {
                gpuTimer_.Release();
                imguiVS_.Reset(); imguiPS_.Reset(); imguiTextPS_.Reset(); imguiTextLcdPS_.Reset(); imguiTextLcdGrayPS_.Reset(); blendLcd_.Reset(); fxVS_.Reset(); fxPS_.Reset();
                fullscreenVS_.Reset(); downsamplePS_.Reset(); layerPS_.Reset();
                imguiLayout_.Reset(); fxLayout_.Reset();
                frameCB_.Reset(); frameCBLayer_.Reset(); passCB_.Reset(); drawCB_.Reset();
                vb_.Reset(); ib_.Reset(); instVB_.Reset();
                blendStraight_.Reset(); blendPremul_.Reset(); blendOpaque_.Reset();
                raster_.Reset(); depth_.Reset(); samplerLinear_.Reset(); samplerPoint_.Reset(); whiteSrv_.Reset();
                std::lock_guard lock(effectMutex_);
                effects_.clear();
            }

            void ReleaseSurfaces()
            {
                copy_ = {};
                for (auto& l : levels_)
                    l = {};
                layer_ = {};
                surfW_ = surfH_ = 0;
                surfFormat_ = DXGI_FORMAT_UNKNOWN;
            }

            bool CreateSurface(Surface11& s, int w, int h, DXGI_FORMAT storage, DXGI_FORMAT view, bool rt)
            {
                D3D11_TEXTURE2D_DESC d = {};
                d.Width = (UINT)w;
                d.Height = (UINT)h;
                d.MipLevels = d.ArraySize = 1;
                d.Format = storage;
                d.SampleDesc.Count = 1;
                d.Usage = D3D11_USAGE_DEFAULT;
                d.BindFlags = D3D11_BIND_SHADER_RESOURCE | (rt ? D3D11_BIND_RENDER_TARGET : 0);
                if (FAILED(device_->CreateTexture2D(&d, nullptr, &s.tex)))
                    return false;
                D3D11_SHADER_RESOURCE_VIEW_DESC sv = {};
                sv.Format = view;
                sv.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
                sv.Texture2D.MipLevels = 1;
                if (FAILED(device_->CreateShaderResourceView(s.tex.Get(), &sv, &s.srv)))
                    return false;
                if (rt)
                {
                    D3D11_RENDER_TARGET_VIEW_DESC rv = {};
                    rv.Format = view;
                    rv.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
                    if (FAILED(device_->CreateRenderTargetView(s.tex.Get(), &rv, &s.rtv)))
                        return false;
                }
                s.w = w;
                s.h = h;
                return true;
            }

            bool EnsureSurfaces()
            {
                if (surfW_ == rtW_ && surfH_ == rtH_ && surfFormat_ == rtFormat_ && copy_.tex)
                    return true;
                ReleaseSurfaces();
                const DXGI_FORMAT storage = CopyStorageFormat(rtFormat_);
                bool ok = CreateSurface(copy_, rtW_, rtH_, storage, CopyViewFormat(storage), false);
                for (int l = 1; ok && l < kBackdropLevels; ++l)
                    ok = CreateSurface(levels_[l], LevelSize(rtW_, l), LevelSize(rtH_, l), kPyramidFormat, kPyramidFormat, true);
                ok = ok && CreateSurface(layer_, rtW_, rtH_, kLayerFormat, kLayerFormat, true);
                if (!ok)
                {
                    Log(2, "WGT d3d11: failed to create backdrop surfaces");
                    ReleaseSurfaces();
                    return false;
                }
                surfW_ = rtW_;
                surfH_ = rtH_;
                surfFormat_ = rtFormat_;
                return true;
            }

            // ------------------------------------------- ImGui textures
            void DestroyImGuiTexture(ImTextureData* tex)
            {
                if (auto* bt = static_cast<ImGuiTex11*>(tex->BackendUserData))
                {
                    delete bt;
                    tex->SetTexID(ImTextureID_Invalid);
                    tex->BackendUserData = nullptr;
                }
                tex->SetStatus(ImTextureStatus_Destroyed);
            }

            void UpdateImGuiTexture(ImTextureData* tex)
            {
                if (tex->Status == ImTextureStatus_WantCreate)
                {
                    // RGBA32 (images) or Alpha8 (glyph atlases -> R8, rendered by the coverage pipeline)
                    auto* bt = new ImGuiTex11();
                    D3D11_TEXTURE2D_DESC d = {};
                    d.Width = (UINT)tex->Width;
                    d.Height = (UINT)tex->Height;
                    d.MipLevels = d.ArraySize = 1;
                    d.Format = tex->Format == ImTextureFormat_Alpha8 ? DXGI_FORMAT_R8_UNORM : DXGI_FORMAT_R8G8B8A8_UNORM;
                    d.SampleDesc.Count = 1;
                    d.Usage = D3D11_USAGE_DEFAULT;
                    d.BindFlags = D3D11_BIND_SHADER_RESOURCE;
                    D3D11_SUBRESOURCE_DATA init = {tex->GetPixels(), (UINT)tex->GetPitch(), 0};
                    device_->CreateTexture2D(&d, &init, &bt->tex);
                    if (bt->tex)
                        device_->CreateShaderResourceView(bt->tex.Get(), nullptr, &bt->srv);
                    tex->SetTexID((ImTextureID)(intptr_t)bt->srv.Get());
                    tex->BackendUserData = bt;
                    tex->SetStatus(ImTextureStatus_OK);
                }
                else if (tex->Status == ImTextureStatus_WantUpdates)
                {
                    auto* bt = static_cast<ImGuiTex11*>(tex->BackendUserData);
                    if (bt && bt->tex)
                        for (const ImTextureRect& r : tex->Updates)
                        {
                            D3D11_BOX box = {(UINT)r.x, (UINT)r.y, 0, (UINT)(r.x + r.w), (UINT)(r.y + r.h), 1};
                            ctx_->UpdateSubresource(bt->tex.Get(), 0, &box, tex->GetPixelsAt(r.x, r.y), (UINT)tex->GetPitch(), 0);
                        }
                    tex->SetStatus(ImTextureStatus_OK);
                }
                if (tex->Status == ImTextureStatus_WantDestroy && tex->UnusedFrames > 0)
                    DestroyImGuiTexture(tex);
            }

            // ------------------------------------------------- buffers
            template <class T>
            bool EnsureBuffer(ComPtr<ID3D11Buffer>& buf, int& capacity, int needed, UINT bind)
            {
                if (buf && capacity >= needed)
                    return true;
                buf.Reset();
                capacity = std::max(needed + needed / 2, 1024);
                D3D11_BUFFER_DESC d = {};
                d.Usage = D3D11_USAGE_DYNAMIC;
                d.ByteWidth = (UINT)(capacity * sizeof(T));
                d.BindFlags = bind;
                d.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
                return SUCCEEDED(device_->CreateBuffer(&d, nullptr, &buf));
            }

            bool UploadBuffers(ImDrawData* dd)
            {
                if (!EnsureBuffer<ImDrawVert>(vb_, vbCap_, std::max(dd->TotalVtxCount, 1), D3D11_BIND_VERTEX_BUFFER))
                    return false;
                if (!EnsureBuffer<ImDrawIdx>(ib_, ibCap_, std::max(dd->TotalIdxCount, 1), D3D11_BIND_INDEX_BUFFER))
                    return false;
                if (!EnsureBuffer<fx::Instance>(instVB_, instCap_, std::max((int)plan_.instances.size(), 1), D3D11_BIND_VERTEX_BUFFER))
                    return false;
                D3D11_MAPPED_SUBRESOURCE vm, im;
                if (FAILED(ctx_->Map(vb_.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &vm)))
                    return false;
                if (FAILED(ctx_->Map(ib_.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &im)))
                {
                    ctx_->Unmap(vb_.Get(), 0);
                    return false;
                }
                auto* vdst = static_cast<ImDrawVert*>(vm.pData);
                auto* idst = static_cast<ImDrawIdx*>(im.pData);
                for (const ImDrawList* dl : dd->CmdLists)
                {
                    std::memcpy(vdst, dl->VtxBuffer.Data, dl->VtxBuffer.Size * sizeof(ImDrawVert));
                    std::memcpy(idst, dl->IdxBuffer.Data, dl->IdxBuffer.Size * sizeof(ImDrawIdx));
                    vdst += dl->VtxBuffer.Size;
                    idst += dl->IdxBuffer.Size;
                }
                ctx_->Unmap(vb_.Get(), 0);
                ctx_->Unmap(ib_.Get(), 0);
                if (!plan_.instances.empty())
                {
                    D3D11_MAPPED_SUBRESOURCE xm;
                    if (FAILED(ctx_->Map(instVB_.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &xm)))
                        return false;
                    std::memcpy(xm.pData, plan_.instances.data(), plan_.instances.size() * sizeof(fx::Instance));
                    ctx_->Unmap(instVB_.Get(), 0);
                }
                return true;
            }

            void UpdateCB(ID3D11Buffer* cb, const void* data, size_t size)
            {
                D3D11_MAPPED_SUBRESOURCE m;
                if (SUCCEEDED(ctx_->Map(cb, 0, D3D11_MAP_WRITE_DISCARD, 0, &m)))
                {
                    std::memcpy(m.pData, data, size);
                    ctx_->Unmap(cb, 0);
                }
            }

            // --------------------------------------------------- state
            void SetViewportFull()
            {
                D3D11_VIEWPORT vp = {};
                vp.Width = display_->DisplaySize.x * display_->FramebufferScale.x;
                vp.Height = display_->DisplaySize.y * display_->FramebufferScale.y;
                vp.MaxDepth = 1.0f;
                ctx_->RSSetViewports(1, &vp);
            }

            void SetupCommonState(bool layerTarget)
            {
                ID3D11RenderTargetView* rtv = currentRtv_;
                ctx_->OMSetRenderTargets(1, &rtv, nullptr);
                SetViewportFull();
                ID3D11Buffer* cbs[3] = {layerTarget ? frameCBLayer_.Get() : frameCB_.Get(), passCB_.Get(), drawCB_.Get()};
                ctx_->VSSetConstantBuffers(0, 2, cbs);
                ctx_->PSSetConstantBuffers(0, 3, cbs);
                ID3D11SamplerState* samplers[2] = {samplerLinear_.Get(), samplerPoint_.Get()};
                ctx_->PSSetSamplers(0, 2, samplers);
                ctx_->GSSetShader(nullptr, nullptr, 0);
                ctx_->HSSetShader(nullptr, nullptr, 0);
                ctx_->DSSetShader(nullptr, nullptr, 0);
                ctx_->CSSetShader(nullptr, nullptr, 0);
                ctx_->OMSetDepthStencilState(depth_.Get(), 0);
                ctx_->RSSetState(raster_.Get());
                pipe_ = Pipe::None;
                boundTex_ = nullptr;
                boundEffect_ = ~0u;
                backdropBound_ = false;
            }

            void SetFade(const float fade[4])
            {
                if (std::memcmp(fade, drawFade_, sizeof(drawFade_)) == 0)
                    return;
                std::memcpy(drawFade_, fade, sizeof(drawFade_));
                DrawConstants dc;
                std::memcpy(dc.fade, fade, sizeof(dc.fade));
                UpdateCB(drawCB_.Get(), &dc, sizeof(dc));
            }

            // 0 = images / geometry, 1 = grayscale text, 2 = sub-pixel text, 3 = sub-pixel text inside a glow layer
            void BindImGuiPipe(const RenderOp& op)
            {
                const int coverage = !op.coverage ? 0 : !op.lcd ? 1 : (layerDepth_ > 0 ? 3 : 2);
                if (pipe_ != Pipe::ImGui)
                {
                    UINT stride = sizeof(ImDrawVert), offset = 0;
                    ID3D11Buffer* vb = vb_.Get();
                    ctx_->IASetInputLayout(imguiLayout_.Get());
                    ctx_->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
                    ctx_->IASetIndexBuffer(ib_.Get(), sizeof(ImDrawIdx) == 2 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT, 0);
                    ctx_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
                    ctx_->VSSetShader(imguiVS_.Get(), nullptr, 0);
                    const float bf[4] = {0, 0, 0, 0};
                    ctx_->OMSetBlendState(blendStraight_.Get(), bf, 0xFFFFFFFFu);
                    pipe_ = Pipe::ImGui;
                    boundEffect_ = ~0u;
                    coverageBound_ = -1;   // force the PS bind below
                }
                if (coverageBound_ != coverage)
                {
                    ID3D11PixelShader* ps = coverage == 0 ? imguiPS_.Get() : coverage == 1 ? imguiTextPS_.Get() : coverage == 2 ? imguiTextLcdPS_.Get() : imguiTextLcdGrayPS_.Get();
                    ctx_->PSSetShader(ps, nullptr, 0);
                    const bool lcd = coverage == 2, wasLcd = coverageBound_ == 2;
                    if (lcd != wasLcd)
                    {
                        const float bf[4] = {0, 0, 0, 0};
                        ctx_->OMSetBlendState(lcd ? blendLcd_.Get() : blendStraight_.Get(), bf, 0xFFFFFFFFu);
                    }
                    coverageBound_ = coverage;
                }
            }

            void BindFxPipe(EffectId effect)
            {
                if (pipe_ != Pipe::Fx)
                {
                    UINT stride = sizeof(fx::Instance), offset = 0;
                    ID3D11Buffer* vb = instVB_.Get();
                    ctx_->IASetInputLayout(fxLayout_.Get());
                    ctx_->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
                    ctx_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
                    ctx_->VSSetShader(fxVS_.Get(), nullptr, 0);
                    const float bf[4] = {0, 0, 0, 0};
                    ctx_->OMSetBlendState(blendPremul_.Get(), bf, 0xFFFFFFFFu);
                    pipe_ = Pipe::Fx;
                    boundEffect_ = ~0u;
                }
                if (boundEffect_ != effect)
                {
                    ID3D11PixelShader* ps = fxPS_.Get();
                    if (effect != 0)
                        if (ID3D11PixelShader* custom = GetEffectShader(effect))
                            ps = custom;
                    ctx_->PSSetShader(ps, nullptr, 0);
                    boundEffect_ = effect;
                }
                if (!backdropBound_)
                {
                    ID3D11ShaderResourceView* srvs[kBackdropLevels] = {};
                    if (surfacesReady_)
                    {
                        srvs[0] = copy_.srv.Get();
                        for (int l = 1; l < kBackdropLevels; ++l)
                            srvs[l] = levels_[l].srv.Get();
                    }
                    ctx_->PSSetShaderResources(1, kBackdropLevels, srvs);
                    backdropBound_ = true;
                }
            }

            void BindTexture(ImTextureID id)
            {
                ID3D11ShaderResourceView* srv = id != ImTextureID_Invalid ? (ID3D11ShaderResourceView*)(intptr_t)id : whiteSrv_.Get();
                if (srv != boundTex_)
                {
                    ctx_->PSSetShaderResources(0, 1, &srv);
                    boundTex_ = srv;
                }
            }

            void SetScissor(const PxRect& r)
            {
                D3D11_RECT rc = {(LONG)std::max(0.0f, r.x0), (LONG)std::max(0.0f, r.y0), (LONG)std::min((float)rtW_, r.x1), (LONG)std::min((float)rtH_, r.y1)};
                ctx_->RSSetScissorRects(1, &rc);
            }

            // Effects compile on a worker thread (D3DCompile + CreatePixelShader are free-threaded): until the
            // shader is ready the shape renders with the built-in shader, so a new effect never stalls a frame.
            void StartCompile(Effect11& e)
            {
                ComPtr<ID3D11Device> device = device_;
                if (!device || e.source.empty())
                    return;
                e.job = std::async(std::launch::async, [device, name = e.name, src = e.source]() {
                    CompiledEffect11 r;
                    EffectBytecode bc;
                    r.ok = CompileUserEffect(name, src, bc) && SUCCEEDED(device->CreatePixelShader(bc.code.data(), bc.code.size(), nullptr, &r.ps));
                    r.error = bc.error;
                    return r;
                });
            }

            ID3D11PixelShader* GetEffectShader(EffectId id)
            {
                std::lock_guard lock(effectMutex_);
                auto it = effects_.find(id);
                if (it == effects_.end())
                    return nullptr;
                Effect11& e = it->second;
                if (e.compiled || e.failed)
                    return e.ps.Get();
                if (!e.job.valid())
                {
                    StartCompile(e);
                    return nullptr;
                }
                if (e.job.wait_for(std::chrono::seconds(0)) != std::future_status::ready)
                    return nullptr;
                CompiledEffect11 r = e.job.get();
                e.compiled = r.ok;
                e.failed = !r.ok;
                e.ps = r.ps;
                if (!r.ok)
                    Log(2, "WGT effect '" + e.name + "' failed to compile:\n" + r.error);
                return e.ps.Get();
            }

            // -------------------------------------------- post passes
            void UnbindShaderInputs()
            {
                ID3D11ShaderResourceView* nulls[kBackdropLevels + 1] = {};
                ctx_->PSSetShaderResources(0, kBackdropLevels + 1, nulls);
                boundTex_ = nullptr;
                backdropBound_ = false;
            }

            // Downsamples `source` (full res) into levels 1..levels inside `region` (pixels).
            void BuildPyramid(ID3D11ShaderResourceView* source, const PxRect& region, int levels)
            {
                ctx_->IASetInputLayout(nullptr);
                ctx_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
                ctx_->VSSetShader(fullscreenVS_.Get(), nullptr, 0);
                ctx_->PSSetShader(downsamplePS_.Get(), nullptr, 0);
                const float bf[4] = {0, 0, 0, 0};
                ctx_->OMSetBlendState(blendOpaque_.Get(), bf, 0xFFFFFFFFu);
                ID3D11ShaderResourceView* src = source;
                int srcW = rtW_, srcH = rtH_;
                PyramidStep step = PyramidStep::First(region, rtW_, rtH_);
                for (int l = 1; l <= std::min(levels, kBackdropLevels - 1); ++l)
                {
                    Surface11& dst = levels_[l];
                    ID3D11ShaderResourceView* nullSrv = nullptr;
                    ctx_->PSSetShaderResources(0, 1, &nullSrv);
                    ID3D11RenderTargetView* rtv = dst.rtv.Get();
                    ctx_->OMSetRenderTargets(1, &rtv, nullptr);
                    D3D11_VIEWPORT vp = {0, 0, (float)dst.w, (float)dst.h, 0, 1};
                    ctx_->RSSetViewports(1, &vp);
                    const PxRect written = step.Next(region, l, dst.w, dst.h);
                    D3D11_RECT sc = {(LONG)written.x0, (LONG)written.y0, (LONG)written.x1, (LONG)written.y1};
                    ctx_->RSSetScissorRects(1, &sc);
                    PassConstants pc = step.Constants(srcW, srcH);
                    UpdateCB(passCB_.Get(), &pc, sizeof(pc));
                    ctx_->PSSetShaderResources(0, 1, &src);
                    ctx_->Draw(3, 0);
                    ++statDraws_;
                    step.valid = written;
                    src = dst.srv.Get();
                    srcW = dst.w;
                    srcH = dst.h;
                }
                ID3D11ShaderResourceView* nullSrv = nullptr;
                ctx_->PSSetShaderResources(0, 1, &nullSrv);
            }

            // The render target as a shader resource, when the host created it with one (engine targets usually are;
            // swap chains with DXGI_USAGE_SHADER_INPUT) and it reads the same values as the copy. Made per frame and
            // released at its end: a view kept on a swap chain buffer would block the host's ResizeBuffers.
            ID3D11ShaderResourceView* TargetSrv()
            {
                if (targetSrvTried_)
                    return targetSrv_.Get();
                targetSrvTried_ = true;
                D3D11_TEXTURE2D_DESC d;
                rtTex_->GetDesc(&d);
                const DXGI_FORMAT view = CopyViewFormat(CopyStorageFormat(rtFormat_));
                if (d.SampleDesc.Count != 1 || !(d.BindFlags & D3D11_BIND_SHADER_RESOURCE) || (rtFormat_ != view && rtFormat_ != CopyStorageFormat(rtFormat_)))
                    return nullptr;
                D3D11_SHADER_RESOURCE_VIEW_DESC sd = {};
                sd.Format = view;
                sd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
                sd.Texture2D.MipLevels = 1;
                device_->CreateShaderResourceView(rtTex_, &sd, &targetSrv_);
                return targetSrv_.Get();
            }

            void CaptureBackdrop(const PxRect& wanted, int levels, bool needsLevel0)
            {
                if (!surfacesReady_)
                    return;
                const PxRect region = AlignCaptureRegion(wanted, rtW_, rtH_);
                if (region.Empty())
                    return;
                const int timing = gpuTimer_.Begin(ctx_.Get());
                UnbindShaderInputs();
                ctx_->OMSetRenderTargets(0, nullptr, nullptr);
                // Glass that only reads blurred levels (frost) needs no full-resolution copy: the first downsample
                // reads the render target directly - same values, a copy's bandwidth saved
                ID3D11ShaderResourceView* direct = (!needsLevel0 && rtSamples_ == 1) ? TargetSrv() : nullptr;
                if (direct)
                {
                    BuildPyramid(direct, region, levels);
                    SetupCommonState(layerDepth_ > 0);
                    gpuTimer_.End(ctx_.Get(), timing, GpuTimer11::Glass);
                    return;
                }
                if (rtSamples_ > 1)
                    ctx_->ResolveSubresource(copy_.tex.Get(), 0, rtTex_, 0, ResolveFormat(CopyViewFormat(CopyStorageFormat(rtFormat_))));
                else
                {
                    D3D11_BOX box = {(UINT)region.x0, (UINT)region.y0, 0, (UINT)region.x1, (UINT)region.y1, 1};
                    ctx_->CopySubresourceRegion(copy_.tex.Get(), 0, (UINT)region.x0, (UINT)region.y0, 0, rtTex_, 0, &box);
                }
                BuildPyramid(copy_.srv.Get(), region, levels);
                SetupCommonState(layerDepth_ > 0);
                gpuTimer_.End(ctx_.Get(), timing, GpuTimer11::Glass);
            }

            void BeginLayer(const RenderOp& op)
            {
                if (layerDepth_++ > 0 || !surfacesReady_)
                    return;
                UnbindShaderInputs();
                // clear only the region the layer will be sampled in (the pyramid clamps to it)
                const float clear[4] = {0, 0, 0, 0};
                const PxRect region = AlignCaptureRegion(op.bounds, rtW_, rtH_);
                if (ctx1_)
                {
                    const D3D11_RECT rc = {(LONG)region.x0, (LONG)region.y0, (LONG)region.x1, (LONG)region.y1};
                    ctx1_->ClearView(layer_.rtv.Get(), clear, &rc, 1);
                }
                else
                    ctx_->ClearRenderTargetView(layer_.rtv.Get(), clear);
                currentRtv_ = layer_.rtv.Get();
                activeLayer_ = op.layer;
                SetupCommonState(true);
            }

            void EndLayer(const RenderOp& op)
            {
                if (layerDepth_ == 0)
                    return;
                if (--layerDepth_ > 0 || !surfacesReady_)
                    return;
                const int timing = gpuTimer_.Begin(ctx_.Get());
                UnbindShaderInputs();
                const PxRect region = AlignCaptureRegion(op.bounds, rtW_, rtH_);
                BuildPyramid(layer_.srv.Get(), region, LevelsForBloom(op.blurPx));

                currentRtv_ = mainRtv_;
                SetupCommonState(false);
                ctx_->IASetInputLayout(nullptr);
                ctx_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
                ctx_->VSSetShader(fullscreenVS_.Get(), nullptr, 0);
                ctx_->PSSetShader(layerPS_.Get(), nullptr, 0);
                const float bf[4] = {0, 0, 0, 0};
                ctx_->OMSetBlendState(blendPremul_.Get(), bf, 0xFFFFFFFFu);
                PassConstants pc = {{activeLayer_.color[0], activeLayer_.color[1], activeLayer_.color[2], activeLayer_.color[3]},
                                    {activeLayer_.intensity, activeLayer_.radius, activeLayer_.opacity, 0}};
                UpdateCB(passCB_.Get(), &pc, sizeof(pc));
                ID3D11ShaderResourceView* srvs[kBackdropLevels + 1] = {layer_.srv.Get(), copy_.srv.Get()};
                for (int l = 1; l < kBackdropLevels; ++l)
                    srvs[l + 1] = levels_[l].srv.Get();
                ctx_->PSSetShaderResources(0, kBackdropLevels + 1, srvs);
                SetScissor(op.bounds);
                ctx_->Draw(3, 0);
                ++statDraws_;
                ++statLayers_;
                UnbindShaderInputs();
                pipe_ = Pipe::None;
                gpuTimer_.End(ctx_.Get(), timing, GpuTimer11::Layer);
            }

            // -------------------------------------------------- execute
            // WGT_GPU_PROFILE: times runs of consecutive ops of one kind (-1 ends the current run)
            void ProfileRun(int kind)
            {
                if (!profile_ || kind == runKind_)
                    return;
                if (runKind_ >= 0)
                    gpuTimer_.End(ctx_.Get(), runStart_, (GpuTimer11::Kind)runKind_);
                runKind_ = kind;
                runStart_ = kind >= 0 ? gpuTimer_.Begin(ctx_.Get()) : -1;
            }

            void Execute()
            {
                ImGuiPlatformIO& pio = ImGui::GetPlatformIO();
                runKind_ = -1;
                for (const RenderOp& op : plan_.ops)
                {
                    switch (op.type)
                    {
                    case RenderOp::Draw:
                        ProfileRun(GpuTimer11::Imgui);
                        BindImGuiPipe(op);
                        BindTexture(op.texture);
                        SetScissor(op.clip);
                        SetFade(op.fade);
                        ctx_->DrawIndexed(op.idxCount, op.idxOffset, (INT)op.vtxOffset);
                        ++statDraws_;
                        break;

                    case RenderOp::FxBatch:
                        // captures are planned per frame (FramePlan::PlanCaptures): one serves many glass batches
                        if (op.glass && surfacesReady_ && !op.captureRegion.Empty())
                        {
                            if (captures_ < captureBudget_)
                            {
                                ProfileRun(-1);
                                if (profile_ && profFrames_ == 100)
                                {
                                    const PxRect a = AlignCaptureRegion(op.captureRegion, rtW_, rtH_);
                                    char line[160];
                                    std::snprintf(line, sizeof(line), "WGT capture %d: %.0fx%.0f at (%.0f,%.0f) levels %d | own glass region %.0fx%.0f at (%.0f,%.0f)", captures_, a.x1 - a.x0, a.y1 - a.y0, a.x0, a.y0, op.captureLevels, op.glassRegion.x1 - op.glassRegion.x0, op.glassRegion.y1 - op.glassRegion.y0, op.glassRegion.x0, op.glassRegion.y0);
                                    Log(2, line);
                                }
                                CaptureBackdrop(op.captureRegion, op.captureLevels, op.captureLevel0);
                                ++captures_;
                            }
                            else
                                overBudget_ = true;
                        }
                        ProfileRun(op.glass ? GpuTimer11::FxGlass : GpuTimer11::Fx);
                        BindFxPipe(op.effect);
                        BindTexture(op.texture);
                        SetScissor(op.clip);
                        SetFade(op.fade);
                        ctx_->DrawInstanced(4, op.instCount, 0, op.instStart);
                        ++statDraws_;
                        break;

                    case RenderOp::LayerBegin:
                        ProfileRun(-1);
                        BeginLayer(op);
                        break;

                    case RenderOp::LayerEnd:
                        ProfileRun(-1);
                        EndLayer(op);
                        break;

                    case RenderOp::ResetState:
                        SetupCommonState(layerDepth_ > 0);
                        break;

                    case RenderOp::UserCallback:
                        SetScissor(op.clip);
                        pio.Renderer_RenderState = ctx_.Get();
                        op.cmd->UserCallback(op.list, op.cmd);
                        pio.Renderer_RenderState = nullptr;
                        SetupCommonState(layerDepth_ > 0);
                        break;
                    }
                }
                ProfileRun(-1);
                while (layerDepth_ > 0)
                {
                    RenderOp end;
                    end.type = RenderOp::LayerEnd;
                    end.bounds = {0, 0, (float)rtW_, (float)rtH_};
                    layerDepth_ = 1;
                    EndLayer(end);
                }
            }

            // -------------------------------------------------- members
            ComPtr<ID3D11Device> device_;
            ComPtr<ID3D11DeviceContext> ctx_;
            ComPtr<ID3D11DeviceContext1> ctx1_;
            bool restoreState_ = true;

            ComPtr<ID3D11VertexShader> imguiVS_, fxVS_, fullscreenVS_;
            ComPtr<ID3D11PixelShader> imguiPS_, imguiTextPS_, imguiTextLcdPS_, imguiTextLcdGrayPS_, fxPS_, downsamplePS_, layerPS_;
            int coverageBound_ = -1;
            ComPtr<ID3D11InputLayout> imguiLayout_, fxLayout_;
            ComPtr<ID3D11Buffer> frameCB_, frameCBLayer_, passCB_, drawCB_;
            float drawFade_[4] = {-1, -1, -1, -1};   // edge fade currently in drawCB_
            ComPtr<ID3D11Buffer> vb_, ib_, instVB_;
            int vbCap_ = 0, ibCap_ = 0, instCap_ = 0;
            ComPtr<ID3D11BlendState> blendStraight_, blendPremul_, blendOpaque_, blendLcd_;
            ComPtr<ID3D11RasterizerState> raster_;
            ComPtr<ID3D11DepthStencilState> depth_;
            ComPtr<ID3D11SamplerState> samplerLinear_, samplerPoint_;
            ComPtr<ID3D11ShaderResourceView> whiteSrv_;

            Surface11 copy_;
            Surface11 levels_[kBackdropLevels];
            Surface11 layer_;
            int surfW_ = 0, surfH_ = 0;
            DXGI_FORMAT surfFormat_ = DXGI_FORMAT_UNKNOWN;
            bool surfacesReady_ = false;

            // per-frame
            FramePlan plan_;
            int captures_ = 0, captureBudget_ = 64;
            bool overBudget_ = false;
            const ImDrawData* display_ = nullptr;
            ID3D11RenderTargetView* mainRtv_ = nullptr;
            ID3D11RenderTargetView* currentRtv_ = nullptr;
            ID3D11Texture2D* rtTex_ = nullptr;
            int rtW_ = 0, rtH_ = 0;
            UINT rtSamples_ = 1;
            DXGI_FORMAT rtFormat_ = DXGI_FORMAT_UNKNOWN;
            bool outputLinear_ = false;
            Pipe pipe_ = Pipe::None;
            ID3D11ShaderResourceView* boundTex_ = nullptr;
            EffectId boundEffect_ = ~0u;
            bool backdropBound_ = false;
            int layerDepth_ = 0;
            fx::LayerParams activeLayer_{};

            std::mutex effectMutex_;
            std::unordered_map<EffectId, Effect11> effects_;
            std::mutex texMutex_;
            std::unordered_map<ImTextureID, ComPtr<ID3D11ShaderResourceView>> userTextures_;
            std::vector<ComPtr<ID3D11ShaderResourceView>> pendingRelease_;

            void (*log_)(void*, int, const char*) = nullptr;
            void* logUser_ = nullptr;
            int statDraws_ = 0, statFx_ = 0, statCaptures_ = 0, statLayers_ = 0, statVerts_ = 0;
            GpuTimer11 gpuTimer_;
            ComPtr<ID3D11ShaderResourceView> targetSrv_;   // this frame's render target as a shader resource (TargetSrv)
            bool targetSrvTried_ = false;
            bool profile_ = std::getenv("WGT_GPU_PROFILE") != nullptr;   // per-kind GPU breakdown in the log
            int runKind_ = -1, runStart_ = -1;
            int profFrames_ = 0;
            double profAcc_[GpuTimer11::KindCount + 1] = {};
            bool warnedBudget_ = false;
        };
    }

    std::unique_ptr<IRenderBackend> CreateD3D11Backend(ID3D11Device* device, ID3D11DeviceContext* context, bool restoreState)
    {
        return std::make_unique<D3D11Backend>(device, context, restoreState);
    }
}
