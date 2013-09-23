#import <Foundation/Foundation.h>
#import "GHTestMacros.h"

@interface SimpleTestCase : NSObject {}

- (void)failWithException:(NSException *)exception;

@end
