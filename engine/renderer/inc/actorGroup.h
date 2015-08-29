#import <Foundation/Foundation.h>
#import <Metal/Metal.h>
#import "mesh.h"
#import "pipeline.h"

@interface ActorGroup : NSObject {
}
@property (nonatomic, readonly) __weak Mesh* mesh;
@property (nonatomic, readonly) NSArray* bodyPtrs;
- (id)initWithMeshAndNSArray: (Mesh*)mesh bodyPtrs: (NSArray*)bodyPtrs;
@end

@interface ConstantBufferGroup : NSObject {
}
@property (nonatomic, readonly) __weak Pipeline* pipeline;
@property (nonatomic, readonly) NSMutableArray* actorGroups;
- (id)initPipelineAndActorGroups: (id<MTLDevice>)device pipeline:(Pipeline*)pipeline uniformBlockSize:(NSUInteger)uniformBlockSize actorGroups: (NSMutableArray*)actorGroups;
- (id<MTLBuffer>)getConstantBuffer: (NSUInteger)bufferIndex;
@end