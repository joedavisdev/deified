#pragma once
#include "simd/simd.h"

namespace JMD {
struct Body {
    simd::float4 position;
    simd::float4 rotation;
    Body()
    :
    position(0.0),
    rotation(0.0)
    {}
};
}