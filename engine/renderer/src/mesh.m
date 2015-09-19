#import "mesh.h"

@implementation Mesh {
}
-(id)initWithBytes:(id <MTLDevice>)device vertexBuffer:(const char[]) vertexBuffer numberOfVertices:(NSUInteger)numberOfVertices stride:(NSUInteger)stride indexBuffer:(const char[]) indexBuffer indexBufferLength:(NSUInteger) indexBufferLength {
    if( self = [super init]) {
        assert(vertexBuffer != nil);
        _numberOfVertices = numberOfVertices;
        _stride = stride;
        const NSUInteger vertexBufferLength =_numberOfVertices * _stride;
        _vertexBuffer = [device newBufferWithBytes:vertexBuffer length:vertexBufferLength options:MTLResourceOptionCPUCacheModeDefault];
        if(indexBuffer != nil) {
            _indexBuffer = [device newBufferWithBytes:indexBuffer length:indexBufferLength options:MTLResourceOptionCPUCacheModeDefault];
        }
    }
    return self;
}
@end