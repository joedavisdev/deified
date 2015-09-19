#import <Foundation/Foundation.h>
#import <Metal/Metal.h>

@interface Mesh: NSObject {
}
@property (nonatomic, readonly) id <MTLBuffer> vertexBuffer;
@property (nonatomic, readonly) id <MTLBuffer> indexBuffer;
-(id)initWithBytes:(id <MTLDevice>)device vertexBuffer:(const char[]) vertexBuffer vertexBufferLength:(NSUInteger)vertexBufferLength
       indexBuffer:(const char[]) indexBuffer indexBufferLength:(NSUInteger) indexBufferLength;
@end