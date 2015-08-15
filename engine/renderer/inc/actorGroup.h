#import <Foundation/Foundation.h>
#import <Metal/Metal.h>
#import "mesh.h"
#import "pipeline.h"

// TODO: Make Actor a C++ API independant class
@interface Actor : NSObject {
}
//@property (nonatomic, readonly) float4 position;
//@property (nonatomic, readonly) float4 rotation;
@endx

@interface ActorGroup : NSObject {
}
@property (nonatomic, readonly) Mesh* mesh;
@property (nonatomic, readonly) NSMutableArray* actorPtrs;
@end

@interface ConstantBufferGroup : NSObject {
}
@property (nonatomic, readonly) Pipeline* pipeline;
@property (nonatomic, readonly) NSMutableArray* actorGroupPtrs;
@end