#pragma once
#import <simd/simd.h>

#ifdef __cplusplus
#include <unordered_map>
#include <string>

namespace JMD {
    namespace UB { // Uniform block
        enum Index {
            CUBE_LIGHTING
        };
        std::unordered_map<std::string,int> IndexNames {
            {"CubeLighting",CUBE_LIGHTING}
        };

        struct CubeLighting {
            simd::float4x4 modelview_projection_matrix;
            simd::float4x4 normal_matrix;
            simd::float4   ambient_color;
            simd::float4   diffuse_color;
            int            multiplier;
        };
    }
}
#endif