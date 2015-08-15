#import <Foundation/Foundation.h>
#import <Metal/Metal.h>
#import "mesh.h"
#import "pipeline.h"

// TODO: Make Object a C++ API independant class
@interface Object : NSObject {
}
//@property (nonatomic, readonly) float4 position;
//@property (nonatomic, readonly) float4 rotation;
@end

@interface ActorGroup : NSObject {
}
@property (nonatomic, readonly) Mesh* mesh;
@property (nonatomic, readonly) NSMutableArray* objectPtrs;
@end

@interface ConstantBufferGroup : NSObject {
}
@property (nonatomic, readonly) Pipeline* pipeline;
@property (nonatomic, readonly) NSMutableArray* actorGroupPtrs;
@end