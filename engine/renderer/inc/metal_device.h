#pragma once

#include <string>
#include <vector>

namespace JMD {
namespace GFX {
void LoadDevice();
struct BufferImpl;
struct Buffer {
    Buffer():impl(nullptr){}
    bool Initialise(const char* const data, const unsigned int length);
    void Release();
    BufferImpl* impl;
private:
    void Create();
};
struct PixelFormatImpl;
struct PixelFormat {
    PixelFormat():impl(nullptr){}
    bool Initialize(const std::string &pixel_format);
    void Release();
    PixelFormatImpl* impl;
private:
    void Create();
};
class CommandBuffer {
};
struct LibraryImpl;
struct Library {
    Library():impl(nullptr){}
    void Release();
    void Initialize(const std::string& name);
    LibraryImpl* impl;
private:
    void Create();
};
struct EffectImpl;
struct Effect {
    Effect():impl(nullptr){}
    void Release();
    void Initialize(Library& library, const std::string& vert_name, const std::string& frag_name);
    EffectImpl* impl;
private:
    void Create();
};
struct PipelineDescImpl;
struct PipelineDesc {
    PipelineDesc():impl(nullptr){}
    void Release();
    void Initialize(Effect& effect,
              const unsigned int sample_count,
              const std::vector<PixelFormat>& colour_formats,
              const PixelFormat& depth_format,
              const PixelFormat& stencil_format);
    PipelineDescImpl* impl;
private:
    void Create();
};
struct PipelineStateImpl;
struct PipelineState {
    PipelineState():impl(nullptr){}
    void Release();
    void Initialize(const PipelineDesc& pipeline_descriptor);
    PipelineStateImpl* impl;
private:
    void Create();
};
}}