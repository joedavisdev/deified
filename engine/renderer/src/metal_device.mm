#include "metal_device.h"

#import <Metal/Metal.h>

#include <unordered_map>

namespace JMD {
namespace GFX {
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
}}