#import "pipeline.h"

@implementation Effect {
}

- (id)initWithLibrary: (id<MTLLibrary>)library vertexName: (NSString*)vertexName fragmentName: (NSString*)fragmentName {
    if (self = [super init]) {
        _vertexProgram = [library newFunctionWithName:vertexName];
        if(!_vertexProgram)
            NSLog(@">> ERROR: Couldn't load %@ vertex program from supplied library (%@)", vertexName, library.label);
        _fragmentProgram = [library newFunctionWithName:fragmentName];
        if(!_fragmentProgram)
            NSLog(@">> ERROR: Couldn't load %@ fragment program from supplied library (%@)", fragmentName, library.label);
    }
    return self;
}
@end

@implementation Pipeline {
}
- (id)initWithDescTemplate: (id <MTLDevice>)device templatePipelineDesc: (MTLRenderPipelineDescriptor*)templatePipelineDesc effect: (Effect*)effect {
    if (self = [super init]) {
        MTLRenderPipelineDescriptor *pipelineDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
        pipelineDescriptor.sampleCount                     = templatePipelineDesc.sampleCount;
        pipelineDescriptor.vertexFunction                  = effect.vertexProgram;
        pipelineDescriptor.fragmentFunction                = effect.fragmentProgram;
        pipelineDescriptor.colorAttachments[0].pixelFormat = templatePipelineDesc.colorAttachments[0].pixelFormat;
        pipelineDescriptor.depthAttachmentPixelFormat      = templatePipelineDesc.depthAttachmentPixelFormat;
        
        NSError *error = nil;
        _state = [device newRenderPipelineStateWithDescriptor:pipelineDescriptor error:&error];
        if(!_state) {
            NSLog(@">> ERROR: Failed Aquiring pipeline state: %@", error);
        }
    }
    return self;
}
@end