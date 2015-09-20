/*
 Copyright (C) 2015 Apple Inc. All Rights Reserved.
 See LICENSE.txt for this sample’s licensing information
 
 Abstract:
 lighting shader for Basic Metal 3D
 */

#include <metal_stdlib>
#include <simd/simd.h>
#include "UniformBlocks.h"

using namespace metal;
using namespace JMD;

// variables in constant address space
constant float3 _lightPosition = float3(0.0, 1.0, -1.0);

typedef struct
{
	packed_float3 position;
	packed_float3 normal;
} _attributeLayout;

struct VertexOutput {
    float4 position [[position]];
    half4 color;
};

// vertex shader function
vertex VertexOutput basicLightingVertex(device _attributeLayout* vertexArray [[ buffer(0) ]],
                                  constant UB::BasicLighting& constants [[ buffer(1) ]],
                                  unsigned int vertexID [[ vertex_id ]]) {
    VertexOutput out;
    
	float4 position = float4(float3(vertexArray[vertexID].position), 1.0);
    out.position = constants.mvpMatrix * position;
    
    float3 normal = vertexArray[vertexID].normal;
    float4 eyeNormal = normalize(constants.normalMatrix * float4(normal, 0.0));
    float nDotL = dot(eyeNormal.rgb, normalize(_lightPosition));
    nDotL = fmax(0.0, nDotL);
    
    out.color = half4(constants.ambientColor + constants.diffuseColor * nDotL);
    
    return out;
}

// fragment shader function
fragment half4 basicLightingFragment(VertexOutput in [[stage_in]]) {
    return in.color;
};

