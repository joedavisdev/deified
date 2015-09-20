#import "mesh.h"

@implementation Mesh {
}
-(id)initWithBytes:(id <MTLDevice>)device vertexBuffer:(const char[]) vertexBuffer numberOfVertices:(NSUInteger)numberOfVertices stride:(NSUInteger)stride indexBuffer:(const char[]) indexBuffer numberOfIndices:(NSUInteger) numberOfIndices sizeOfIndices:(NSUInteger) sizeOfIndices {
    if( self = [super init]) {
        assert(vertexBuffer != nil);
        _numberOfVertices = numberOfVertices;
        _stride = stride;
        _numberOfIndices = numberOfIndices;
        _sizeOfIndices = sizeOfIndices;
        const NSUInteger vertexBufferLength =_numberOfVertices * _stride;
        const NSUInteger indexBufferLength =_numberOfIndices * _sizeOfIndices;
        _vertexBuffer = [device newBufferWithBytes:vertexBuffer length:vertexBufferLength options:MTLResourceOptionCPUCacheModeDefault];
        if(indexBuffer != nil) {
            _indexBuffer = [device newBufferWithBytes:indexBuffer length:indexBufferLength options:MTLResourceOptionCPUCacheModeDefault];
        }
    }
    return self;
}
@end