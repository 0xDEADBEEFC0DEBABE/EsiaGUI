// Esia - runtime HLSL compilation for the Direct3D backends (see d3d_shader.hpp)
#include "d3d_shader.hpp"
#include "esia/rhi/d3d_common.hpp"
#include <d3dcompiler.h>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <condition_variable>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <thread>

namespace esia::rhi::d3d
{
    namespace
    {
        // D3DDisassemble's and D3DPreprocess's types, declared here (not every SDK's d3dcompiler.h names them)
        using DisassembleFn = HRESULT(WINAPI*)(LPCVOID data, SIZE_T size, UINT flags, LPCSTR comments, ID3DBlob** out);
        using PreprocessFn = HRESULT(WINAPI*)(LPCVOID data, SIZE_T size, LPCSTR name, const D3D_SHADER_MACRO* defines, ID3DInclude* include,
                                              ID3DBlob** out, ID3DBlob** errors);

        // Loaded at runtime rather than linked: a missing DLL then fails device creation with a message (the
        // conformance suite reports SKIP) instead of the process failing to start.
        struct CompilerDll
        {
            HMODULE module = nullptr;
            pD3DCompile compile = nullptr;
            DisassembleFn disassemble = nullptr;
            PreprocessFn preprocess = nullptr;
            std::string identity;   // the DLL's path, size and time: a new compiler invalidates the disk cache
            std::string error;

            CompilerDll()
            {
                module = LoadLibraryW(L"d3dcompiler_47.dll");
                if (!module)
                {
                    error = "d3dcompiler_47.dll could not be loaded";
                    return;
                }
                compile = reinterpret_cast<pD3DCompile>(reinterpret_cast<void*>(GetProcAddress(module, "D3DCompile")));
                disassemble = reinterpret_cast<DisassembleFn>(reinterpret_cast<void*>(GetProcAddress(module, "D3DDisassemble")));
                preprocess = reinterpret_cast<PreprocessFn>(reinterpret_cast<void*>(GetProcAddress(module, "D3DPreprocess")));
                if (!compile)
                    error = "d3dcompiler_47.dll has no D3DCompile";
                wchar_t path[MAX_PATH] = {};
                WIN32_FILE_ATTRIBUTE_DATA a = {};
                if (GetModuleFileNameW(module, path, MAX_PATH) && GetFileAttributesExW(path, GetFileExInfoStandard, &a))
                {
                    char buf[64];
                    std::snprintf(buf, sizeof(buf), "%lu:%lu:%lu:%lu", (unsigned long)a.nFileSizeHigh, (unsigned long)a.nFileSizeLow,
                                  (unsigned long)a.ftLastWriteTime.dwHighDateTime, (unsigned long)a.ftLastWriteTime.dwLowDateTime);
                    identity = buf;
                    for (const wchar_t* c = path; *c; ++c)
                        identity += (char)(*c & 0x7F);
                }
            }
        };

        CompilerDll& Dll()
        {
            static CompilerDll dll;
            return dll;
        }

        // Serves #include "esia_common.hlsli" (the embedded sources) and "esia_user_effect.hlsli" (the effect).
        class Includes final : public ID3DInclude
        {
        public:
            explicit Includes(const ShaderRequest& r) : request_(r) {}

            HRESULT STDMETHODCALLTYPE Open(D3D_INCLUDE_TYPE, LPCSTR name, LPCVOID, LPCVOID* data, UINT* bytes) noexcept override
            {
                const char* text = nullptr;
                if (std::strcmp(name, "esia_user_effect.hlsli") == 0)
                    text = request_.effectSource;
                else
                    text = shaders::FindSource(name);
                if (!text)
                    return E_FAIL;
                files_.push_back(std::make_unique<std::string>(text));
                *data = files_.back()->data();
                *bytes = (UINT)files_.back()->size();
                return S_OK;
            }

            HRESULT STDMETHODCALLTYPE Close(LPCVOID) noexcept override { return S_OK; }

        private:
            const ShaderRequest& request_;
            std::vector<std::unique_ptr<std::string>> files_;
        };

        const char* EntryOf(const ShaderRequest& r)
        {
            const shaders::ProgramSource src = shaders::SourceOf(r.program);
            return r.stage == shaders::Stage::Vertex ? src.vertexEntry : src.pixelEntry;
        }

        // Everything that changes the bytecode. The vertex shader of an effect or of a feature variant does not
        // see the effect, so it shares the plain one's key (and bytecode).
        std::string KeyOf(const ShaderRequest& r)
        {
            const bool fx = r.program == ShaderProgram::Fx;
            std::string k = ShaderProfile(r.model, r.stage);
            k += ':';
            k += shaders::SourceOf(r.program).file;
            k += ':';
            k += EntryOf(r);
            if (fx && r.fxFeatures)
                k += ":f" + std::to_string(r.fxFeatures);
            if (fx && r.effectSource && r.stage == shaders::Stage::Pixel)
                k += std::string(":e") + r.effectSource;
            if (fx && r.instanceAttribute && r.stage == shaders::Stage::Vertex)
                k += ":i";
            return k;
        }

        // ------------------------------------------------------------------ disk cache
        // A shader is found by what makes its bytecode: the preprocessed source (every include and macro expanded),
        // the entry, the profile, the flags and the compiler. File <hash>.dxbc: "ESIADXBC", a second hash of the
        // same text (a collision of the first cannot load a wrong shader), the size, the bytecode.
        struct DiskCache
        {
            std::mutex mutex;
            std::filesystem::path dir;   // empty: no cache
        };

        DiskCache& Disk()
        {
            static DiskCache* d = new DiskCache;   // never destroyed: workers may still compile at exit
            return *d;
        }

        std::filesystem::path DiskDir()
        {
            DiskCache& d = Disk();
            std::lock_guard lock(d.mutex);
            return d.dir;
        }

        std::uint64_t Fnv1a(const std::string& text, std::uint64_t h)
        {
            for (const char c : text)
                h = (h ^ (std::uint8_t)c) * 1099511628211ull;
            return h;
        }

        constexpr char kDiskMagic[8] = {'E', 'S', 'I', 'A', 'D', 'X', 'B', 'C'};

        Bytecode DiskLoad(const std::filesystem::path& file, std::uint64_t check)
        {
            std::ifstream in(file, std::ios::binary);
            char magic[8] = {};
            std::uint64_t storedCheck = 0;
            std::uint32_t size = 0;
            if (!in.read(magic, 8) || std::memcmp(magic, kDiskMagic, 8) != 0 || !in.read(reinterpret_cast<char*>(&storedCheck), 8) ||
                storedCheck != check || !in.read(reinterpret_cast<char*>(&size), 4) || size == 0 || size > (64u << 20))
                return {};
            auto code = std::make_unique<std::vector<std::uint8_t>>(size);
            if (!in.read(reinterpret_cast<char*>(code->data()), size))
                return {};
            return Bytecode(code.release());
        }

        void DiskStore(const std::filesystem::path& file, std::uint64_t check, const std::vector<std::uint8_t>& code)
        {
            std::error_code ec;
            std::filesystem::create_directories(file.parent_path(), ec);
            // written beside and renamed over: a process reading it never sees half a file
            std::filesystem::path tmp = file;
            tmp += "." + std::to_string(GetCurrentProcessId()) + "." + std::to_string(GetCurrentThreadId()) + ".tmp";
            {
                std::ofstream out(tmp, std::ios::binary | std::ios::trunc);
                const std::uint32_t size = (std::uint32_t)code.size();
                out.write(kDiskMagic, 8);
                out.write(reinterpret_cast<const char*>(&check), 8);
                out.write(reinterpret_cast<const char*>(&size), 4);
                out.write(reinterpret_cast<const char*>(code.data()), (std::streamsize)code.size());
                if (!out)
                {
                    out.close();
                    std::filesystem::remove(tmp, ec);
                    return;
                }
            }
            if (!MoveFileExW(tmp.c_str(), file.c_str(), MOVEFILE_REPLACE_EXISTING))
                std::filesystem::remove(tmp, ec);
        }

        // What a compile produced, and the message to log for it (effects compile on workers, which never call the
        // host's log callback: the device that asks next logs it, on the render thread).
        struct Compiled
        {
            Bytecode code;
            LogLevel level = LogLevel::Info;
            std::string message;
        };

        void Report(const Compiled& c, const Logger& log)
        {
            if (!c.message.empty() && (c.level != LogLevel::Info || log.DebugLayer()))
                log.Log(c.level, c.message);
        }

        Compiled Compile(const ShaderRequest& r)
        {
            Compiled out;
            CompilerDll& dll = Dll();
            if (!dll.compile)
            {
                out.level = LogLevel::Error;
                out.message = dll.error;
                return out;
            }
            const shaders::ProgramSource src = shaders::SourceOf(r.program);
            const char* text = shaders::FindSource(src.file);
            if (!text)
                return out;
            std::string source;
            if (r.prelude)
                source = std::string(r.prelude) + "\n#line 1 \"" + src.file + "\"\n";
            source += text;

            const bool fx = r.program == ShaderProgram::Fx;
            const std::string features = std::to_string(r.fxFeatures) + "u";
            std::vector<D3D_SHADER_MACRO> macros;
            if (r.model != ShaderModel::Sm5)
                macros.push_back({"ESIA_FX_STORAGE_TEXTURE", "1"});
            if (fx && r.fxFeatures)
                macros.push_back({"ESIA_FX_FEATURES", features.c_str()});
            const bool effect = fx && r.effectSource && r.stage == shaders::Stage::Pixel;
            if (effect)
                macros.push_back({"ESIA_CUSTOM_EFFECT", "1"});
            if (fx && r.instanceAttribute && r.stage == shaders::Stage::Vertex)
                macros.push_back({"ESIA_INSTANCE_ATTRIBUTE", "1"});
            macros.push_back({nullptr, nullptr});

            // as tools/shaders/build_shaders.py runs fxc: /O3, /Gec for SM3 (legacy syntax), /Ges otherwise
            UINT flags = D3DCOMPILE_OPTIMIZATION_LEVEL3;
            flags |= r.model == ShaderModel::Sm3 ? D3DCOMPILE_ENABLE_BACKWARDS_COMPATIBILITY : D3DCOMPILE_ENABLE_STRICTNESS;
            char variant[32] = "";
            if (fx && r.fxFeatures)
                std::snprintf(variant, sizeof(variant), ", features 0x%x", r.fxFeatures);
            char what[128];
            std::snprintf(what, sizeof(what), "%s (%s%s%s)", EntryOf(r), ShaderProfile(r.model, r.stage), variant, effect ? ", user effect" : "");
            const auto t0 = std::chrono::steady_clock::now();

            // the disk cache: keyed by the preprocessed text (a few milliseconds; a compile takes hundreds)
            const std::filesystem::path dir = DiskDir();
            std::filesystem::path cached;
            std::uint64_t check = 0;
            if (!dir.empty() && dll.preprocess)
            {
                Includes preIncludes(r);
                ComPtr<ID3DBlob> pre, preErrors;
                if (SUCCEEDED(dll.preprocess(source.data(), source.size(), src.file, macros.data(), &preIncludes, &pre, &preErrors)) && pre)
                {
                    std::string key = "esia-dxbc-1|" + dll.identity + "|" + EntryOf(r) + "|" + ShaderProfile(r.model, r.stage) + "|" +
                                      std::to_string(flags) + "|";
                    key.append(static_cast<const char*>(pre->GetBufferPointer()), pre->GetBufferSize());
                    char name[32];
                    std::snprintf(name, sizeof(name), "%016llx.dxbc", (unsigned long long)Fnv1a(key, 14695981039346656037ull));
                    cached = dir / name;
                    check = Fnv1a(key, 0x9E3779B97F4A7C15ull);
                    if (Bytecode code = DiskLoad(cached, check))
                    {
                        const double ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t0).count();
                        char head[192];
                        std::snprintf(head, sizeof(head), "loaded %s from the shader cache in %.0f ms", what, ms);
                        out.message = head;
                        out.code = std::move(code);
                        return out;
                    }
                }
            }

            Includes includes(r);
            ComPtr<ID3DBlob> code, errors;
            const HRESULT hr = dll.compile(source.data(), source.size(), src.file, macros.data(), &includes, EntryOf(r),
                                           ShaderProfile(r.model, r.stage), flags, 0, &code, &errors);
            const double ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t0).count();
            if (FAILED(hr) || !code)
            {
                char head[192];
                std::snprintf(head, sizeof(head), "D3DCompile %s failed (0x%08lX): ", what, (unsigned long)hr);
                out.level = LogLevel::Error;
                out.message = std::string(head) + (errors ? static_cast<const char*>(errors->GetBufferPointer()) : "no message");
                return out;
            }
            char head[192];
            std::snprintf(head, sizeof(head), "compiled %s in %.0f ms", what, ms);
            out.message = std::string(head) + (errors ? std::string(": ") + static_cast<const char*>(errors->GetBufferPointer()) : std::string());
            const auto* b = static_cast<const std::uint8_t*>(code->GetBufferPointer());
            out.code = Bytecode(new std::vector<std::uint8_t>(b, b + code->GetBufferSize()));
            if (!cached.empty())
                DiskStore(cached, check, *out.code);
            return out;
        }

        // One entry per key, shared by every device of the process: a compile in progress (on this thread, another
        // device's, or an effect worker) is waited for, never started twice. No std::future / std::make_shared:
        // with clang and MinGW's libstdc++ 13 they drag in a duplicate type_info::operator== at link time.
        struct Entry
        {
            bool done = false;
            bool reported = false;   // a worker's message was logged
            Compiled result;
        };

        struct Cache
        {
            std::mutex mutex;
            std::condition_variable finished;
            std::unordered_map<std::string, std::shared_ptr<Entry>> entries;
        };

        // Never destroyed: an effect worker may still be compiling while the process exits.
        Cache& TheCache()
        {
            static Cache* c = new Cache;
            return *c;
        }

        void Finish(Cache& c, Entry& e, Compiled result)
        {
            {
                std::lock_guard lock(c.mutex);
                e.result = std::move(result);
                e.done = true;
            }
            c.finished.notify_all();
        }
    }

    const char* ShaderProfile(ShaderModel model, shaders::Stage stage)
    {
        const bool vs = stage == shaders::Stage::Vertex;
        switch (model)
        {
        case ShaderModel::Sm3: return vs ? "vs_3_0" : "ps_3_0";
        case ShaderModel::Sm4: return vs ? "vs_4_0" : "ps_4_0";
        case ShaderModel::Sm5: return vs ? "vs_5_0" : "ps_5_0";
        }
        return "";
    }

    bool ShaderCompilerAvailable(std::string& error)
    {
        CompilerDll& dll = Dll();
        error = dll.error;
        return dll.compile != nullptr;
    }

    Bytecode CompileShader(const ShaderRequest& request, const Logger& log)
    {
        Cache& c = TheCache();
        const std::string key = KeyOf(request);
        std::shared_ptr<Entry> entry;
        {
            std::unique_lock lock(c.mutex);
            auto it = c.entries.find(key);
            if (it != c.entries.end())
            {
                entry = it->second;
                c.finished.wait(lock, [&] { return entry->done; });
                return entry->result.code;
            }
            entry.reset(new Entry);
            entry->reported = true;   // compiled here, on the caller's thread: logged right away
            c.entries.emplace(key, entry);
        }
        Compiled result = Compile(request);
        Report(result, log);
        Finish(c, *entry, std::move(result));
        return entry->result.code;
    }

    CompileState CompileShaderAsync(const ShaderRequest& request, const Logger& log, Bytecode& out)
    {
        Cache& c = TheCache();
        const std::string key = KeyOf(request);
        std::unique_lock lock(c.mutex);
        auto it = c.entries.find(key);
        if (it == c.entries.end())
        {
            std::shared_ptr<Entry> entry(new Entry);
            c.entries.emplace(key, entry);
            // the worker owns a copy of the effect source (the prelude is static data) and never logs:
            // the host's callback and its user pointer may be gone before a compile of seconds ends
            std::thread([r = request, source = std::string(request.effectSource ? request.effectSource : ""), entry]() mutable {
                r.effectSource = r.effectSource ? source.c_str() : nullptr;
                Finish(TheCache(), *entry, Compile(r));
            }).detach();
            return CompileState::Pending;
        }
        Entry& e = *it->second;
        if (!e.done)
            return CompileState::Pending;
        out = e.result.code;
        if (!std::exchange(e.reported, true))
        {
            const Compiled result = e.result;
            lock.unlock();   // the log callback may take its time (or compile something)
            Report(result, log);
        }
        return out ? CompileState::Ready : CompileState::Failed;
    }

    unsigned InstructionSlots(const Bytecode& code)
    {
        CompilerDll& dll = Dll();
        ComPtr<ID3DBlob> text;
        if (!code || !dll.disassemble || FAILED(dll.disassemble(code->data(), code->size(), 0, nullptr, &text)) || !text)
            return 0;
        const std::string listing(static_cast<const char*>(text->GetBufferPointer()), text->GetBufferSize());
        const char* const kMark = "// approximately ";
        const std::size_t at = listing.rfind(kMark);
        unsigned slots = 0;
        if (at == std::string::npos || std::sscanf(listing.c_str() + at + std::strlen(kMark), "%u instruction slots", &slots) != 1)
            return 0;
        return slots;
    }

    void SetShaderCacheDirectory(const std::string& utf8Directory)
    {
        DiskCache& d = Disk();
        std::lock_guard lock(d.mutex);
        d.dir = std::filesystem::path(std::u8string_view(reinterpret_cast<const char8_t*>(utf8Directory.data()), utf8Directory.size()));
    }

    void ForgetCompiledShaders()
    {
        Cache& c = TheCache();
        std::unique_lock lock(c.mutex);
        c.finished.wait(lock, [&] {
            for (const auto& [key, entry] : c.entries)
                if (!entry->done)
                    return false;
            return true;
        });
        c.entries.clear();
    }
}
