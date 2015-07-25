#import <Foundation/Foundation.h>
#import <Metal/Metal.h>

@interface Actor : NSObject {
}
- (id)initWithConstantStruct:(void*) constantStruct numberOfBuffers:(NSUInteger) numberOfBuffers;
@end