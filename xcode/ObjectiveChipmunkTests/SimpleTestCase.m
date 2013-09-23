#import "SimpleTestCase.h"

@implementation SimpleTestCase

// Need to implement this method in order to use the GH-unit macros
- (void)failWithException:(NSException *)exception;
{
  [exception raise];
}

@end
