#import "effect.h"

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