#pragma once
#ifdef __cplusplus
#include <unordered_map>
#include <string>

namespace JMD {
    namespace UB { // Uniform block
       // Enums for the structs defined in UniformBlocks.h
        enum Index {
            CUBE_LIGHTING
        };
        std::unordered_map<std::string,int> IndexNames {
            {"CubeLighting",CUBE_LIGHTING}
        };
    }
}
#endif // __cplusplus