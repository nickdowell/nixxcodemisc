
#import <Cocoa/Cocoa.h>

@interface ComputerIconAppDelegate : NSObject <NSApplicationDelegate> {
    NSWindow *window;
	NSImageView *imageView;
}

@property (assign) IBOutlet NSWindow *window;
@property (assign) IBOutlet NSImageView *imageView;

@end
