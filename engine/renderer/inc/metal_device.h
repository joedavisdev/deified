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