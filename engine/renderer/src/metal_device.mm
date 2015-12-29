#include "metal_device.h"

#import <Metal/Metal.h>

#include <unordered_map>

namespace JMD {
namespace GFX {
static id <MTLDevice> mtl_device;
void LoadDevice() {
    mtl_device = MTLCreateSystemDefaultDevice();
}
struct LibraryImpl {id<MTLLibrary> library;};
Library::Library() {impl = new LibraryImpl();}
Library::~Library() {delete impl;}
bool RenderAttachmentDesc::SetPixelFormat(const std::string &pixel_format){
    // Find the requested format
    std::unordered_map<std::string,MTLPixelFormat> pixel_format_map{
        {"BGRA8U",MTLPixelFormatBGRA8Unorm}
    };
    const auto selected(pixel_format_map.find(pixel_format));
    MTLPixelFormat mtl_pixel_format((MTLPixelFormat)0);
    if(selected != pixel_format_map.end()){
        mtl_pixel_format = selected->second;
    }else{
        return false;
    }
    
    pixel_format_ = (unsigned int)mtl_pixel_format;
    return true;
}
void Library::Load(const std::string &name) {
    id<MTLLibrary> library(impl->library);
    if(name == ""){
        library = [mtl_device newDefaultLibrary];
    }else{
        assert(0);
    }
    if(!library) {
        NSLog(@">> ERROR: Couldnt create a shader library %s",name.c_str());
        assert(0);
    }
}
}}