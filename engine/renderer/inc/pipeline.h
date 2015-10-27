#import <Foundation/Foundation.h>
#import <Metal/Metal.h>

@interface Effect: NSObject {
}
@property (nonatomic, readonly) id <MTLFunction> vertexProgram;
@property (nonatomic, readonly) id <MTLFunction> fragmentProgram;

- (id)initWithLibrary: (id<MTLLibrary>)library vertexName: (NSString*)vertexName fragmentName: (NSString*)fragmentName;
@end

@interface Pipeline: NSObject {
}
@property (nonatomic, readonly) id <MTLRenderPipelineState> state;

- (id)initWithDescTemplate: (id <MTLDevice>)device templatePipelineDesc: (MTLRenderPipelineDescriptor*)templatePipelineDesc effect: (Effect*)effect label:(NSString*)label;
@end