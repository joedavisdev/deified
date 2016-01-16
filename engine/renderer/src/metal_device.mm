#include "gfx_device.h"

#import <Metal/Metal.h>

#include <unordered_map>

namespace JMD {
namespace GFX {
static id <MTLDevice> mtl_device;
void LoadDevice() {
    mtl_device = MTLCreateSystemDefaultDevice();
}
#pragma mark Impls
#define PIMPL_DEF(_name,_setup,_shutdown) \
    _name::_name():impl(nullptr){} \
    _name::~_name(){} \
    void _name::Create(){if (impl == nullptr){impl = new Impl();_setup;}} \
    void _name::Release(){_shutdown;delete impl;} \
    
struct Buffer::Impl {
    id<MTLBuffer> buffer;
    unsigned int length;
};
PIMPL_DEF(Buffer,NULL,NULL)
    
struct PixelFormat::Impl {
    Impl():format(MTLPixelFormatInvalid){}
    MTLPixelFormat format;
};
PIMPL_DEF(PixelFormat,NULL,NULL)

struct Library::Impl {
    id<MTLLibrary> library;
};
PIMPL_DEF(Library,NULL,NULL)
    
struct Effect::Impl {
    id<MTLFunction> vertex_fn;
    id<MTLFunction> fragment_fn;
};
PIMPL_DEF(Effect,NULL,NULL)
    
struct PipelineDesc::Impl {
    MTLRenderPipelineDescriptor* descriptor;
};
PIMPL_DEF(PipelineDesc,impl->descriptor=[[MTLRenderPipelineDescriptor alloc]init],NULL)
    
struct PipelineState::Impl {
    id<MTLRenderPipelineState> data;
};
PIMPL_DEF(PipelineState,NULL,NULL)
    
#pragma mark Member functions
bool Buffer::Initialise(const char* const data, const unsigned int length) {
    this->Create();
    if(data == nullptr){
        impl->buffer = [mtl_device newBufferWithLength:length options:MTLResourceOptionCPUCacheModeDefault];
    }else{
        impl->buffer = [mtl_device newBufferWithBytes:data length:length options:MTLResourceOptionCPUCacheModeDefault];
    }
    impl->length = length;
    return true;
}
void Buffer::Update(const char* const data, const unsigned int length) {
    assert(length <= impl->length);
    memcpy(impl->buffer.contents, data, length);
}
bool PixelFormat::Initialize(const std::string &pixel_format){
    this->Create();
    MTLPixelFormat& mtl_pixel_format(impl->format);
    // Find the requested format
    std::unordered_map<std::string,MTLPixelFormat> pixel_format_map{
        {"BGRA8U",MTLPixelFormatBGRA8Unorm},
        {"D32F",MTLPixelFormatDepth32Float}
    };
    const auto selected(pixel_format_map.find(pixel_format));
    if(selected != pixel_format_map.end()){
        mtl_pixel_format = selected->second;
    }else{
        return false;
    }
    return true;
}
void Library::Initialize(const std::string &name) {
    this->Create();
    if(name == ""){
        impl->library = [mtl_device newDefaultLibrary];
    }else{
        assert(0);
    }
    if(!impl->library) {
        NSLog(@">> ERROR: Couldnt create a shader library %s",name.c_str());
        assert(0);
    }
    impl->library.label = [[NSString alloc]initWithCString:name.c_str() encoding:NSASCIIStringEncoding];
}
void Effect::Initialize(Library& library, const std::string &vert_name, const std::string &frag_name){
    this->Create();
    id<MTLLibrary> mtl_library(library.impl->library);
    NSString* vert_nsstring = [[NSString alloc]initWithCString:vert_name.c_str() encoding:NSASCIIStringEncoding];
    impl->vertex_fn = [mtl_library newFunctionWithName:vert_nsstring];
    if(!impl->vertex_fn)
        NSLog(@">> ERROR: Couldn't load %@ vertex program from supplied library (%@)", vert_nsstring, mtl_library.label);
    assert(impl->vertex_fn.functionType == MTLFunctionType::MTLFunctionTypeVertex);
    NSString* frag_nsstring = [[NSString alloc]initWithCString:frag_name.c_str() encoding:NSASCIIStringEncoding];
    impl->fragment_fn = [mtl_library newFunctionWithName:frag_nsstring];
    if(!impl->fragment_fn)
        NSLog(@">> ERROR: Couldn't load %@ fragment program from supplied library (%@)", frag_nsstring, mtl_library.label);
    assert(impl->fragment_fn.functionType == MTLFunctionType::MTLFunctionTypeFragment);
}
void PipelineDesc::Initialize(Effect& effect,
                        const unsigned int sample_count,
                        const std::vector<PixelFormat>& colour_formats,
                        const PixelFormat& depth_format,
                        const PixelFormat& stencil_format) {
    this->Create();
    MTLRenderPipelineDescriptor* mtl_pipeline_descriptor(impl->descriptor);
    mtl_pipeline_descriptor.sampleCount = sample_count;
    assert(effect.impl != nullptr);
    mtl_pipeline_descriptor.fragmentFunction = effect.impl->fragment_fn;
    mtl_pipeline_descriptor.vertexFunction = effect.impl->vertex_fn;
    for (unsigned int index=0; index < colour_formats.size(); ++index) {
        const PixelFormat& pixel_format(colour_formats.at(index));
        if(pixel_format.impl != nullptr && pixel_format.impl->format != MTLPixelFormatInvalid)
            mtl_pipeline_descriptor.colorAttachments[index].pixelFormat = pixel_format.impl->format;
    }
    if(depth_format.impl != nullptr && depth_format.impl->format != MTLPixelFormatInvalid)
        mtl_pipeline_descriptor.depthAttachmentPixelFormat = depth_format.impl->format;
    if(stencil_format.impl != nullptr && stencil_format.impl->format != MTLPixelFormatInvalid)
        mtl_pipeline_descriptor.stencilAttachmentPixelFormat = stencil_format.impl->format;
}
void PipelineState::Initialize(const JMD::GFX::PipelineDesc &pipeline_descriptor) {
    this->Create();
    MTLRenderPipelineDescriptor* mtl_pipeline_desc(pipeline_descriptor.impl->descriptor);
    assert(mtl_pipeline_desc != nullptr);
    NSError *error = nil;
    impl->data = [mtl_device newRenderPipelineStateWithDescriptor:mtl_pipeline_desc error:&error];
    if(!impl->data) {
        NSLog(@">> ERROR: Failed Aquiring pipeline state: %@", error);
    }
}
}}