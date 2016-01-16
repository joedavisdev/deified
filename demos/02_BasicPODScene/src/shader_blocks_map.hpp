#pragma once
#include "shader_blocks.hpp"

#include <string>
#include <unordered_map>

namespace JMD {
    std::unordered_map<std::string,unsigned int> attribute_block_sizes{
        {"PositionNormal",sizeof(JMD::AB::PositionNormal)}
    };
    std::unordered_map<std::string,unsigned int> uniform_block_sizes{
        {"BasicLighting",sizeof(JMD::UB::BasicLighting)}
    };
}