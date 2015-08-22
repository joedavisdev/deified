#import "actorGroup.h"
#import "AAPLSharedTypes.h"

#include "body.hpp"

@implementation ActorGroup : NSObject {
}
- (id)initWithMeshAndBodies: (Mesh*)mesh bodies: (const void*)bodies numberOfBodies:(NSUInteger)numberOfBodies {
    if(self = [super init]) {
        _mesh = mesh;
        _bodyPtrs = new void*[numberOfBodies];
        memcpy(_bodyPtrs, bodies, numberOfBodies);
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