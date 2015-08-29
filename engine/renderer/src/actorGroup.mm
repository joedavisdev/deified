#import "actorGroup.h"

@implementation ActorGroup {
}
- (id)initWithMeshAndNSArray: (Mesh*)mesh bodyPtrs: (NSArray*)bodyPtrs {
    if(self = [super init]) {
        _mesh = mesh;
        _bodyPtrs = [[NSArray alloc]initWithArray:bodyPtrs];
    }
    return self;
}
@end

static const long kInFlightCommandBuffers = 3;
@implementation ConstantBufferGroup {
    id <MTLBuffer> constantBuffers[kInFlightCommandBuffers];
}
- (id)initPipelineAndActorGroups: (id<MTLDevice>)device pipeline:(Pipeline*)pipeline uniformBlockSize:(NSUInteger)uniformBlockSize actorGroups:(NSMutableArray*)actorGroups {
    if(self = [super init]) {
        _pipeline = pipeline;
        _actorGroups = [[NSMutableArray alloc]initWithArray:actorGroups];
        NSUInteger numberOfActors(0);
        for (ActorGroup* actorGroup in _actorGroups) {
            numberOfActors += [[actorGroup bodyPtrs]count];
        }
        for (int index = 0; index < kInFlightCommandBuffers; index++) {
            constantBuffers[index] = [device newBufferWithLength:(numberOfActors*uniformBlockSize) options:0];
        }
    }
    return self;
}
- (id<MTLBuffer>)getConstantBuffer: (NSUInteger)bufferIndex {
    if(bufferIndex > kInFlightCommandBuffers)
        return NULL;
    return constantBuffers[bufferIndex];
}
@end