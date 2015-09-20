#pragma once
#ifdef __cplusplus
#include <unordered_map>
#include <string>

namespace JMD {
    namespace UB { // Uniform block
       // Enums for the structs defined in UniformBlocks.h
        enum Index {
            BASIC_LIGHTING
        };
        std::unordered_map<std::string,int> IndexNames {
            {"BasicLighting",BASIC_LIGHTING}
        };
    }
}
#endif // __cplusplus