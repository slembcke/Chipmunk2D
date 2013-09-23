#import <string.h>

#import <objc/runtime.h>
#import <Foundation/Foundation.h>

#import "SimpleTestCase.h"
#import "ObjectiveChipmunk.h"


static void
PrintException(NSException *e)
{
	NSLog(@"%@\n%@", e, [e userInfo]);
}

static void
RunTestsForClass(Class klass)
{
	NSLog(@"Running %@", klass);
	
	id obj = [[klass alloc] init];
	
	unsigned int count = 0;
	Method *methods = class_copyMethodList(klass, &count);
	
	for(int i=0; i<count; i++){
		Method m = methods[i];
		SEL sel = method_getName(m);
		
		const char *name = sel_getName(sel);
		if(strncmp(name, "test", 4) == 0){
			NSLog(@"\t%s", name);
			void (*imp)(id, SEL) = (void *)method_getImplementation(m);
			
			NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
			@try {
				imp(obj, sel);
			} @catch (NSException *e){
				PrintException(e);
			} @finally {
				[pool release];
			}
		}
	}
	
	[obj release];
	free(methods);
}

int main(void){
	[[[ChipmunkSpace alloc] init] release]; // force an early init
	
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	
//	@try {
//		Class klass = [[NSBundle mainBundle] classNamed:@"SpaceTest"];
//		NSObject *obj = [[klass alloc] init];
//		[obj performSelector:@selector(testSleepActivateOnImpact)];
//		NSLog(@"Test passed.");
//	} @catch (NSException *e){
//		PrintException(e);
//	} @finally {
//		[pool release];
//	}
//	return 0;
	
	int count = objc_getClassList(NULL, 0);
	Class classes[count];
	objc_getClassList(classes, count);
	
	for(int i=0; i<count; i++){
		Class klass = classes[i];
		if(class_getSuperclass(klass) == [SimpleTestCase class]){
			RunTestsForClass(klass);
		}
	}
	
	NSLog(@"Finished");
	[pool release];
	
	return 0;
}
