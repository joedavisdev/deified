#include <string>

namespace JMD {
namespace GFX {
void LoadDevice();
class RenderAttachmentDesc {
public:
    bool SetPixelFormat(const std::string &pixel_format);
    unsigned int pixel_format() const {return pixel_format_;}
private:
    unsigned int pixel_format_;
};
class CommandBuffer {
};
struct LibraryImpl;
class Library {
public:
    Library();
    ~Library();
    void Load(const std::string& name);
    void Release();
    LibraryImpl* impl;
};
class PipelineDesc {
};
}}