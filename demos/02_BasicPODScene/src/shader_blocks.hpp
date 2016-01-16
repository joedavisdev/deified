#pragma once
#import <simd/simd.h>

#ifdef __cplusplus
namespace JMD {
    namespace AB { // Atrribute blocks
        struct PositionNormal {
            simd::float4   position;
            simd::float4   normal;
        };
    }
    namespace UB { // Uniform blocks
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