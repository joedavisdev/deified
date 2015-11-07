#import "AAPLRenderer.h"
#import "AAPLViewController.h"
#import "AAPLView.h"
#import "AAPLTransforms.h"
#import "UniformBlocks.h"

#import "pipeline.h"
#import "mesh.h"
#import "actorGroup.h"

#include <vector>
#include "body.hpp"

using namespace AAPL;
using namespace JMD;
using namespace simd;

static const long kInFlightCommandBuffers = 3;

static const NSUInteger kNumberOfBoxes = 2;
static const float4 kBoxAmbientColors[2] = {
    {0.18, 0.24, 0.8, 1.0},
    {0.8, 0.24, 0.1, 1.0}
};

static const float4 kBoxDiffuseColors[2] = {
    {0.4, 0.4, 1.0, 1.0},
    {0.8, 0.4, 0.4, 1.0}
};

static const float kFOVY    = 65.0f;
static const float3 kEye    = {0.0f, 0.0f, 0.0f};
static const float3 kCenter = {0.0f, 0.0f, 1.0f};
static const float3 kUp     = {0.0f, 1.0f, 0.0f};

static const float kWidth  = 0.75f;
static const float kHeight = 0.75f;
static const float kDepth  = 0.75f;

static const NSUInteger kNumCubeVertices = 36;
static const float kCubeVertexData[] = {
    kWidth, -kHeight, kDepth,   0.0, -1.0,  0.0,
    -kWidth, -kHeight, kDepth,   0.0, -1.0, 0.0,
    -kWidth, -kHeight, -kDepth,   0.0, -1.0,  0.0,
    kWidth, -kHeight, -kDepth,  0.0, -1.0,  0.0,
    kWidth, -kHeight, kDepth,   0.0, -1.0,  0.0,
    -kWidth, -kHeight, -kDepth,   0.0, -1.0,  0.0,
    
    kWidth, kHeight, kDepth,    1.0, 0.0,  0.0,
    kWidth, -kHeight, kDepth,   1.0,  0.0,  0.0,
    kWidth, -kHeight, -kDepth,  1.0,  0.0,  0.0,
    kWidth, kHeight, -kDepth,   1.0, 0.0,  0.0,
    kWidth, kHeight, kDepth,    1.0, 0.0,  0.0,
    kWidth, -kHeight, -kDepth,  1.0,  0.0,  0.0,
    
    -kWidth, kHeight, kDepth,    0.0, 1.0,  0.0,
    kWidth, kHeight, kDepth,    0.0, 1.0,  0.0,
    kWidth, kHeight, -kDepth,   0.0, 1.0,  0.0,
    -kWidth, kHeight, -kDepth,   0.0, 1.0,  0.0,
    -kWidth, kHeight, kDepth,    0.0, 1.0,  0.0,
    kWidth, kHeight, -kDepth,   0.0, 1.0,  0.0,
    
    -kWidth, -kHeight, kDepth,  -1.0,  0.0, 0.0,
    -kWidth, kHeight, kDepth,   -1.0, 0.0,  0.0,
    -kWidth, kHeight, -kDepth,  -1.0, 0.0,  0.0,
    -kWidth, -kHeight, -kDepth,  -1.0,  0.0,  0.0,
    -kWidth, -kHeight, kDepth,  -1.0,  0.0, 0.0,
    -kWidth, kHeight, -kDepth,  -1.0, 0.0,  0.0,
    
    kWidth, kHeight,  kDepth,  0.0, 0.0,  1.0,
    -kWidth, kHeight,  kDepth,  0.0, 0.0,  1.0,
    -kWidth, -kHeight, kDepth,   0.0,  0.0, 1.0,
    -kWidth, -kHeight, kDepth,   0.0,  0.0, 1.0,
    kWidth, -kHeight, kDepth,   0.0,  0.0,  1.0,
    kWidth, kHeight,  kDepth,  0.0, 0.0,  1.0,
    
    kWidth, -kHeight, -kDepth,  0.0,  0.0, -1.0,
    -kWidth, -kHeight, -kDepth,   0.0,  0.0, -1.0,
    -kWidth, kHeight, -kDepth,  0.0, 0.0, -1.0,
    kWidth, kHeight, -kDepth,  0.0, 0.0, -1.0,
    kWidth, -kHeight, -kDepth,  0.0,  0.0, -1.0,
    -kWidth, kHeight, -kDepth,  0.0, 0.0, -1.0
};

@implementation AAPLRenderer {
    dispatch_semaphore_t _inflightSemaphore;
    
    // renderer global ivars
    id <MTLDevice> _mtlDevice;
    id <MTLCommandQueue> _mtlCommandQueue;
    id <MTLLibrary> _mtlDefaultLibrary;
    id <MTLDepthStencilState> _mtlDepthState;
    
    // globals used in update calculation
    float4x4 _projectionMatrix;
    float4x4 _viewMatrix;
    float _rotation;

    NSUInteger _constantDataBufferIndex;
    
    Pipeline* _defaultPipeline;
    Mesh* _mesh;
    JMD::Body _bodies[kNumberOfBoxes];
    ConstantBufferGroup* _constantBufferGroup;
}

- (instancetype)init {
    self = [super init];
    if (self) {
        _constantDataBufferIndex = 0;
        _inflightSemaphore = dispatch_semaphore_create(kInFlightCommandBuffers);
        for(auto& body: _bodies) {
            body.rotation += simd::float4(1.0);
        }
        _bodies[0].position.z = 1.5f;
        _bodies[1].position.z = -1.5f;
    }
    return self;
}

#pragma mark Configure
- (void)configure:(AAPLView *)view {
    _mtlDevice = view.device;
    
    // setup view with drawable formats
    view.depthPixelFormat   = MTLPixelFormatDepth32Float;
    view.stencilPixelFormat = MTLPixelFormatInvalid;
    view.sampleCount        = 1;
    
    // create a new command queue
    _mtlCommandQueue = [_mtlDevice newCommandQueue];
    
    _mtlDefaultLibrary = [_mtlDevice newDefaultLibrary];
    if(!_mtlDefaultLibrary) {
        NSLog(@">> ERROR: Couldnt create a default shader library");
        // assert here becuase if the shader libary isn't loading, nothing good will happen
        assert(0);
    }
    
    if (![self preparePipelineState:view]) {
        NSLog(@">> ERROR: Couldnt create a valid pipeline state");
        
        // cannot render anything without a valid compiled pipeline state object.
        assert(0);
    }
    // Load meshes
    _mesh = [[Mesh alloc]initWithBytes:_mtlDevice vertexBuffer:(char*)kCubeVertexData numberOfVertices:kNumCubeVertices stride:sizeof(float)*6 indexBuffer:NULL numberOfIndices:0 sizeOfIndices:0];
    // Prepare constant buffer groups
    NSMutableArray* actorGroupArray = [[NSMutableArray alloc]init];
    NSMutableArray* bodies = [[NSMutableArray alloc]init];
    for(unsigned int index = 0; index < kNumberOfBoxes; ++index) {
        [bodies addObject:[NSValue valueWithPointer:&_bodies[index]]];
    }
    [actorGroupArray addObject: [[ActorGroup alloc]initWithMeshAndNSArray:_mesh bodyPtrs:bodies]];
    
    _constantBufferGroup = [[ConstantBufferGroup alloc]initPipelineAndActorGroups:_mtlDevice pipeline:_defaultPipeline uniformBlockSize:sizeof(JMD::UB::CubeLighting) actorGroups:actorGroupArray];
    
    
    MTLDepthStencilDescriptor *mtlDepthStateDesc = [[MTLDepthStencilDescriptor alloc] init];
    mtlDepthStateDesc.depthCompareFunction = MTLCompareFunctionLess;
    mtlDepthStateDesc.depthWriteEnabled = YES;
    _mtlDepthState = [_mtlDevice newDepthStencilStateWithDescriptor:mtlDepthStateDesc];
    
    for (int i = 0; i < kInFlightCommandBuffers; i++) {
        id<MTLBuffer> mtlConstantBuffer = [_constantBufferGroup getConstantBuffer:i];
        mtlConstantBuffer.label = [NSString stringWithFormat:@"ConstantBuffer%i", i];
        
        // write initial color values for both cubes (at each offset).
        // Note, these will get animated during update
        UB::CubeLighting *constantBuffer = (UB::CubeLighting *)[mtlConstantBuffer contents];
        for (int j = 0; j < kNumberOfBoxes; j++) {
            if (j%2==0) {
                constantBuffer[j].multiplier = 1;
                constantBuffer[j].ambient_color = kBoxAmbientColors[0];
                constantBuffer[j].diffuse_color = kBoxDiffuseColors[0];
            }
            else {
                constantBuffer[j].multiplier = -1;
                constantBuffer[j].ambient_color = kBoxAmbientColors[1];
                constantBuffer[j].diffuse_color = kBoxDiffuseColors[1];
            }
        }
    }
}

- (BOOL)preparePipelineState:(AAPLView *)view {
    MTLRenderPipelineDescriptor *mtlRenderPassPipelineDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
    mtlRenderPassPipelineDescriptor.sampleCount = view.sampleCount;
    mtlRenderPassPipelineDescriptor.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm;
    mtlRenderPassPipelineDescriptor.depthAttachmentPixelFormat = view.depthPixelFormat;
    
    Effect* effect = [[Effect alloc]initWithLibrary:_mtlDefaultLibrary vertexName:@"lighting_vertex" fragmentName:@"lighting_fragment"];
    _defaultPipeline = [[Pipeline alloc]initWithDescTemplate:_mtlDevice templatePipelineDesc:mtlRenderPassPipelineDescriptor effect:effect label:@"basic_lighting"];
    return YES;
}

#pragma mark Render

- (void)render:(AAPLView *)view {
    dispatch_semaphore_wait(_inflightSemaphore, DISPATCH_TIME_FOREVER);
    
    // Prior to sending any data to the GPU, constant buffers should be updated accordingly on the CPU.
    [self updateConstantBuffer];
    
    // create a new command buffer for each renderpass to the current drawable
    id <MTLCommandBuffer> mtlCommandBuffer = [_mtlCommandQueue commandBuffer];
    
    // create a render command encoder so we can render into something
    MTLRenderPassDescriptor *mtlRenderPassDescriptor = view.renderPassDescriptor;
    if (mtlRenderPassDescriptor) {
        id <MTLRenderCommandEncoder> mtlRenderEncoder = [mtlCommandBuffer renderCommandEncoderWithDescriptor:mtlRenderPassDescriptor];
        id<MTLBuffer> constantBuffer = [_constantBufferGroup getConstantBuffer:_constantDataBufferIndex];
        [mtlRenderEncoder pushDebugGroup:@"Boxes"];
        [mtlRenderEncoder setDepthStencilState:_mtlDepthState];
        [mtlRenderEncoder setRenderPipelineState:[_constantBufferGroup pipeline].state];
        [mtlRenderEncoder setVertexBuffer:_mesh.vertexBuffer offset:0 atIndex:0 ];
        
        for (int i = 0; i < kNumberOfBoxes; i++) {
            //  set constant buffer for each box
            [mtlRenderEncoder setVertexBuffer:constantBuffer offset:i*sizeof(UB::CubeLighting) atIndex:1 ];
            
            // tell the render context we want to draw our primitives
            [mtlRenderEncoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:kNumCubeVertices];
        }
        
        [mtlRenderEncoder endEncoding];
        [mtlRenderEncoder popDebugGroup];
        
        // schedule a present once rendering to the framebuffer is complete
        [mtlCommandBuffer presentDrawable:view.currentDrawable];
    }
    
    // call the view's completion handler which is required by the view since it will signal its semaphore and set up the next buffer
    __block dispatch_semaphore_t block_sema = _inflightSemaphore;
    [mtlCommandBuffer addCompletedHandler:^(id<MTLCommandBuffer> buffer) {
        dispatch_semaphore_signal(block_sema);
    }];
    
    // finalize rendering here. this will push the command buffer to the GPU
    [mtlCommandBuffer commit];
    _constantDataBufferIndex = (_constantDataBufferIndex + 1) % kInFlightCommandBuffers;
}

- (void)reshape:(AAPLView *)view {
    // when reshape is called, update the view and projection matricies since this means the view orientation or size changed
    float aspect = fabsf((float)view.bounds.size.width / (float)view.bounds.size.height);
    _projectionMatrix = perspective_fov(kFOVY, aspect, 0.1f, 100.0f);
    _viewMatrix = lookAt(kEye, kCenter, kUp);
}

#pragma mark Update
- (void)updateBodies:(NSTimeInterval)timeSinceLastDraw {
    for(auto& body: _bodies) {
        body.rotation.x += timeSinceLastDraw * 50.0f;
    }
}
// called every frame
- (void)updateConstantBuffer {
    float4x4 baseModelViewMatrix = translate(0.0f, 0.0f, 5.0f) * rotate(_rotation, 1.0f, 1.0f, 1.0f);
    baseModelViewMatrix = _viewMatrix * baseModelViewMatrix;
    
    id<MTLBuffer> mtlConstantBuffer = [_constantBufferGroup getConstantBuffer:_constantDataBufferIndex];
    UB::CubeLighting *constantBuffer = (UB::CubeLighting *)[mtlConstantBuffer contents];
    for (int i = 0; i < kNumberOfBoxes; i++) {
        const simd::float4 position = _bodies[i].position;
        const simd::float4 rotation = _bodies[i].rotation;
        simd::float4x4 modelViewMatrix
            = AAPL::translate(position.x, position.y, position.z) * AAPL::rotate(rotation.x, rotation.y, rotation.z, rotation.w);
        modelViewMatrix = baseModelViewMatrix * modelViewMatrix;
        
        constantBuffer[i].normal_matrix = inverse(transpose(modelViewMatrix));
        constantBuffer[i].modelview_projection_matrix = _projectionMatrix * modelViewMatrix;
        
        // change the color each frame
        // reverse direction if we've reached a boundary
        if (constantBuffer[i].ambient_color.y >= 0.8) {
            constantBuffer[i].multiplier = -1;
            constantBuffer[i].ambient_color.y = 0.79;
        } else if (constantBuffer[i].ambient_color.y <= 0.2) {
            constantBuffer[i].multiplier = 1;
            constantBuffer[i].ambient_color.y = 0.21;
        } else
            constantBuffer[i].ambient_color.y += constantBuffer[i].multiplier * 0.01*i;
    }
}

// just use this to update app globals
- (void)update:(AAPLViewController *)controller {
    [self updateBodies:controller.timeSinceLastDraw];
    _rotation += controller.timeSinceLastDraw * 50.0f;
}

- (void)viewController:(AAPLViewController *)controller willPause:(BOOL)pause {
    // timer is suspended/resumed
    // Can do any non-rendering related background work here when suspended
}

@end
