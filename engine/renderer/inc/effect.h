#import <Foundation/Foundation.h>
#import <Metal/Metal.h>

@interface Effect: NSObject {
}
@property (nonatomic, readonly) id <MTLFunction> vertexProgram;
@property (nonatomic, readonly) id <MTLFunction> fragmentProgram;

- (id)initWithLibrary: (id<MTLLibrary>)library vertexName: (NSString*)vertexName fragmentName: (NSString*)fragmentName;
@end