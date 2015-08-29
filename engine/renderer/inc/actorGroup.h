#import <Foundation/Foundation.h>
#import <Metal/Metal.h>
#import "mesh.h"
#import "pipeline.h"

@interface ActorGroup : NSObject {
}
@property (nonatomic, readonly) Mesh* mesh;
@property (nonatomic, readonly) void* bodyPtrs;
- (id)initWithMeshAndBodies: (Mesh*)mesh bodies: (const void*)bodies numberOfBodies:(NSUInteger)numberOfBodies;
@end

@interface ConstantBufferGroup : NSObject {
}
@property (nonatomic, readonly) Pipeline* pipeline;
@property (nonatomic, readonly) NSMutableArray* actorGroupPtrs;
- (id)initPipelineAndActorGroups: (id<MTLDevice>)device pipeline:(Pipeline*)pipeline uniformBlockSize:(NSUInteger)uniformBlockSize actorGroups: (NSMutableArray*)actorGroups;
- (id<MTLBuffer>)getConstantBuffer: (NSUInteger)bufferIndex;
@end