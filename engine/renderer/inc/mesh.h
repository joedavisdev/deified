#import <Foundation/Foundation.h>
#import <Metal/Metal.h>

@interface Mesh: NSObject {
}
@property (nonatomic, readonly) id <MTLBuffer> vertexBuffer;
@property (nonatomic, readonly) id <MTLBuffer> indexBuffer;
-(id)initWithBytes:(id <MTLDevice>)device vertexBuffer:(const float[]) vertexBuffer vertexBufferLength:(NSUInteger)vertexBufferLength
       indexBuffer:(const int[]) indexBuffer indexBufferLength:(NSUInteger) indexBufferLength;
@end