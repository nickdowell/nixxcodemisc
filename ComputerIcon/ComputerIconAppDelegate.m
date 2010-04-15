
#import "ComputerIconAppDelegate.h"

@implementation ComputerIconAppDelegate

@synthesize window;
@synthesize imageView;

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
	// Insert code here to initialize your application 
	
	OSStatus			oserr = noErr;
	IconRef				iconRef = NULL;
	IconFamilyHandle	iconFamily = NULL;
	NSData *			iconData = nil;
	NSImage *			iconImage = nil;
	
	oserr = GetIconRef(kOnSystemDisk, kSystemIconsCreator, kComputerIcon, &iconRef);
	NSAssert(oserr == noErr, @"GetIconRef() failed");
	
	oserr = IconRefToIconFamily(iconRef, kSelectorAllAvailableData, &iconFamily);
	NSAssert(oserr == noErr, @"IconRefToIconFamily() failed");
	NSAssert(NSSwapBigIntToHost(iconFamily[0]->resourceType) == 'icns', @"IconFamily resourceType mismatch");
	
	oserr = ReleaseIconRef(iconRef);
	NSAssert(oserr == noErr, @"ReleaseIconRef() failed");
	iconRef = NULL;
	
	NSUInteger length = NSSwapBigIntToHost(iconFamily[0]->resourceSize);
	iconData = [NSData dataWithBytes:*iconFamily length:length];
	NSAssert(iconData != nil, @"[NSData dataWithBytes:] failed");
	
	iconImage = [[NSImage alloc] initWithData:iconData];
	NSAssert(iconImage != nil, @"[NSImage initWithData:] failed");
	
	[imageView setImage:iconImage];
}

@end
