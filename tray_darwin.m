//go:build darwin
// +build darwin

#import <Cocoa/Cocoa.h>
#import <dispatch/dispatch.h>
#import <objc/runtime.h>
#import "tray_darwin.h"

void goOnLaunch(void);
void goOnQuit(void);

@interface TrayDelegate : NSObject <NSApplicationDelegate>
- (instancetype)initWithTooltip:(const char *)tooltip
                       iconPath:(const char *)iconPath;
@end

@implementation TrayDelegate {
    NSStatusItem *_item;
}

- (instancetype)initWithTooltip:(const char *)tooltip
                       iconPath:(const char *)iconPath
{
    self = [super init];
    if (!self) return nil;

    _item = [[NSStatusBar systemStatusBar]
             statusItemWithLength:NSVariableStatusItemLength];

    if (tooltip) {
        _item.button.toolTip = [NSString stringWithUTF8String:tooltip];
    }

    if (iconPath) {
        NSImage *img = [[NSImage alloc] initWithContentsOfFile:
                        [NSString stringWithUTF8String:iconPath]];
        img.template = YES;
        img.resizingMode = NSImageResizingModeStretch;
        _item.button.image = img;
    }

    // "Quit [tooltip]" menu
    NSMenu *menu = [[NSMenu alloc] init];
    NSString *title = @"Quit";
    if (_item.button.toolTip.length)
        title = [title stringByAppendingFormat:@" %@", _item.button.toolTip];
    [menu addItem:[[NSMenuItem alloc] initWithTitle:title
                                             action:@selector(terminate:)
                                      keyEquivalent:@""]];
    _item.menu = menu;
    return self;
}

- (void)applicationDidFinishLaunching:(NSNotification *)n { goOnLaunch(); }
- (void)applicationWillTerminate:(NSNotification *)n {
    _item.menu = nil;
    _item.button.image = nil;
    goOnQuit();
}

@end  // TrayDelegate


// C API exposed to Go

void runTray(const char *tooltip, const char *iconPath)
{
    @autoreleasepool {
        NSApplication *app = [NSApplication sharedApplication];
        app.activationPolicy = NSApplicationActivationPolicyAccessory;

        TrayDelegate *d = [[TrayDelegate alloc] initWithTooltip:tooltip
                                                       iconPath:iconPath];
        objc_setAssociatedObject(app, @selector(delegate), d, OBJC_ASSOCIATION_RETAIN);
        app.delegate = d;
        [app activateIgnoringOtherApps:YES];

        [app run];
    }
}
