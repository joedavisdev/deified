#pragma once
#import <simd/simd.h>

#ifdef __cplusplus
namespace JMD {
namespace UB { // Uniform block
    struct BasicLighting {
        simd::float4x4 mvpMatrix;
        simd::float4x4 normalMatrix;
        simd::float4   ambientColor;
        simd::float4   diffuseColor;
        int            multiplier;
    };
}
}
#endif // __cplusplus