#import "mesh.h"

@implementation Mesh {
}
-(id)initWithBytes:(id <MTLDevice>)device vertexBuffer:(const char[]) vertexBuffer vertexBufferLength:(NSUInteger)vertexBufferLength
       indexBuffer:(const char[]) indexBuffer indexBufferLength:(NSUInteger) indexBufferLength {
    if( self = [super init]) {
        assert(vertexBuffer != nil);
        _vertexBuffer = [device newBufferWithBytes:vertexBuffer length:vertexBufferLength options:MTLResourceOptionCPUCacheModeDefault];
        if(indexBuffer != nil) {
            _indexBuffer = [device newBufferWithBytes:indexBuffer length:indexBufferLength options:MTLResourceOptionCPUCacheModeDefault];
        }
    }
    return self;
}
@end