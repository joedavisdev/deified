#pragma once

#include <string>
#include <vector>

namespace JMD {
namespace GFX {
void LoadDevice();
    
#define PIMPL_DECL(_name) \
    _name();~_name(); \
    struct Impl;std::shared_ptr<Impl> impl; \
    
struct Buffer {
    PIMPL_DECL(Buffer)
    bool Initialise(const char* const data, const unsigned int length);
    void Update(const char* const data, const unsigned int length);
};
struct PixelFormat {
    PIMPL_DECL(PixelFormat)
    bool Initialize(const std::string &pixel_format);
};
class CommandBuffer {
};
struct Library {
    PIMPL_DECL(Library)
    void Initialize(const std::string& name);
};
struct Effect {
    PIMPL_DECL(Effect)
    void Initialize(Library& library, const std::string& vert_name, const std::string& frag_name);
};
struct PipelineDesc {
    PIMPL_DECL(PipelineDesc)
    void Initialize(
        Effect& effect,
        const unsigned int sample_count,
        const std::vector<PixelFormat>& colour_formats,
        const PixelFormat& depth_format,
        const PixelFormat& stencil_format);
};
struct PipelineState {
    PIMPL_DECL(PipelineState)
    void Initialize(const PipelineDesc& pipeline_descriptor);
};
}}