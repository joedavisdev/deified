#import "BasicPODScene.h"
#import "AAPLViewController.h"
#import "AAPLView.h"
#import "AAPLTransforms.h"
#import "UniformBlocks.h"

#import "pipeline.h"
#import "mesh.h"
#import "actorGroup.h"

#include <vector>
#include "body.hpp"

#include "PVRTModelPOD.h"
#include "PVRTResourceFile.h"

#include "scene_parser.hpp"

using namespace AAPL;
using namespace JMD;
using namespace simd;

static const long kInFlightCommandBuffers = 3;

static const float4 kAmbientColors[2] = {
    {0.18, 0.24, 0.8, 1.0},
    {0.8, 0.24, 0.1, 1.0}
};
static const float4 kDiffuseColors[2] = {
    {0.4, 0.4, 1.0, 1.0},
    {0.8, 0.4, 0.4, 1.0}
};

static const float kFOVY    = 65.0f;
static const float3 kEye    = {0.0f, 0.0f, 0.0f};
static const float3 kCenter = {0.0f, 0.0f, 1.0f};
static const float3 kUp     = {0.0f, 1.0f, 0.0f};

@implementation BasicPODScene
{
    // Semaphore for constant buffer synchronization
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
    
    // 3D Model
    CPVRTModelPOD _podModel;
    JMD::Body _body;
    Pipeline* _defaultPipeline;
    Mesh* _mesh;
    ConstantBufferGroup* _constantBufferGroup;
    
    JMD::SceneParser scene_parser;
}

- (instancetype)init {
    self = [super init];
    if (self) {
        _constantDataBufferIndex = 0;
        _inflightSemaphore = dispatch_semaphore_create(kInFlightCommandBuffers);
        _body.position.z = 0.25;
        _body.rotation = simd::float4(1.0);
    }
    return self;
}

#pragma mark Configure
- (void)configure:(AAPLView *)view {
    _mtlDevice = view.device;
    
    [self pvrFrameworkSetup];
    _mesh = [self loadModel:_podModel];
    
    // setup view with drawable formats
    view.depthPixelFormat   = MTLPixelFormatDepth32Float;
    view.stencilPixelFormat = MTLPixelFormatInvalid;
    view.sampleCount        = 1;
    
    // create a new command queue
    _mtlCommandQueue = [_mtlDevice newCommandQueue];
    
    _mtlDefaultLibrary = [_mtlDevice newDefaultLibrary];
    if(!_mtlDefaultLibrary) {
        NSLog(@">> ERROR: Couldnt create a default shader library");
        assert(0);
    }
    
    if (![self preparePipelineState:view]) {
        NSLog(@">> ERROR: Couldnt create a valid pipeline state");
        assert(0);
    }
    
    MTLDepthStencilDescriptor *mtlDepthStateDesc = [[MTLDepthStencilDescriptor alloc] init];
    mtlDepthStateDesc.depthCompareFunction = MTLCompareFunctionLess;
    mtlDepthStateDesc.depthWriteEnabled = YES;
    _mtlDepthState = [_mtlDevice newDepthStencilStateWithDescriptor:mtlDepthStateDesc];
    
    // Prepare constant buffer groups
    NSMutableArray* actorGroupArray = [[NSMutableArray alloc]init];
    NSMutableArray* bodies = [[NSMutableArray alloc]init];
    [bodies addObject:[NSValue valueWithPointer:&_body]];
    [actorGroupArray addObject: [[ActorGroup alloc]initWithMeshAndNSArray:_mesh bodyPtrs:bodies]];
    _constantBufferGroup = [[ConstantBufferGroup alloc]initPipelineAndActorGroups:_mtlDevice pipeline:_defaultPipeline uniformBlockSize:sizeof(JMD::UB::BasicLighting) actorGroups:actorGroupArray];
    
    for (int i = 0; i < kInFlightCommandBuffers; i++) {
        id<MTLBuffer> mtlConstantBuffer = [_constantBufferGroup getConstantBuffer:i];
        mtlConstantBuffer.label = [NSString stringWithFormat:@"ConstantBuffer%i", i];
        UB::BasicLighting *constantBuffer = (UB::BasicLighting*)[mtlConstantBuffer contents];
        constantBuffer[0].multiplier = 1;
        constantBuffer[0].ambientColor = kAmbientColors[0];
        constantBuffer[0].diffuseColor = kDiffuseColors[0];
    }
}
-(Mesh*)loadModel:(CPVRTModelPOD&)pod {
    SPODMesh& podMesh(pod.pMesh[0]); // TODO: Support more than one mesh
    return [[Mesh alloc]initWithBytes:_mtlDevice
                            vertexBuffer:(char*)podMesh.pInterleaved
                            numberOfVertices:podMesh.nNumVertex
                            stride:podMesh.sVertex.nStride
                            indexBuffer:(char*)podMesh.sFaces.pData
                            numberOfIndices:PVRTModelPODCountIndices(podMesh)
                            sizeOfIndices:podMesh.sFaces.nStride];
}
-(void)pvrFrameworkSetup {
    NSString* readPath = [NSString stringWithFormat:@"%@%@", [[NSBundle mainBundle] bundlePath], @"/"];
    CPVRTResourceFile::SetReadPath([readPath UTF8String]);
    CPVRTResourceFile::SetLoadReleaseFunctions(NULL, NULL);
    
    CPVRTResourceFile scene_json("scene.json");
    scene_parser.Parse(std::string((char *)scene_json.DataPtr()));
    
    if (_podModel.ReadFromFile("bunny.pod") != PVR_SUCCESS) {
        printf("ERROR: Couldn't load the .pod file\n");
        return;
    }
}
-(void)pvrFrameworkShutdown {
    _podModel.Destroy();
}
- (BOOL)preparePipelineState:(AAPLView *)view {
    MTLRenderPipelineDescriptor *mtlRenderPassPipelineDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
    mtlRenderPassPipelineDescriptor.sampleCount = view.sampleCount;
    mtlRenderPassPipelineDescriptor.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm;
    mtlRenderPassPipelineDescriptor.depthAttachmentPixelFormat = view.depthPixelFormat;
    
    Effect* effect = [[Effect alloc]initWithLibrary:_mtlDefaultLibrary vertexName:@"basicLightingVertex" fragmentName:@"basicLightingFragment"];
    _defaultPipeline = [[Pipeline alloc]initWithDescTemplate:_mtlDevice templatePipelineDesc:mtlRenderPassPipelineDescriptor effect:effect label:@"basicLighting"];
    return YES;
}

#pragma mark Render

- (void)render:(AAPLView *)view {
    dispatch_semaphore_wait(_inflightSemaphore, DISPATCH_TIME_FOREVER);
    [self updateConstantBuffer];
    
    // create a new command buffer for each renderpass to the current drawable
    id <MTLCommandBuffer> mtlCommandBuffer = [_mtlCommandQueue commandBuffer];
    
    // create a render command encoder so we can render into something
    MTLRenderPassDescriptor *mtlRenderPassDescriptor = view.renderPassDescriptor;
    if (mtlRenderPassDescriptor) {
        Mesh* mesh = _mesh;
        id <MTLRenderCommandEncoder> mtlRenderEncoder = [mtlCommandBuffer renderCommandEncoderWithDescriptor:mtlRenderPassDescriptor];
        id<MTLBuffer> constantBuffer = [_constantBufferGroup getConstantBuffer:_constantDataBufferIndex];
        [mtlRenderEncoder pushDebugGroup:@"Model"];
        [mtlRenderEncoder setDepthStencilState:_mtlDepthState];
        [mtlRenderEncoder setRenderPipelineState:[_constantBufferGroup pipeline].state];
        [mtlRenderEncoder setVertexBuffer:mesh.vertexBuffer offset:0 atIndex:0];
        [mtlRenderEncoder setVertexBuffer:constantBuffer offset:0 atIndex:1];
        
        // tell the render context we want to draw our primitives
        if(mesh.indexBuffer == NULL) {
            [mtlRenderEncoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:mesh.numberOfVertices];
        }else{
            MTLIndexType indexType(MTLIndexTypeUInt32);
            if(mesh.sizeOfIndices == 2) indexType = MTLIndexTypeUInt16;
            [mtlRenderEncoder drawIndexedPrimitives:MTLPrimitiveTypeTriangle indexCount:mesh.numberOfIndices indexType:indexType indexBuffer:mesh.indexBuffer indexBufferOffset:0];
        }
        
        [mtlRenderEncoder endEncoding];
        [mtlRenderEncoder popDebugGroup];
        
        // schedule a present once rendering to the framebuffer is complete
        [mtlCommandBuffer presentDrawable:view.currentDrawable];
    }
    
    // call the view's completion handler which is required by the view since it will signal its semaphore and set up the next buffer
    __block dispatch_semaphore_t blockSemaphore = _inflightSemaphore;
    [mtlCommandBuffer addCompletedHandler:^(id<MTLCommandBuffer> buffer) {
        dispatch_semaphore_signal(blockSemaphore);
    }];
    
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
    _body.rotation.x += timeSinceLastDraw * 50.0f;
}
// called every frame
- (void)updateConstantBuffer {    
    id<MTLBuffer> mtlConstantBuffer = [_constantBufferGroup getConstantBuffer:_constantDataBufferIndex];
    UB::BasicLighting *constantBuffer = (UB::BasicLighting*)[mtlConstantBuffer contents];
        const simd::float4 position = _body.position;
        const simd::float4 rotation = _body.rotation;
        simd::float4x4 modelViewMatrix
        = AAPL::translate(position.x, position.y, position.z) * AAPL::rotate(rotation.x, rotation.y, rotation.z, rotation.w);
        modelViewMatrix = _viewMatrix * modelViewMatrix;
        
        constantBuffer[0].normalMatrix = inverse(transpose(modelViewMatrix));
        constantBuffer[0].mvpMatrix = _projectionMatrix * modelViewMatrix;
        
        // change the color each frame
        // reverse direction if we've reached a boundary
        if (constantBuffer[0].ambientColor.y >= 0.8) {
            constantBuffer[0].multiplier = -1;
            constantBuffer[0].ambientColor.y = 0.79;
        } else if (constantBuffer[0].ambientColor.y <= 0.2) {
            constantBuffer[0].multiplier = 1;
            constantBuffer[0].ambientColor.y = 0.21;
        } else
            constantBuffer[0].ambientColor.y += constantBuffer[0].multiplier * 0.01;
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