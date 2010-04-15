
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
	NSAssert((*iconFamily)->resourceType == NSSwapHostIntToBig('icns'), @"");
	
	oserr = ReleaseIconRef(iconRef);
	NSAssert(oserr == noErr, @"ReleaseIconRef() failed");
	iconRef = NULL;
	
	SInt32 resourceSizeBigEndian = iconFamily[0]->resourceSize;
	NSUInteger length = NSSwapBigIntToHost(resourceSizeBigEndian);
	iconData = [NSData dataWithBytes:*iconFamily length:length];
	NSAssert(iconData != nil, @"[NSData dataWithBytes:] failed");
	
	iconImage = [[NSImage alloc] initWithData:iconData];
	NSAssert(iconImage != nil, @"[NSImage initWithData:] failed");
	
	[imageView setImage:iconImage];
}

@end
