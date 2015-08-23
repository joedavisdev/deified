#import "mesh.h"

@implementation Mesh: NSObject {
}
-(id)initWithBytes:(id <MTLDevice>)device vertexBuffer:(const float[]) vertexBuffer vertexBufferLength:(NSUInteger)vertexBufferLength
       indexBuffer:(const int[]) indexBuffer indexBufferLength:(NSUInteger) indexBufferLength {
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