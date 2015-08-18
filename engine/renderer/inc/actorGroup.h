#import <Foundation/Foundation.h>
#import <Metal/Metal.h>
#import "mesh.h"
#import "pipeline.h"

@interface ActorGroup : NSObject {
}
@property (nonatomic, readonly) Mesh* mesh;
@property (nonatomic, readonly) NSMutableArray* bodyPtrs;
- (id)initWithMeshAndBodies: (Mesh*)mesh bodies: (NSMutableArray*)bodies;
@end

@interface ConstantBufferGroup : NSObject {
}
@property (nonatomic, readonly) Pipeline* pipeline;
@property (nonatomic, readonly) NSMutableArray* actorGroupPtrs;
- (id)initPipelineAndActorGroups: (Pipeline*)pipeline bodies: (NSMutableArray*)actor_groups;
@end