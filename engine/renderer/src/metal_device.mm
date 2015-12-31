#include "metal_device.h"

#import <Metal/Metal.h>

#include <unordered_map>

namespace JMD {
namespace GFX {
static id <MTLDevice> mtl_device;
void LoadDevice() {
    mtl_device = MTLCreateSystemDefaultDevice();
}
#pragma mark Impls
struct PixelFormatImpl {
    MTLPixelFormat data;
};
void PixelFormat::Create() {
    Release();
    impl = new PixelFormatImpl();
}
void PixelFormat::Release() {delete impl;}
    
struct LibraryImpl {
    id<MTLLibrary> data;
};
void Library::Create() {
    Release();
    impl = new LibraryImpl();
}
void Library::Release() {delete impl;}
    
struct EffectImpl {
    id<MTLFunction> vertex_fn;
    id<MTLFunction> fragment_fn;
};
void Effect::Create() {
    Release();
    impl = new EffectImpl();
}
void Effect::Release() {delete impl;}
    
struct PipelineDescImpl {
    MTLRenderPipelineDescriptor* data;
};
void PipelineDesc::Create() {
    Release();
    impl = new PipelineDescImpl();
    impl->data = [[MTLRenderPipelineDescriptor alloc] init];
}
void PipelineDesc::Release() {delete impl;}
    
struct PipelineStateImpl {id<MTLRenderPipelineState> data;};
void PipelineState::Create() {
    Release();
    impl = new PipelineStateImpl();
}
void PipelineState::Release() {delete impl;}
    
#pragma mark Member functions
bool PixelFormat::Load(const std::string &pixel_format){
    MTLPixelFormat& mtl_pixel_format(impl->data);
    // Find the requested format
    std::unordered_map<std::string,MTLPixelFormat> pixel_format_map{
        {"BGRA8U",MTLPixelFormatBGRA8Unorm}
    };
    const auto selected(pixel_format_map.find(pixel_format));
    if(selected != pixel_format_map.end()){
        mtl_pixel_format = selected->second;
    }else{
        return false;
    }
    return true;
}
void Library::Load(const std::string &name) {
    this->Create();
    if(name == ""){
        impl->data = [mtl_device newDefaultLibrary];
    }else{
        assert(0);
    }
    if(!impl->data) {
        NSLog(@">> ERROR: Couldnt create a shader library %s",name.c_str());
        assert(0);
    }
    impl->data.label = [[NSString alloc]initWithCString:name.c_str() encoding:NSASCIIStringEncoding];
}
void Effect::Load(Library& library, const std::string &vert_name, const std::string &frag_name){
    this->Create();
    id<MTLLibrary> mtl_library(library.impl->data);
    NSString* vert_nsstring = [[NSString alloc]initWithCString:vert_name.c_str() encoding:NSASCIIStringEncoding];
    impl->vertex_fn = [mtl_library newFunctionWithName:vert_nsstring];
    if(!impl->vertex_fn)
        NSLog(@">> ERROR: Couldn't load %@ vertex program from supplied library (%@)", vert_nsstring, mtl_library.label);
    NSString* frag_nsstring = [[NSString alloc]initWithCString:frag_name.c_str() encoding:NSASCIIStringEncoding];
    impl->fragment_fn = [mtl_library newFunctionWithName:frag_nsstring];
    if(!impl->fragment_fn)
        NSLog(@">> ERROR: Couldn't load %@ fragment program from supplied library (%@)", frag_nsstring, mtl_library.label);
}
}}