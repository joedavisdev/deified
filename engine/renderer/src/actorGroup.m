#import "actorGroup.h"

#import "AAPLSharedTypes.h"

@implementation ActorGroup : NSObject {
}
- (id)initWithMeshAndBodies: (Mesh*)mesh bodies: (NSMutableArray*)bodies {
    if(self = [super init]) {
        _mesh = mesh;
        _bodyPtrs = [[NSMutableArray alloc]initWithArray:bodies];
    }
    return self;
}
@end

@implementation ConstantBufferGroup : NSObject {
}
- (id)initPipelineAndActorGroups: (Pipeline*)pipeline bodies: (NSMutableArray*)actor_groups {
    if(self = [super init]) {
        _pipeline = pipeline;
        _actorGroupPtrs = [[NSMutableArray alloc]initWithArray:actor_groups];
    }
    return self;
}
@end