// Esia OpenGL backend - rhi::Device on OpenGL 3.3 core / OpenGL ES 3.0 (see gl_device.hpp)
#include "gl_device.hpp"
#include "esia/render/shader_library.hpp"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <unordered_set>

namespace esia::rhi::opengl
{
    namespace
    {
        struct FormatInfo
        {
            GLenum internal = 0, format = 0, type = 0;
        };

        // BGRA8 is only an encoding hint in GL (there is no BGRA internal format in GL 3.3 / GLES 3.0): it is stored
        // as RGBA, and uploads are swizzled on the CPU.
        FormatInfo Info(Format f)
        {
            switch (f)
            {
            case Format::RGBA8_UNORM:
            case Format::BGRA8_UNORM: return {GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE};
            case Format::RGBA8_SRGB:
            case Format::BGRA8_SRGB: return {GL_SRGB8_ALPHA8, GL_RGBA, GL_UNSIGNED_BYTE};
            case Format::RGB10A2_UNORM: return {GL_RGB10_A2, GL_RGBA, GL_UNSIGNED_INT_2_10_10_10_REV};
            case Format::RGBA16_FLOAT: return {GL_RGBA16F, GL_RGBA, GL_HALF_FLOAT};
            case Format::RGBA32_FLOAT: return {GL_RGBA32F, GL_RGBA, GL_FLOAT};
            case Format::R8_UNORM: return {GL_R8, GL_RED, GL_UNSIGNED_BYTE};
            case Format::Unknown: break;
            }
            return {};
        }

        bool IsBgra(Format f) { return f == Format::BGRA8_UNORM || f == Format::BGRA8_SRGB; }

        std::uint8_t Unorm8(float v)
        {
            v = std::isnan(v) ? 0.0f : std::clamp(v, 0.0f, 1.0f);
            return (std::uint8_t)std::lround(v * 255.0f);
        }

        void ESIA_GLAPI DebugCallback(GLenum, GLenum type, GLuint, GLenum severity, GLsizei, const GLchar* message, const void* user)
        {
            static_cast<GlDevice*>(const_cast<void*>(user))->OnDebugMessage(type, severity, message);
        }
    }

    // ------------------------------------------------------------------ setup
    GlDevice::GlDevice(const Desc& desc, std::unique_ptr<OwnedContext> owned) : desc_(desc), owned_(std::move(owned))
    {
        for (Program& p : programs_)
            std::fill(std::begin(p.sampler), std::end(p.sampler), -1);
        std::fill(std::begin(unitTex_), std::end(unitTex_), kUnknown);
        std::fill(std::begin(unitSampler_), std::end(unitSampler_), kUnknown);
    }

    bool GlDevice::Init(std::string& error)
    {
        const char* missing = nullptr;
        if (!gl_.Load(desc_.getProcAddress, missing))
        {
            error = std::string("OpenGL function not found: ") + missing;
            return false;
        }
        gl_.GetIntegerv(GL_MAJOR_VERSION, &glMajor_);
        gl_.GetIntegerv(GL_MINOR_VERSION, &glMinor_);
        const char* version = reinterpret_cast<const char*>(gl_.GetString(GL_VERSION));
        const bool esContext = version && std::strncmp(version, "OpenGL ES", 9) == 0;
        if (const char* renderer = reinterpret_cast<const char*>(gl_.GetString(GL_RENDERER)))
            renderer_ = renderer;
        const int v = glMajor_ * 10 + glMinor_;
        if (esContext != desc_.es || v < (desc_.es ? 30 : 33))
        {
            error = std::string("need ") + (desc_.es ? "OpenGL ES 3.0" : "OpenGL 3.3 core") + ", the context is " + (version ? version : "unknown");
            return false;
        }
        std::unordered_set<std::string> ext;
        GLint count = 0;
        gl_.GetIntegerv(GL_NUM_EXTENSIONS, &count);
        for (GLint i = 0; i < count; ++i)
            if (const GLubyte* e = gl_.GetStringi(GL_EXTENSIONS, (GLuint)i))
                ext.insert(reinterpret_cast<const char*>(e));
        // Desc::coreOnly: what GL 3.3 core / GLES 3.0 guarantee, nothing more - except KHR_debug, which only reports
        const bool extra = !desc_.coreOnly;
        auto has = [&](const char* name) { return extra && ext.count(name) != 0; };
        const bool khrDebug = (desc_.es ? v >= 32 : v >= 43) || ext.count("GL_KHR_debug");

        // optional entry points, by the name the version or extension defines
        if (desc_.es)
        {
            if (has("GL_EXT_disjoint_timer_query"))
            {
                gl_.LoadOptional(gl_.QueryCounter, "glQueryCounterEXT");
                gl_.LoadOptional(gl_.GetQueryObjectui64v, "glGetQueryObjectui64vEXT");
            }
            gl_.LoadOptional(gl_.InvalidateFramebuffer, "glInvalidateFramebuffer");   // GLES 3.0 core
            if (khrDebug)
            {
                const bool core = v >= 32;
                gl_.LoadOptional(gl_.DebugMessageCallback, core ? "glDebugMessageCallback" : "glDebugMessageCallbackKHR");
                gl_.LoadOptional(gl_.DebugMessageControl, core ? "glDebugMessageControl" : "glDebugMessageControlKHR");
                gl_.LoadOptional(gl_.ObjectLabel, core ? "glObjectLabel" : "glObjectLabelKHR");
            }
        }
        else
        {
            gl_.LoadOptional(gl_.QueryCounter, "glQueryCounter");
            gl_.LoadOptional(gl_.GetQueryObjectui64v, "glGetQueryObjectui64v");
            gl_.LoadOptional(gl_.PolygonMode, "glPolygonMode");
            if (extra && (v >= 43 || ext.count("GL_ARB_invalidate_subdata")))
                gl_.LoadOptional(gl_.InvalidateFramebuffer, "glInvalidateFramebuffer");
            if (khrDebug)
            {
                gl_.LoadOptional(gl_.DebugMessageCallback, "glDebugMessageCallback");
                gl_.LoadOptional(gl_.DebugMessageControl, "glDebugMessageControl");
                gl_.LoadOptional(gl_.ObjectLabel, "glObjectLabel");
            }
        }
        if (!gl_.PolygonMode && !desc_.es)
        {
            error = "OpenGL function not found: glPolygonMode";
            return false;
        }
        ready_ = true;

        OutsideFrame guard(*this);
        // NVIDIA's WGL ES profile lists EXT_disjoint_timer_query but rejects GL_GPU_DISJOINT_EXT: probe it before our
        // debug output is on (the error is cleared below) and treat a rejection as "never disjoint"
        if (desc_.es && gl_.QueryCounter)
        {
            while (gl_.GetError() != GL_NO_ERROR) {}
            GLint disjoint = 0;
            gl_.GetIntegerv(GL_GPU_DISJOINT_EXT, &disjoint);
            disjointQuery_ = gl_.GetError() == GL_NO_ERROR;
        }
        if (desc_.debug && gl_.DebugMessageCallback && gl_.DebugMessageControl)
        {
            gl_.Enable(GL_DEBUG_OUTPUT);
            gl_.Enable(GL_DEBUG_OUTPUT_SYNCHRONOUS);   // errors reported in the call that caused them
            gl_.DebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
            gl_.DebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE);
            gl_.DebugMessageCallback(&DebugCallback, this);
            debugOutput_ = true;
        }
        while (gl_.GetError() != GL_NO_ERROR) {}   // whatever the host left behind is not ours

        GLint maxTex = 0;
        gl_.GetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTex);
        gl_.GetIntegerv(GL_MAX_SAMPLES, &maxSamples_);
        gl_.GetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &uboAlign_);
        uboAlign_ = std::max<GLint>(uboAlign_, 16);
        srgbDecodeControl_ = has("GL_EXT_texture_sRGB_decode");

        caps_.fxStorage = FxStorage::Texture;
        caps_.shaderFormat = (std::uint8_t)(desc_.es ? shaders::Format::Essl300 : shaders::Format::Glsl330);
        caps_.framebufferOriginBottomLeft = true;
        caps_.clipSpaceYDown = false;
        caps_.halfPixelOffset = false;
        caps_.sampleRenderTarget = true;
        caps_.timestampQueries = gl_.QueryCounter && gl_.GetQueryObjectui64v;
        caps_.readback = true;
        caps_.runtimeEffects = false;
        caps_.fxFeatureVariants = false;
        caps_.maxTextureSize = maxTex;
        // the instance texture is perRow * 24 texels wide and uploaded a whole row at a time: 4096 texels (170
        // instances, 64 KB per row) keeps a frame with a few instances from uploading a 16384-wide row
        caps_.maxFxDataWidth = std::min(maxTex, 4096);
        // RGBA16F must be color-renderable: always on GL 3.3, on GLES only with EXT_color_buffer_(half_)float
        caps_.floatRenderTargets = !desc_.es || has("GL_EXT_color_buffer_half_float") || has("GL_EXT_color_buffer_float");
        if (caps_.floatRenderTargets)
        {
            Tex probe;
            probe.desc.width = probe.desc.height = 1;
            probe.desc.format = Format::RGBA16_FLOAT;
            probe.desc.usage = TextureUsage_RenderTarget;
            gl_.GenTextures(1, &probe.tex);
            BindUnit(kScratchUnit, probe.tex);
            gl_.TexImage2D(GL_TEXTURE_2D, 0, (GLint)GL_RGBA16F, 1, 1, 0, GL_RGBA, GL_HALF_FLOAT, nullptr);
            caps_.floatRenderTargets = AttachAndCheck(probe);
            ReleaseTex(probe);
        }

        gl_.GenSamplers(2, samplers_);
        for (int i = 0; i < 2; ++i)
        {
            const GLint filter = i == 0 ? (GLint)GL_LINEAR : (GLint)GL_NEAREST;
            gl_.SamplerParameteri(samplers_[i], GL_TEXTURE_MIN_FILTER, filter);
            gl_.SamplerParameteri(samplers_[i], GL_TEXTURE_MAG_FILTER, filter);
            gl_.SamplerParameteri(samplers_[i], GL_TEXTURE_WRAP_S, (GLint)GL_CLAMP_TO_EDGE);
            gl_.SamplerParameteri(samplers_[i], GL_TEXTURE_WRAP_T, (GLint)GL_CLAMP_TO_EDGE);
            // sampler state wins over texture state: sRGB render targets are read raw (the shaders decode nothing)
            if (srgbDecodeControl_)
                gl_.SamplerParameteri(samplers_[i], GL_TEXTURE_SRGB_DECODE_EXT, (GLint)GL_SKIP_DECODE_EXT);
        }

        gl_.GenVertexArrays(1, &uiVao_);
        gl_.GenVertexArrays(1, &emptyVao_);   // core profiles draw nothing without a vertex array object
        BindVertexArray(uiVao_);
        for (GLuint a = 0; a < 3; ++a)
            gl_.EnableVertexAttribArray(a);

        uboSize_ = 256 * 1024;
        gl_.GenBuffers(1, &ubo_);
        gl_.BindBuffer(GL_COPY_WRITE_BUFFER, ubo_);
        gl_.BufferData(GL_COPY_WRITE_BUFFER, uboSize_, nullptr, GL_STREAM_DRAW);
        Label(GL_BUFFER, ubo_, "esia-constants");

        if (caps_.timestampQueries)
            for (TimerSlot& s : timer_)
            {
                s.queries.resize(kMaxStamps);
                gl_.GenQueries(kMaxStamps, s.queries.data());
            }
        CheckErrors("device creation");
        return true;
    }

    GlDevice::~GlDevice()
    {
        if (!ready_)
            return;   // Init failed before creating anything
        if (owned_)
            owned_->MakeCurrent();
        OutsideFrame guard(*this);
        for (auto& [id, t] : textures_)
            ReleaseTex(t);
        ReleaseTex(resolve_);
        for (auto& [id, b] : buffers_)
            gl_.DeleteBuffers(1, &b.id);
        for (Program& p : programs_)
            if (p.id)
                gl_.DeleteProgram(p.id);
        if (copyProgram_)
            gl_.DeleteProgram(copyProgram_);
        if (samplers_[0])
            gl_.DeleteSamplers(2, samplers_);
        GLuint vaos[2] = {uiVao_, emptyVao_};
        gl_.DeleteVertexArrays(2, vaos);
        if (ubo_)
            gl_.DeleteBuffers(1, &ubo_);
        for (TimerSlot& s : timer_)
            if (!s.queries.empty())
                gl_.DeleteQueries((GLsizei)s.queries.size(), s.queries.data());
        if (debugOutput_)
            gl_.DebugMessageCallback(nullptr, nullptr);
        if (errors_ > 0)
            std::fprintf(stderr, "esia %s: %u GL error(s) during the device's lifetime\n", Name(), errors_);
    }

    void GlDevice::OnDebugMessage(GLenum type, GLenum severity, const char* message)
    {
        const bool error = type == GL_DEBUG_TYPE_ERROR;
        if (error)
            ++errors_;
        if (error || severity == GL_DEBUG_SEVERITY_HIGH || severity == GL_DEBUG_SEVERITY_MEDIUM)
            std::fprintf(stderr, "esia %s: GL %s: %s\n", Name(), error ? "error" : "warning", message);
    }

    void GlDevice::CheckErrors(const char* where)
    {
        if (!desc_.debug)
            return;
        // with KHR_debug the callback has reported (and counted) every error already
        for (GLenum e = gl_.GetError(); e != GL_NO_ERROR; e = gl_.GetError())
            if (!debugOutput_)
            {
                ++errors_;
                std::fprintf(stderr, "esia %s: GL error 0x%04X (%s)\n", Name(), e, where);
            }
    }

    void GlDevice::Label(GLenum type, GLuint id, const char* name)
    {
        if (debugOutput_ && gl_.ObjectLabel && name && id)
            gl_.ObjectLabel(type, id, -1, name);
    }

    // ------------------------------------------------------------------ host state
    GlDevice::OutsideFrame::OutsideFrame(GlDevice& d) : d_(d), active_(!d.inFrame_ && d.desc_.restoreHostState)
    {
        // between frames the host may have changed any binding the caches remember, and any fixed state
        if (d_.inFrame_)
            return;
        d_.ForgetGlState();
        if (active_)
            d_.SaveHostState(d_.frameState_);
        d_.ApplyFixedState();
    }

    GlDevice::OutsideFrame::~OutsideFrame()
    {
        if (active_)
            d_.RestoreHostState(d_.frameState_);
        if (!d_.inFrame_)
            d_.ForgetGlState();
    }

    void GlDevice::SaveHostState(HostState& s)
    {
        const GlApi& g = gl_;
        g.GetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &s.drawFbo);
        g.GetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &s.readFbo);
        g.GetIntegerv(GL_RENDERBUFFER_BINDING, &s.renderbuffer);
        g.GetIntegerv(GL_CURRENT_PROGRAM, &s.program);
        g.GetIntegerv(GL_VERTEX_ARRAY_BINDING, &s.vao);
        g.GetIntegerv(GL_ARRAY_BUFFER_BINDING, &s.arrayBuffer);
        g.GetIntegerv(GL_COPY_WRITE_BUFFER, &s.copyWriteBuffer);   // the enum doubles as its binding query
        g.GetIntegerv(GL_UNIFORM_BUFFER_BINDING, &s.uniformBuffer);
        g.GetIntegerv(GL_PIXEL_PACK_BUFFER_BINDING, &s.packBuffer);
        g.GetIntegerv(GL_PIXEL_UNPACK_BUFFER_BINDING, &s.unpackBuffer);
        g.GetIntegerv(GL_ACTIVE_TEXTURE, &s.activeTexture);
        for (int u = 0; u < kUnits; ++u)
        {
            g.ActiveTexture(GL_TEXTURE0 + (GLenum)u);
            g.GetIntegerv(GL_TEXTURE_BINDING_2D, &s.tex[u]);
            g.GetIntegerv(GL_SAMPLER_BINDING, &s.sampler[u]);
        }
        g.ActiveTexture((GLenum)s.activeTexture);
        for (GLuint i = 0; i < 3; ++i)
        {
            g.GetIntegeri_v(GL_UNIFORM_BUFFER_BINDING, i, &s.ubo[i]);
            g.GetInteger64i_v(GL_UNIFORM_BUFFER_START, i, &s.uboStart[i]);
            g.GetInteger64i_v(GL_UNIFORM_BUFFER_SIZE, i, &s.uboSize[i]);
        }
        g.GetIntegerv(GL_VIEWPORT, s.viewport);
        g.GetIntegerv(GL_SCISSOR_BOX, s.scissor);
        if (!desc_.es)
            g.GetIntegerv(GL_POLYGON_MODE, s.polygonMode);
        g.GetFloatv(GL_COLOR_CLEAR_VALUE, s.clearColor);
        g.GetBooleanv(GL_COLOR_WRITEMASK, s.colorMask);
        g.GetIntegerv(GL_BLEND_SRC_RGB, &s.blendSrcRgb);
        g.GetIntegerv(GL_BLEND_DST_RGB, &s.blendDstRgb);
        g.GetIntegerv(GL_BLEND_SRC_ALPHA, &s.blendSrcAlpha);
        g.GetIntegerv(GL_BLEND_DST_ALPHA, &s.blendDstAlpha);
        g.GetIntegerv(GL_BLEND_EQUATION_RGB, &s.blendEqRgb);
        g.GetIntegerv(GL_BLEND_EQUATION_ALPHA, &s.blendEqAlpha);
        s.blend = g.IsEnabled(GL_BLEND);
        s.scissorTest = g.IsEnabled(GL_SCISSOR_TEST);
        s.depthTest = g.IsEnabled(GL_DEPTH_TEST);
        s.stencilTest = g.IsEnabled(GL_STENCIL_TEST);
        s.cullFace = g.IsEnabled(GL_CULL_FACE);
        s.rasterizerDiscard = g.IsEnabled(GL_RASTERIZER_DISCARD);
        s.alphaToCoverage = g.IsEnabled(GL_SAMPLE_ALPHA_TO_COVERAGE);
        s.multisample = !desc_.es && g.IsEnabled(GL_MULTISAMPLE);
        s.srgb = !desc_.es && g.IsEnabled(GL_FRAMEBUFFER_SRGB);
        const GLenum pack[4] = {GL_PACK_ALIGNMENT, GL_PACK_ROW_LENGTH, GL_PACK_SKIP_ROWS, GL_PACK_SKIP_PIXELS};
        const GLenum unpack[4] = {GL_UNPACK_ALIGNMENT, GL_UNPACK_ROW_LENGTH, GL_UNPACK_SKIP_ROWS, GL_UNPACK_SKIP_PIXELS};
        for (int i = 0; i < 4; ++i)
        {
            g.GetIntegerv(pack[i], &s.pack[i]);
            g.GetIntegerv(unpack[i], &s.unpack[i]);
        }
    }

    void GlDevice::RestoreHostState(const HostState& s)
    {
        const GlApi& g = gl_;
        auto enable = [&](GLenum cap, bool on) { on ? g.Enable(cap) : g.Disable(cap); };
        g.UseProgram((GLuint)s.program);
        for (int u = 0; u < kUnits; ++u)
        {
            g.ActiveTexture(GL_TEXTURE0 + (GLenum)u);
            g.BindTexture(GL_TEXTURE_2D, (GLuint)s.tex[u]);
            g.BindSampler((GLuint)u, (GLuint)s.sampler[u]);
        }
        g.ActiveTexture((GLenum)s.activeTexture);
        g.BindVertexArray((GLuint)s.vao);
        g.BindBuffer(GL_ARRAY_BUFFER, (GLuint)s.arrayBuffer);
        g.BindBuffer(GL_COPY_WRITE_BUFFER, (GLuint)s.copyWriteBuffer);
        for (GLuint i = 0; i < 3; ++i)
        {
            if (s.ubo[i] != 0 && s.uboSize[i] > 0)
                g.BindBufferRange(GL_UNIFORM_BUFFER, i, (GLuint)s.ubo[i], (GLintptr)s.uboStart[i], (GLsizeiptr)s.uboSize[i]);
            else
                g.BindBufferBase(GL_UNIFORM_BUFFER, i, (GLuint)s.ubo[i]);
        }
        g.BindBuffer(GL_UNIFORM_BUFFER, (GLuint)s.uniformBuffer);   // after the indexed ones, which also set it
        g.BindBuffer(GL_PIXEL_PACK_BUFFER, (GLuint)s.packBuffer);
        g.BindBuffer(GL_PIXEL_UNPACK_BUFFER, (GLuint)s.unpackBuffer);
        g.BindFramebuffer(GL_READ_FRAMEBUFFER, (GLuint)s.readFbo);
        g.BindFramebuffer(GL_DRAW_FRAMEBUFFER, (GLuint)s.drawFbo);
        g.BindRenderbuffer(GL_RENDERBUFFER, (GLuint)s.renderbuffer);
        g.Viewport(s.viewport[0], s.viewport[1], s.viewport[2], s.viewport[3]);
        g.Scissor(s.scissor[0], s.scissor[1], s.scissor[2], s.scissor[3]);
        if (!desc_.es)
        {
            g.PolygonMode(GL_FRONT_AND_BACK, (GLenum)s.polygonMode[0]);
            enable(GL_MULTISAMPLE, s.multisample);
            enable(GL_FRAMEBUFFER_SRGB, s.srgb);
        }
        g.ClearColor(s.clearColor[0], s.clearColor[1], s.clearColor[2], s.clearColor[3]);
        g.ColorMask(s.colorMask[0], s.colorMask[1], s.colorMask[2], s.colorMask[3]);
        g.BlendFuncSeparate((GLenum)s.blendSrcRgb, (GLenum)s.blendDstRgb, (GLenum)s.blendSrcAlpha, (GLenum)s.blendDstAlpha);
        g.BlendEquationSeparate((GLenum)s.blendEqRgb, (GLenum)s.blendEqAlpha);
        enable(GL_BLEND, s.blend);
        enable(GL_SCISSOR_TEST, s.scissorTest);
        enable(GL_DEPTH_TEST, s.depthTest);
        enable(GL_STENCIL_TEST, s.stencilTest);
        enable(GL_CULL_FACE, s.cullFace);
        enable(GL_RASTERIZER_DISCARD, s.rasterizerDiscard);
        enable(GL_SAMPLE_ALPHA_TO_COVERAGE, s.alphaToCoverage);
        const GLenum pack[4] = {GL_PACK_ALIGNMENT, GL_PACK_ROW_LENGTH, GL_PACK_SKIP_ROWS, GL_PACK_SKIP_PIXELS};
        const GLenum unpack[4] = {GL_UNPACK_ALIGNMENT, GL_UNPACK_ROW_LENGTH, GL_UNPACK_SKIP_ROWS, GL_UNPACK_SKIP_PIXELS};
        for (int i = 0; i < 4; ++i)
        {
            g.PixelStorei(pack[i], s.pack[i]);
            g.PixelStorei(unpack[i], s.unpack[i]);
        }
    }

    void GlDevice::ForgetGlState()
    {
        drawFbo_ = readFbo_ = program_ = vao_ = activeUnit_ = vaoVb_ = vaoIb_ = kUnknown;
        std::fill(std::begin(unitTex_), std::end(unitTex_), kUnknown);
        std::fill(std::begin(unitSampler_), std::end(unitSampler_), kUnknown);
        scissorTest_ = srgbWrite_ = blend_ = -1;
    }

    // What every draw and copy of the device assumes, whatever the host left: no depth / stencil / culling, all
    // channels written, filled polygons, additive blending, tightly packed pixel transfers from client memory.
    void GlDevice::ApplyFixedState()
    {
        const GlApi& g = gl_;
        g.Disable(GL_DEPTH_TEST);
        g.Disable(GL_STENCIL_TEST);
        g.Disable(GL_CULL_FACE);
        g.Disable(GL_RASTERIZER_DISCARD);
        g.Disable(GL_SAMPLE_ALPHA_TO_COVERAGE);
        if (!desc_.es)
        {
            g.Enable(GL_MULTISAMPLE);
            g.PolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
        g.ColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        g.BlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
        g.BindBuffer(GL_PIXEL_PACK_BUFFER, 0);
        g.BindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
        for (GLenum p : {GL_PACK_ROW_LENGTH, GL_PACK_SKIP_ROWS, GL_PACK_SKIP_PIXELS, GL_UNPACK_ROW_LENGTH, GL_UNPACK_SKIP_ROWS, GL_UNPACK_SKIP_PIXELS})
            g.PixelStorei(p, 0);
        g.PixelStorei(GL_PACK_ALIGNMENT, 1);
        g.PixelStorei(GL_UNPACK_ALIGNMENT, 1);
    }

    void GlDevice::BindFramebuffer(GLenum target, GLuint fbo)
    {
        const bool draw = target == GL_FRAMEBUFFER || target == GL_DRAW_FRAMEBUFFER;
        const bool read = target == GL_FRAMEBUFFER || target == GL_READ_FRAMEBUFFER;
        if ((!draw || drawFbo_ == fbo) && (!read || readFbo_ == fbo))
            return;
        gl_.BindFramebuffer(target, fbo);
        if (draw)
            drawFbo_ = fbo;
        if (read)
            readFbo_ = fbo;
    }

    void GlDevice::BindUnit(int unit, GLuint tex)
    {
        if (unitTex_[unit] == tex)
            return;
        if (activeUnit_ != (GLuint)unit)
        {
            gl_.ActiveTexture(GL_TEXTURE0 + (GLenum)unit);
            activeUnit_ = (GLuint)unit;
        }
        gl_.BindTexture(GL_TEXTURE_2D, tex);
        unitTex_[unit] = tex;
    }

    void GlDevice::BindSampler(int unit, GLuint sampler)
    {
        if (unitSampler_[unit] != sampler)
        {
            gl_.BindSampler((GLuint)unit, sampler);
            unitSampler_[unit] = sampler;
        }
    }

    void GlDevice::SetScissorTest(bool on)
    {
        if (scissorTest_ != (int)on)
        {
            on ? gl_.Enable(GL_SCISSOR_TEST) : gl_.Disable(GL_SCISSOR_TEST);
            scissorTest_ = on;
        }
    }

    // Desktop GL encodes into sRGB attachments only with GL_FRAMEBUFFER_SRGB; GLES always does (nothing to set).
    void GlDevice::SetSrgbWrite(bool on)
    {
        if (!desc_.es && srgbWrite_ != (int)on)
        {
            on ? gl_.Enable(GL_FRAMEBUFFER_SRGB) : gl_.Disable(GL_FRAMEBUFFER_SRGB);
            srgbWrite_ = on;
        }
    }

    void GlDevice::SetBlend(BlendMode mode)
    {
        if (blend_ == (int)mode)
            return;
        blend_ = (int)mode;
        switch (mode)
        {
        case BlendMode::Opaque: gl_.Disable(GL_BLEND); return;
        case BlendMode::Straight: gl_.BlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA); break;
        case BlendMode::Premultiplied: gl_.BlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA); break;
        }
        gl_.Enable(GL_BLEND);
    }

    void GlDevice::UseProgram(GLuint program)
    {
        if (program_ != program)
        {
            gl_.UseProgram(program);
            program_ = program;
        }
    }

    void GlDevice::BindVertexArray(GLuint vao)
    {
        if (vao_ != vao)
        {
            gl_.BindVertexArray(vao);
            vao_ = vao;
        }
    }

    // ------------------------------------------------------------------ textures
    GlDevice::Tex* GlDevice::FindTex(Texture t)
    {
        auto it = textures_.find(t.id);
        return it != textures_.end() ? &it->second : nullptr;
    }

    const GlDevice::Tex* GlDevice::FindTex(Texture t) const
    {
        auto it = textures_.find(t.id);
        return it != textures_.end() ? &it->second : nullptr;
    }

    // Attaches the texture (or renderbuffer) to a new framebuffer; false if GL does not render into that format.
    bool GlDevice::AttachAndCheck(Tex& t)
    {
        gl_.GenFramebuffers(1, &t.fbo);
        BindFramebuffer(GL_DRAW_FRAMEBUFFER, t.fbo);
        if (t.rbo)
            gl_.FramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, t.rbo);
        else
            gl_.FramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, t.tex, 0);
        return gl_.CheckFramebufferStatus(GL_DRAW_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
    }

    GLuint GlDevice::EnsureFbo(Tex& t)
    {
        if (!t.wrapped && !t.fbo && !AttachAndCheck(t))
        {
            gl_.DeleteFramebuffers(1, &t.fbo);
            t.fbo = 0;
            BindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
            return kUnknown;
        }
        return t.fbo;
    }

    void GlDevice::ReleaseTex(Tex& t)
    {
        if (t.wrapped)
            return;
        for (GLuint& u : unitTex_)
            if (u == t.tex)
                u = kUnknown;   // GL unbinds a deleted texture from the current context's units
        if (t.fbo)
        {
            gl_.DeleteFramebuffers(1, &t.fbo);
            if (drawFbo_ == t.fbo)
                drawFbo_ = kUnknown;
            if (readFbo_ == t.fbo)
                readFbo_ = kUnknown;
        }
        if (t.tex)
            gl_.DeleteTextures(1, &t.tex);
        if (t.rbo)
            gl_.DeleteRenderbuffers(1, &t.rbo);
        t = Tex();
    }

    // Uploads rows (top row first in `data`) into a rectangle given top-left: textures whose rows are in GL's order
    // get it flipped. BGRA is swizzled and odd row pitches repacked on the CPU.
    void GlDevice::Upload(const Tex& t, int x, int y, int w, int h, const void* data, int rowPitch)
    {
        const FormatInfo fi = Info(t.desc.format);
        const int bpp = BytesPerPixel(t.desc.format);
        const std::size_t row = (std::size_t)w * (std::size_t)bpp;
        const std::size_t pitch = rowPitch > 0 ? (std::size_t)rowPitch : row;
        const auto* src = static_cast<const std::uint8_t*>(data);
        std::vector<std::uint8_t> packed;
        if (t.bottomUp || IsBgra(t.desc.format) || pitch != row)
        {
            packed.resize(row * (std::size_t)h);
            for (int r = 0; r < h; ++r)
            {
                std::uint8_t* d = packed.data() + row * (std::size_t)(t.bottomUp ? h - 1 - r : r);
                std::memcpy(d, src + pitch * (std::size_t)r, row);
                if (IsBgra(t.desc.format))
                    for (std::size_t i = 0; i < row; i += 4)
                        std::swap(d[i], d[i + 2]);
            }
            src = packed.data();
        }
        const int glY = t.bottomUp ? t.desc.height - (y + h) : y;
        BindUnit(kScratchUnit, t.tex);
        gl_.TexSubImage2D(GL_TEXTURE_2D, 0, x, glY, w, h, fi.format, fi.type, src);
    }

    Texture GlDevice::CreateTexture(const TextureDesc& desc, const void* data, int rowPitch)
    {
        const FormatInfo fi = Info(desc.format);
        const bool rt = (desc.usage & TextureUsage_RenderTarget) != 0;
        if (fi.internal == 0 || desc.width <= 0 || desc.height <= 0 || desc.width > caps_.maxTextureSize || desc.height > caps_.maxTextureSize)
            return {};
        // multisampled textures are renderbuffers: render targets that are copied or read back, never sampled
        if (desc.samples > 1 && (!rt || (desc.usage & TextureUsage_Sampled) || desc.samples > maxSamples_ || data))
            return {};
        if (rt && desc.format == Format::RGBA16_FLOAT && !caps_.floatRenderTargets)
            return {};
        OutsideFrame guard(*this);
        Tex t;
        t.desc = desc;
        t.desc.samples = std::max(desc.samples, 1);
        t.desc.debugName = nullptr;   // not owned
        t.bottomUp = rt;
        if (t.desc.samples > 1)
        {
            gl_.GenRenderbuffers(1, &t.rbo);
            gl_.BindRenderbuffer(GL_RENDERBUFFER, t.rbo);
            gl_.RenderbufferStorageMultisample(GL_RENDERBUFFER, t.desc.samples, fi.internal, desc.width, desc.height);
            gl_.BindRenderbuffer(GL_RENDERBUFFER, 0);
        }
        else
        {
            gl_.GenTextures(1, &t.tex);
            BindUnit(kScratchUnit, t.tex);
            // one level, complete without mipmaps even for a sampler-less use; RGBA32F is not filterable on GLES
            const GLint filter = desc.format == Format::RGBA32_FLOAT ? (GLint)GL_NEAREST : (GLint)GL_LINEAR;
            gl_.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
            gl_.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
            gl_.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, (GLint)GL_CLAMP_TO_EDGE);
            gl_.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, (GLint)GL_CLAMP_TO_EDGE);
            gl_.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
            gl_.TexImage2D(GL_TEXTURE_2D, 0, (GLint)fi.internal, desc.width, desc.height, 0, fi.format, fi.type, nullptr);
            if (data)
                Upload(t, 0, 0, desc.width, desc.height, data, rowPitch);
            Label(GL_TEXTURE, t.tex, desc.debugName);
        }
        if (rt && !AttachAndCheck(t))
        {
            ReleaseTex(t);
            CheckErrors("CreateTexture");
            return {};
        }
        Label(GL_FRAMEBUFFER, t.fbo, desc.debugName);
        CheckErrors("CreateTexture");
        const Texture h{next_++};
        textures_[h.id] = t;
        return h;
    }

    void GlDevice::UpdateTexture(Texture tex, const IRect& r, const void* data, int rowPitch)
    {
        Tex* t = FindTex(tex);
        if (!t || !t->tex || t->wrapped || !data || r.Empty() || r.x0 < 0 || r.y0 < 0 || r.x1 > t->desc.width || r.y1 > t->desc.height)
            return;
        OutsideFrame guard(*this);
        Upload(*t, r.x0, r.y0, r.Width(), r.Height(), data, rowPitch);
        CheckErrors("UpdateTexture");
    }

    void GlDevice::DestroyTexture(Texture tex)
    {
        auto it = textures_.find(tex.id);
        if (it == textures_.end())
            return;
        OutsideFrame guard(*this);
        ReleaseTex(it->second);
        textures_.erase(it);
    }

    TextureDesc GlDevice::GetTextureDesc(Texture tex) const
    {
        const Tex* t = FindTex(tex);
        return t ? t->desc : TextureDesc{};
    }

    Texture GlDevice::Wrap(GLuint fbo, int width, int height, Format format, GLuint colorTexture, int samples)
    {
        if (Info(format).internal == 0 || width <= 0 || height <= 0)
            return {};
        Tex t;
        t.wrapped = true;
        t.bottomUp = true;
        t.fbo = fbo;
        t.tex = samples > 1 ? 0 : colorTexture;
        t.desc.width = width;
        t.desc.height = height;
        t.desc.format = format;
        t.desc.samples = std::max(samples, 1);
        t.desc.usage = TextureUsage_RenderTarget | TextureUsage_CopySrc;
        if (t.tex && SamplesRaw(format))
            t.desc.usage |= TextureUsage_Sampled;
        for (auto& [id, w] : textures_)
            if (w.wrapped && w.fbo == fbo && w.tex == t.tex)
            {
                w = t;
                return Texture{id};
            }
        const Texture h{next_++};
        textures_[h.id] = t;
        return h;
    }

    GLuint GlDevice::NativeTexture(Texture t) const
    {
        const Tex* x = FindTex(t);
        return x ? x->tex : 0;
    }

    // ------------------------------------------------------------------ buffers
    Buffer GlDevice::CreateBuffer(const BufferDesc& desc)
    {
        // FX instances live in the RGBA32F texture (Caps::fxStorage = Texture)
        if (desc.size == 0 || desc.kind == BufferKind::FxInstances)
            return {};
        OutsideFrame guard(*this);
        Buf b;
        b.desc = desc;
        b.desc.debugName = nullptr;
        gl_.GenBuffers(1, &b.id);
        // GL_COPY_WRITE_BUFFER: a binding point nothing else uses (the VAO's element buffer stays as it is)
        gl_.BindBuffer(GL_COPY_WRITE_BUFFER, b.id);
        gl_.BufferData(GL_COPY_WRITE_BUFFER, (GLsizeiptr)desc.size, nullptr, GL_DYNAMIC_DRAW);
        Label(GL_BUFFER, b.id, desc.debugName);
        CheckErrors("CreateBuffer");
        const Buffer h{next_++};
        buffers_[h.id] = b;
        return h;
    }

    void GlDevice::UpdateBuffer(Buffer buf, const void* data, std::size_t size)
    {
        auto it = buffers_.find(buf.id);
        if (it == buffers_.end() || !data || size == 0 || size > it->second.desc.size)
            return;
        OutsideFrame guard(*this);
        // ordered after the draws of earlier frames that read the old contents (the driver versions it)
        gl_.BindBuffer(GL_COPY_WRITE_BUFFER, it->second.id);
        gl_.BufferSubData(GL_COPY_WRITE_BUFFER, 0, (GLsizeiptr)size, data);
        CheckErrors("UpdateBuffer");
    }

    void GlDevice::DestroyBuffer(Buffer buf)
    {
        auto it = buffers_.find(buf.id);
        if (it == buffers_.end())
            return;
        OutsideFrame guard(*this);
        if (vaoVb_ == it->second.id)
            vaoVb_ = kUnknown;
        if (vaoIb_ == it->second.id)
            vaoIb_ = kUnknown;
        gl_.DeleteBuffers(1, &it->second.id);
        buffers_.erase(it);
    }

    // ------------------------------------------------------------------ pipelines
    GlDevice::Program* GlDevice::GetProgram(ShaderProgram p)
    {
        Program& prog = programs_[(int)p];
        if (prog.tried)
            return prog.id ? &prog : nullptr;
        prog.tried = true;
        const auto format = (shaders::Format)caps_.shaderFormat;
        const shaders::ShaderBlob* blobs[2] = {shaders::Find(format, p, shaders::Stage::Vertex), shaders::Find(format, p, shaders::Stage::Pixel)};
        if (!blobs[0] || !blobs[1])
            return nullptr;
        GLuint stages[2] = {};
        bool ok = true;
        const GLuint id = gl_.CreateProgram();
        for (int s = 0; s < 2; ++s)
        {
            stages[s] = gl_.CreateShader(s == 0 ? GL_VERTEX_SHADER : GL_FRAGMENT_SHADER);
            const GLchar* src = reinterpret_cast<const GLchar*>(blobs[s]->data);
            const GLint len = (GLint)blobs[s]->size;
            gl_.ShaderSource(stages[s], 1, &src, &len);
            gl_.CompileShader(stages[s]);
            GLint status = 0;
            gl_.GetShaderiv(stages[s], GL_COMPILE_STATUS, &status);
            if (!status)
            {
                char log[4096] = {};
                gl_.GetShaderInfoLog(stages[s], sizeof(log), nullptr, log);
                std::fprintf(stderr, "esia %s: %s (%s) does not compile: %s\n", Name(), ShaderProgramName(p), blobs[s]->entry, log);
                ok = false;
            }
            gl_.AttachShader(id, stages[s]);
        }
        if (ok)
        {
            gl_.LinkProgram(id);
            GLint status = 0;
            gl_.GetProgramiv(id, GL_LINK_STATUS, &status);
            if (!status)
            {
                char log[4096] = {};
                gl_.GetProgramInfoLog(id, sizeof(log), nullptr, log);
                std::fprintf(stderr, "esia %s: %s does not link: %s\n", Name(), ShaderProgramName(p), log);
                ok = false;
            }
        }
        for (GLuint s : stages)
        {
            gl_.DetachShader(id, s);
            gl_.DeleteShader(s);
        }
        if (!ok)
        {
            gl_.DeleteProgram(id);
            CheckErrors("CreatePipeline");
            return nullptr;
        }
        const char* blocks[3] = {"WgtFrame", "WgtPass", "WgtDraw"};
        for (GLuint b = 0; b < 3; ++b)
        {
            const GLuint index = gl_.GetUniformBlockIndex(id, blocks[b]);
            if (index != GL_INVALID_INDEX)
                gl_.UniformBlockBinding(id, index, b);
        }
        // combined samplers: unit = RHI slot; both stages list theirs (the FX data texture appears in both)
        UseProgram(id);
        for (const shaders::ShaderBlob* blob : blobs)
            for (int i = 0; i < blob->textureCount; ++i)
            {
                const shaders::TextureBinding& b = blob->textures[i];
                const GLint loc = gl_.GetUniformLocation(id, b.name);
                if (loc >= 0)
                    gl_.Uniform1i(loc, b.slot);
                prog.sampler[b.slot] = b.sampler;
            }
        Label(GL_PROGRAM, id, ShaderProgramName(p));
        prog.id = id;
        CheckErrors("CreatePipeline");
        return &prog;
    }

    Pipeline GlDevice::CreatePipeline(const PipelineDesc& desc)
    {
        if (desc.program >= ShaderProgram::Count || desc.effect != 0 || desc.fxFeatures != 0)
            return {};   // no runtime effects, no FX variants (Caps)
        Program* prog = nullptr;
        {
            OutsideFrame guard(*this);
            prog = GetProgram(desc.program);
        }
        if (pipe_)
            UseProgram(pipe_->program->id);   // compiled inside a pass: the bound pipeline's program stays current
        if (!prog)
            return {};
        const Pipeline h{next_++};
        pipelines_[h.id] = Pipe{desc, prog};
        return h;
    }

    void GlDevice::DestroyPipeline(Pipeline p)
    {
        pipelines_.erase(p.id);   // programs are shared by every pipeline of their ShaderProgram: they stay
    }

    // ------------------------------------------------------------------ frames and passes
    bool GlDevice::BeginFrame(const FrameDesc&)
    {
        if (owned_)
            owned_->MakeCurrent();
        ForgetGlState();
        if (desc_.restoreHostState)
            SaveHostState(frameState_);
        inFrame_ = true;
        hostTouched_ = false;
        ApplyFixedState();
        ++frame_;
        TimerBeginFrame();
        return true;
    }

    void GlDevice::EndFrame()
    {
        if (!inFrame_)
            return;
        TimerEndFrame();
        // a host's SwapBuffers submits and paces its frames; a headless context has none, and NVIDIA's driver completes
        // the frame's timestamp queries only after a finish (a flush is not enough)
        if (owned_)
            gl_.Finish();
        CheckErrors("frame");
        if (desc_.restoreHostState)
            RestoreHostState(frameState_);
        inFrame_ = inPass_ = false;
        pass_ = nullptr;
        pipe_ = nullptr;
        ForgetGlState();
    }

    void GlDevice::BeginPass(const PassDesc& d)
    {
        Tex* t = FindTex(d.target);
        if (!t || !inFrame_)
            return;
        pass_ = t;
        pipe_ = nullptr;
        vb_ = ib_ = 0;
        inPass_ = true;
        // a texture is never sampled while it is rendered: drop stale unit bindings of it (GL feedback loops)
        if (t->tex)
            for (int u = 0; u < kUnits; ++u)
                if (unitTex_[u] == t->tex)
                    BindUnit(u, 0);
        RestorePassState();
        if (d.load == LoadOp::Clear)
        {
            SetScissorTest(false);
            gl_.ClearColor(d.clearColor[0], d.clearColor[1], d.clearColor[2], d.clearColor[3]);
            gl_.Clear(GL_COLOR_BUFFER_BIT);
            SetScissorTest(true);
        }
        else if (d.load == LoadOp::DontCare && gl_.InvalidateFramebuffer)
        {
            // tilers need not load what the pass never reads (pyramid levels, fresh glow layers)
            const GLenum attachment = t->fbo ? GL_COLOR_ATTACHMENT0 : GL_COLOR;
            gl_.InvalidateFramebuffer(GL_FRAMEBUFFER, 1, &attachment);
        }
    }

    // After a host callback: the fixed state again, before anything that draws or copies (the pass may have ended
    // right after the callback).
    void GlDevice::EnsureFixedState()
    {
        if (hostTouched_)
        {
            ApplyFixedState();
            hostTouched_ = false;
        }
    }

    // The pass's framebuffer, full viewport and scissor, sRGB encoding for sRGB targets - at BeginPass, and again
    // after a host callback changed whatever it liked.
    void GlDevice::RestorePassState()
    {
        EnsureFixedState();
        BindFramebuffer(GL_FRAMEBUFFER, pass_->fbo);
        gl_.Viewport(0, 0, pass_->desc.width, pass_->desc.height);
        SetScissorTest(true);
        gl_.Scissor(0, 0, pass_->desc.width, pass_->desc.height);
        SetSrgbWrite(IsSrgb(pass_->desc.format));
    }

    void GlDevice::EndPass()
    {
        inPass_ = false;
        pass_ = nullptr;
        pipe_ = nullptr;
    }

    void GlDevice::SetPipeline(Pipeline p)
    {
        auto it = pipelines_.find(p.id);
        if (!inPass_ || it == pipelines_.end())
            return;
        if (hostTouched_)
            RestorePassState();
        pipe_ = &it->second;
        UseProgram(pipe_->program->id);
        SetBlend(pipe_->desc.blend);
        for (int s = 0; s < kTextureSlots; ++s)
            if (pipe_->program->sampler[s] >= 0)
                BindSampler(s, samplers_[pipe_->program->sampler[s]]);
    }

    void GlDevice::SetScissor(const IRect& r)
    {
        if (!inPass_)
            return;
        if (hostTouched_)
            RestorePassState();
        gl_.Scissor(r.x0, pass_->desc.height - r.y1, std::max(r.Width(), 0), std::max(r.Height(), 0));
    }

    // Every call writes the next aligned range of one uniform buffer and binds it: draws already recorded keep their
    // range. When the ring is full the buffer is orphaned (the driver hands out fresh storage and keeps the old one
    // for the queued draws) and the current values of all three slots are written again, because the bindings made
    // before now point into the new storage.
    void GlDevice::SetConstants(ConstantSlot slot, const void* data, std::uint32_t size)
    {
        const int s = (int)slot;
        if (!inFrame_ || !data || size == 0 || size > sizeof(lastConstants_[s]))
            return;
        if (hostTouched_ && inPass_)
            RestorePassState();
        std::memcpy(lastConstants_[s], data, size);
        lastConstantSize_[s] = size;
        auto write = [&](int k) {
            const GLintptr aligned = (GLintptr)((lastConstantSize_[k] + (std::uint32_t)uboAlign_ - 1) / (std::uint32_t)uboAlign_ * (std::uint32_t)uboAlign_);
            gl_.BufferSubData(GL_UNIFORM_BUFFER, uboOffset_, (GLsizeiptr)lastConstantSize_[k], lastConstants_[k]);
            gl_.BindBufferRange(GL_UNIFORM_BUFFER, (GLuint)k, ubo_, uboOffset_, (GLsizeiptr)lastConstantSize_[k]);
            uboOffset_ += aligned;
        };
        gl_.BindBuffer(GL_UNIFORM_BUFFER, ubo_);
        if (uboOffset_ + 3 * (GLintptr)(256 + uboAlign_) > uboSize_)
        {
            gl_.BufferData(GL_UNIFORM_BUFFER, uboSize_, nullptr, GL_STREAM_DRAW);
            uboOffset_ = 0;
            for (int k = 0; k < 3; ++k)
                if (k != s && lastConstantSize_[k])
                    write(k);
        }
        write(s);
    }

    void GlDevice::SetTexture(int slot, Texture tex)
    {
        if (!inFrame_ || slot < 0 || slot >= kTextureSlots)
            return;
        if (hostTouched_ && inPass_)
            RestorePassState();
        const Tex* t = FindTex(tex);
        BindUnit(slot, t ? t->tex : 0);
    }

    void GlDevice::SetFxBuffer(Buffer) {}   // FxStorage::Texture: the renderer binds the instance texture at t7

    void GlDevice::SetVertexBuffer(Buffer buf)
    {
        auto it = buffers_.find(buf.id);
        vb_ = it != buffers_.end() ? it->second.id : 0;
    }

    void GlDevice::SetIndexBuffer(Buffer buf)
    {
        auto it = buffers_.find(buf.id);
        ib_ = it != buffers_.end() ? it->second.id : 0;
    }

    void GlDevice::PrepareDraw()
    {
        if (hostTouched_)
            RestorePassState();
        if (pipe_->desc.layout != VertexLayout::UiVertex)
        {
            BindVertexArray(emptyVao_);
            return;
        }
        BindVertexArray(uiVao_);
        if (vaoVb_ != vb_)
        {
            // attribute pointers capture the buffer bound to GL_ARRAY_BUFFER: esia::Vertex, 20 bytes
            gl_.BindBuffer(GL_ARRAY_BUFFER, vb_);
            gl_.VertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 20, reinterpret_cast<const void*>(0));
            gl_.VertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 20, reinterpret_cast<const void*>(8));
            gl_.VertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, 20, reinterpret_cast<const void*>(16));
            vaoVb_ = vb_;
        }
        if (vaoIb_ != ib_)
        {
            gl_.BindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib_);   // vertex array state
            vaoIb_ = ib_;
        }
    }

    void GlDevice::Draw(std::uint32_t vertexCount, std::uint32_t firstVertex)
    {
        if (!inPass_ || !pipe_)
            return;
        PrepareDraw();
        gl_.DrawArrays(pipe_->desc.topology == Topology::TriangleStrip ? GL_TRIANGLE_STRIP : GL_TRIANGLES, (GLint)firstVertex, (GLsizei)vertexCount);
    }

    void GlDevice::DrawIndexed(std::uint32_t indexCount, std::uint32_t firstIndex)
    {
        if (!inPass_ || !pipe_ || !vb_ || !ib_)
            return;
        PrepareDraw();
        gl_.DrawElements(GL_TRIANGLES, (GLsizei)indexCount, GL_UNSIGNED_INT, reinterpret_cast<const void*>((std::uintptr_t)firstIndex * 4u));
    }

    void GlDevice::DrawInstanced(std::uint32_t vertexCount, std::uint32_t instanceCount)
    {
        if (!inPass_ || !pipe_)
            return;
        PrepareDraw();
        gl_.DrawArraysInstanced(GL_TRIANGLE_STRIP, 0, (GLsizei)vertexCount, (GLsizei)instanceCount);
    }

    // ------------------------------------------------------------------ copies
    // A single-sample texture of the source's format and size that a multisampled source is resolved into: GLES only
    // resolves between identical formats, and glReadPixels cannot read a multisampled framebuffer.
    const GlDevice::Tex* GlDevice::ResolveTarget(const Tex& src)
    {
        if (resolve_.tex && resolve_.desc.format == src.desc.format && resolve_.desc.width == src.desc.width &&
            resolve_.desc.height == src.desc.height)
            return &resolve_;
        ReleaseTex(resolve_);
        const FormatInfo fi = Info(src.desc.format);
        resolve_.desc = src.desc;
        resolve_.desc.samples = 1;
        resolve_.desc.usage = TextureUsage_RenderTarget | TextureUsage_CopySrc;
        resolve_.bottomUp = true;
        gl_.GenTextures(1, &resolve_.tex);
        BindUnit(kScratchUnit, resolve_.tex);
        gl_.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
        gl_.TexImage2D(GL_TEXTURE_2D, 0, (GLint)fi.internal, src.desc.width, src.desc.height, 0, fi.format, fi.type, nullptr);
        if (!AttachAndCheck(resolve_))
        {
            ReleaseTex(resolve_);
            return nullptr;
        }
        Label(GL_TEXTURE, resolve_.tex, "esia-resolve");
        return &resolve_;
    }

    // Rectangles converted to GL's bottom-left rows; the scissor test would clip the blit. Only between formats of
    // the same encoding: a blit linearizes an sRGB source (always on GLES, before GL 4.4 on desktop GL too) and
    // encodes into an sRGB destination with GL_FRAMEBUFFER_SRGB on, so sRGB -> sRGB is a round trip (and a resolve
    // averages in linear light), UNORM -> UNORM a plain copy; sRGB -> raw copies go through DrawCopy.
    void GlDevice::Blit(const Tex& src, const Tex& dst, const IRect& r, int dstX, int dstY)
    {
        SetScissorTest(false);
        SetSrgbWrite(IsSrgb(src.desc.format) && IsSrgb(dst.desc.format));
        BindFramebuffer(GL_READ_FRAMEBUFFER, src.fbo);
        BindFramebuffer(GL_DRAW_FRAMEBUFFER, dst.fbo);
        const int sy0 = src.bottomUp ? src.desc.height - r.y1 : r.y0;
        const int dy0 = dst.bottomUp ? dst.desc.height - (dstY + r.Height()) : dstY;
        gl_.BlitFramebuffer(r.x0, sy0, r.x1, sy0 + r.Height(), dstX, dy0, dstX + r.Width(), dy0 + r.Height(), GL_COLOR_BUFFER_BIT, GL_NEAREST);
    }

    void GlDevice::CopyTexture(Texture dst, int dstX, int dstY, Texture src, const IRect& r)
    {
        Tex* d = FindTex(dst);
        Tex* s = FindTex(src);
        if (!d || !s || d->desc.samples > 1 || r.Empty() || inPass_)
            return;
        OutsideFrame guard(*this);
        EnsureFixedState();
        if (EnsureFbo(*d) == kUnknown || EnsureFbo(*s) == kUnknown)
            return;
        d->bottomUp = s->bottomUp;   // the copy keeps the source's row order
        // the backdrop copy of an sRGB target (RawFormat): a draw that reads the stored bits, from a texture
        const bool raw = IsSrgb(s->desc.format) && !IsSrgb(d->desc.format);
        // GLES resolves only into the same rectangle, and between identical formats
        const bool sameRect = r.x0 == dstX && (s->bottomUp ? s->desc.height - r.y1 : r.y0) == (d->bottomUp ? d->desc.height - (dstY + r.Height()) : dstY);
        const bool resolveFirst = s->desc.samples > 1 && (!sameRect || Info(s->desc.format).internal != Info(d->desc.format).internal);
        const Tex* from = s;
        if ((raw && !s->tex) || resolveFirst)
        {
            // resolve (or copy a window / renderbuffer target) into a texture of the same format first
            from = ResolveTarget(*s);
            if (!from)
                return;
            Blit(*s, *from, r, r.x0, r.y0);
        }
        if (raw)
            DrawCopy(*from, *d, r, dstX, dstY);
        else
            Blit(*from, *d, r, dstX, dstY);
        CheckErrors("CopyTexture");
    }

    // Copies the stored bits of an sRGB texture into a non-sRGB target by drawing: the point sampler skips the
    // decode (EXT_texture_sRGB_decode), or, without that extension, the shader encodes what the sampler decoded
    // (exact for 8-bit values). The backend's own program: a full-viewport triangle, scissored to the rectangle.
    bool GlDevice::DrawCopy(const Tex& src, const Tex& dst, const IRect& r, int dstX, int dstY)
    {
        if (!copyTried_)
        {
            copyTried_ = true;
            const char* header = desc_.es ? "#version 300 es\nprecision highp float;\n" : "#version 330\n";
            const char* vs = "void main()\n"
                             "{\n"
                             "    gl_Position = vec4((gl_VertexID & 1) != 0 ? 3.0 : -1.0, (gl_VertexID & 2) != 0 ? 3.0 : -1.0, 0.0, 1.0);\n"
                             "}\n";
            const char* fs = "uniform highp sampler2D uSource;\n"
                             "uniform vec4 uMap;      // xy: source minus destination pixel, zw: 1 / source size\n"
                             "uniform float uEncode;  // 1: the sampler decoded sRGB, encode it again\n"
                             "out vec4 oColor;\n"
                             "void main()\n"
                             "{\n"
                             "    vec4 c = texture(uSource, (gl_FragCoord.xy + uMap.xy) * uMap.zw);\n"
                             "    if (uEncode > 0.5)\n"
                             "    {\n"
                             "        vec3 x = clamp(c.rgb, 0.0, 1.0);\n"
                             "        c.rgb = mix(x * 12.92, 1.055 * pow(x, vec3(1.0 / 2.4)) - 0.055, step(vec3(0.0031308), x));\n"
                             "    }\n"
                             "    oColor = c;\n"
                             "}\n";
            const GLuint program = gl_.CreateProgram();
            GLint ok = 1;
            for (int s = 0; s < 2; ++s)
            {
                const GLuint shader = gl_.CreateShader(s == 0 ? GL_VERTEX_SHADER : GL_FRAGMENT_SHADER);
                const GLchar* parts[2] = {header, s == 0 ? vs : fs};
                gl_.ShaderSource(shader, 2, parts, nullptr);
                gl_.CompileShader(shader);
                GLint compiled = 0;
                gl_.GetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
                ok &= compiled;
                gl_.AttachShader(program, shader);
                gl_.DeleteShader(shader);   // freed with the program
            }
            if (ok)
            {
                gl_.LinkProgram(program);
                gl_.GetProgramiv(program, GL_LINK_STATUS, &ok);
            }
            if (!ok)
            {
                std::fprintf(stderr, "esia %s: the sRGB copy program does not build\n", Name());
                gl_.DeleteProgram(program);
                return false;
            }
            copyProgram_ = program;
            copyMap_ = gl_.GetUniformLocation(program, "uMap");
            copyEncode_ = gl_.GetUniformLocation(program, "uEncode");
            UseProgram(program);
            gl_.Uniform1i(gl_.GetUniformLocation(program, "uSource"), kScratchUnit);
            Label(GL_PROGRAM, program, "esia-srgb-copy");
        }
        if (!copyProgram_)
            return false;
        const int sy0 = src.bottomUp ? src.desc.height - r.y1 : r.y0;
        const int dy0 = dst.bottomUp ? dst.desc.height - (dstY + r.Height()) : dstY;
        BindFramebuffer(GL_DRAW_FRAMEBUFFER, dst.fbo);
        SetSrgbWrite(false);
        SetBlend(BlendMode::Opaque);
        gl_.Viewport(0, 0, dst.desc.width, dst.desc.height);
        SetScissorTest(true);
        gl_.Scissor(dstX, dy0, r.Width(), r.Height());
        UseProgram(copyProgram_);
        gl_.Uniform4f(copyMap_, (float)(r.x0 - dstX), (float)(sy0 - dy0), 1.0f / (float)src.desc.width, 1.0f / (float)src.desc.height);
        gl_.Uniform1f(copyEncode_, srgbDecodeControl_ ? 0.0f : 1.0f);
        BindUnit(kScratchUnit, src.tex);
        BindSampler(kScratchUnit, samplers_[1]);
        BindVertexArray(emptyVao_);
        gl_.DrawArrays(GL_TRIANGLES, 0, 3);
        return true;
    }

    void* GlDevice::NativeRenderState()
    {
        // host code draws with the current context and may change anything: forget every cached binding
        ForgetGlState();
        pipe_ = nullptr;
        vb_ = ib_ = 0;
        hostTouched_ = true;
        return nullptr;
    }

    // ------------------------------------------------------------------ readback
    bool GlDevice::ReadPixels(Texture tex, const IRect& r, std::vector<std::uint8_t>& out)
    {
        out.clear();
        Tex* t = FindTex(tex);
        if (!t || r.Empty() || r.x0 < 0 || r.y0 < 0 || r.x1 > t->desc.width || r.y1 > t->desc.height || inPass_)
            return false;
        if (owned_ && !inFrame_)
            owned_->MakeCurrent();
        OutsideFrame guard(*this);
        EnsureFixedState();
        const Tex* from = t;
        if (EnsureFbo(*t) == kUnknown)
            return false;
        if (t->desc.samples > 1)
        {
            from = ResolveTarget(*t);
            if (!from)
                return false;
            Blit(*t, *from, r, r.x0, r.y0);
        }
        // the stored values of an sRGB framebuffer: GLES never converts, desktop GL not with GL_FRAMEBUFFER_SRGB off
        SetSrgbWrite(false);
        BindFramebuffer(GL_READ_FRAMEBUFFER, from->fbo);
        const int w = r.Width(), h = r.Height();
        const int glY = from->bottomUp ? from->desc.height - r.y1 : r.y0;
        const std::size_t n = (std::size_t)w * (std::size_t)h;
        out.resize(n * 4);
        // what GLES allows for each format class: RGBA / UNSIGNED_BYTE for 8-bit normalized, the packed type for
        // RGB10_A2, RGBA / FLOAT for float formats
        switch (from->desc.format)
        {
        case Format::RGBA16_FLOAT:
        case Format::RGBA32_FLOAT:
        {
            std::vector<float> f(n * 4);
            gl_.ReadPixels(r.x0, glY, w, h, GL_RGBA, GL_FLOAT, f.data());
            for (std::size_t i = 0; i < n * 4; ++i)
                out[i] = Unorm8(f[i]);
            break;
        }
        case Format::RGB10A2_UNORM:
        {
            std::vector<std::uint32_t> p(n);
            gl_.ReadPixels(r.x0, glY, w, h, GL_RGBA, GL_UNSIGNED_INT_2_10_10_10_REV, p.data());
            for (std::size_t i = 0; i < n; ++i)
            {
                for (int c = 0; c < 3; ++c)
                    out[i * 4 + (std::size_t)c] = Unorm8((float)((p[i] >> (10 * c)) & 1023u) / 1023.0f);
                out[i * 4 + 3] = Unorm8((float)(p[i] >> 30) / 3.0f);
            }
            break;
        }
        default: gl_.ReadPixels(r.x0, glY, w, h, GL_RGBA, GL_UNSIGNED_BYTE, out.data()); break;
        }
        if (from->bottomUp)
        {
            const std::size_t row = (std::size_t)w * 4;
            std::vector<std::uint8_t> tmp(row);
            for (int y = 0; y < h / 2; ++y)
            {
                std::uint8_t* a = out.data() + row * (std::size_t)y;
                std::uint8_t* b = out.data() + row * (std::size_t)(h - 1 - y);
                std::memcpy(tmp.data(), a, row);
                std::memcpy(a, b, row);
                std::memcpy(b, tmp.data(), row);
            }
        }
        CheckErrors("ReadPixels");
        return true;
    }

    // ------------------------------------------------------------------ timing
    int GlDevice::Stamp()
    {
        if (!timerCur_ || timerCur_->used >= kMaxStamps)
            return -1;
        gl_.QueryCounter(timerCur_->queries[(std::size_t)timerCur_->used], GL_TIMESTAMP);
        return timerCur_->used++;
    }

    void GlDevice::TimerBeginFrame()
    {
        if (!caps_.timestampQueries)
            return;
        timerCur_ = &timer_[frame_ % kTimerSlots];
        if (timerCur_->pending)
            TimerRead(*timerCur_);   // late: read it now or drop it (the slot is needed again)
        timerCur_->pending = false;
        timerCur_->used = 0;
        timerCur_->frameEnd = -1;
        timerCur_->intervals.clear();
        timerCur_->frame = frame_;
        profileStart_ = -1;
        Stamp();
    }

    void GlDevice::TimerEndFrame()
    {
        if (!timerCur_)
            return;
        if (profileStart_ >= 0)
            EndProfile();
        timerCur_->frameEnd = Stamp();
        timerCur_->pending = timerCur_->frameEnd > 0;
        timerCur_ = nullptr;
    }

    void GlDevice::BeginProfile(ProfileCategory category)
    {
        if (!timerCur_)
            return;
        profileStart_ = Stamp();
        profileCategory_ = category;
    }

    void GlDevice::EndProfile()
    {
        if (!timerCur_ || profileStart_ < 0)
            return;
        const int end = Stamp();
        if (end >= 0)
            timerCur_->intervals.push_back({profileStart_, end, profileCategory_});
        profileStart_ = -1;
    }

    // Non-blocking: false while the GPU has not reached the frame's last timestamp. A disjoint period (GLES: the GPU
    // was reset or its clock changed) invalidates everything in flight.
    bool GlDevice::TimerRead(TimerSlot& s)
    {
        GLuint available = 0;
        gl_.GetQueryObjectuiv(s.queries[(std::size_t)s.frameEnd], GL_QUERY_RESULT_AVAILABLE, &available);
        if (!available)
            return false;
        s.pending = false;
        if (disjointQuery_)
        {
            GLint disjoint = 0;
            gl_.GetIntegerv(GL_GPU_DISJOINT_EXT, &disjoint);
            if (disjoint)
            {
                for (TimerSlot& o : timer_)
                    o.pending = false;
                return false;
            }
        }
        std::vector<GLuint64> t((std::size_t)s.used);
        for (int i = 0; i < s.used; ++i)
            gl_.GetQueryObjectui64v(s.queries[(std::size_t)i], GL_QUERY_RESULT, &t[(std::size_t)i]);
        GpuProfile p;
        p.valid = true;
        p.frame = s.frame;
        p.totalMs = (float)((double)(t[(std::size_t)s.frameEnd] - t[0]) * 1e-6);
        for (const TimerSlot::Interval& iv : s.intervals)
            p.categoryMs[(int)iv.category] += (float)((double)(t[(std::size_t)iv.end] - t[(std::size_t)iv.start]) * 1e-6);
        if (!profile_.valid || p.frame > profile_.frame)
            profile_ = p;
        return true;
    }

    bool GlDevice::ReadProfile(GpuProfile& out)
    {
        if (caps_.timestampQueries && !inFrame_)
        {
            if (owned_)
                owned_->MakeCurrent();
            // oldest first, so the newest complete frame wins
            TimerSlot* order[kTimerSlots];
            for (int i = 0; i < kTimerSlots; ++i)
                order[i] = &timer_[i];
            std::sort(std::begin(order), std::end(order), [](const TimerSlot* a, const TimerSlot* b) { return a->frame < b->frame; });
            for (TimerSlot* s : order)
                if (s->pending)
                    TimerRead(*s);
        }
        out = profile_;
        return profile_.valid;
    }

    // ------------------------------------------------------------------ public API
    std::unique_ptr<Device> CreateDevice(const Desc& desc, std::string* error)
    {
        auto dev = std::make_unique<GlDevice>(desc, nullptr);
        std::string e;
        if (!dev->Init(e))
        {
            if (error)
                *error = e;
            return nullptr;
        }
        return dev;
    }

    Texture WrapFramebuffer(Device& device, unsigned int fbo, int width, int height, Format format, unsigned int colorTexture, int samples)
    {
        return static_cast<GlDevice&>(device).Wrap(fbo, width, height, format, colorTexture, samples);
    }

    unsigned int NativeTexture(Device& device, Texture texture) { return static_cast<GlDevice&>(device).NativeTexture(texture); }

    std::uint32_t ErrorCount(const Device& device) { return static_cast<const GlDevice&>(device).Errors(); }
}
