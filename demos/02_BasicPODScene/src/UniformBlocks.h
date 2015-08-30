#pragma once
#import <simd/simd.h>

#ifdef __cplusplus
namespace JMD {
namespace UB { // Uniform block
    struct CubeLighting {
        simd::float4x4 modelview_projection_matrix;
        simd::float4x4 normal_matrix;
        simd::float4   ambient_color;
        simd::float4   diffuse_color;
        int            multiplier;
    };
}
}
#endif // __cplusplus