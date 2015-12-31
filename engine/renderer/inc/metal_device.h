#pragma once

#include <string>
#include <vector>

namespace JMD {
namespace GFX {
void LoadDevice();
struct PixelFormatImpl;
struct PixelFormat {
    PixelFormat():impl(nullptr){}
    bool Load(const std::string &pixel_format);
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
    void Load(const std::string& name);
    LibraryImpl* impl;
private:
    void Create();
};
struct EffectImpl;
struct Effect {
    Effect():impl(nullptr){}
    void Release();
    void Load(Library& library, const std::string& vert_name, const std::string& frag_name);
    EffectImpl* impl;
private:
    void Create();
};
struct PipelineDescImpl;
struct PipelineDesc {
    PipelineDesc():impl(nullptr){}
    void Release();
    void Load(Effect& effect,
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
    PipelineStateImpl* impl;
private:
    void Create();
};
}}