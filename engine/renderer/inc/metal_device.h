#include <string>

namespace JMD {
namespace GFX {
class RenderAttachment {
public:
    unsigned int set_pixel_format(const std::string &pixel_format);
    unsigned int pixel_format() const {return pixel_format_;}
private:
    unsigned int pixel_format_;
};
}}