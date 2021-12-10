#include "file_utilities_mac.h"
#include <Cocoa/Cocoa.h>
#include <CoreFoundation/CFURL.h>

namespace {
double _macAccessoryWidth = 450;
double _macAccessoryHeight = 90;
int _macAppHintTop = 8;
int _macAlwaysThisAppTop = 4;
int _macEnableFilterAdd = 2;
int _macEnableFilterTop = 5;
int _macSelectorTop = 6;
int _macCautionIconSize = 16;

inline NSString *Q2NSString(const std::string &str) {
    return [NSString stringWithUTF8String:str.c_str()];
}

template <int Size>
inline std::string MakeFromLetters(const uint32 (&letters)[Size]) {
    std::string result;
    result.resize(Size);
    for (uint32 i = 0; i < Size; ++i) {
        auto code = letters[i];
        auto salt1 = (code >> 8) & 0xFFU;
        auto salt2 = (code >> 24) & 0xFFU;
        auto part1 = ((code & 0xFFU) ^ (salt1 ^ salt2)) & 0xFFU;
        auto part2 = (((code >> 16) & 0xFFU) ^ (salt1 ^ ~salt2)) & 0xFFU;
        result.push_back((char)((part2 << 8) | part1));
    }
    return result;
}

std::string strNeedToReload() {
    const uint32 letters[] = { 0xAD92C02B, 0xA2217C97, 0x5E55F4F5, 0x2207DAAC, 0xD18BA536, 0x03E41869, 0xB96D2BFD, 0x810C7284, 0xE412099E, 0x5AAD0837, 0xE6637AEE, 0x8E5E2FF5, 0xE3BDA123, 0x94A5CE38, 0x4A42F7D1, 0xCE4677DC, 0x40A81701, 0x9C5B38CD, 0x61801E1A, 0x6FF16179 };
    return MakeFromLetters(letters);
}

std::string strNeedToRefresh1() {
    const uint32 letters[] = { 0xEDDFCD66, 0x434DF1FB, 0x820B76AB, 0x48CE7965, 0x3609C0BA, 0xFC9A990C, 0x3EDD1C51, 0xE2BDA036, 0x7140CEE9, 0x65DB414D, 0x88592EC3, 0x2CB2613A };
    return MakeFromLetters(letters);
}

std::string strNeedToRefresh2() {
    const uint32 letters[] = { 0x8AE4915D, 0x7159D7EF, 0x79C74167, 0x29B7611C, 0x0E6B9ADD, 0x0D93610F, 0xEBEAFE7A, 0x5BD17540, 0x121EF3B7, 0x61B02E26, 0x2174AAEE, 0x61AD3325 };
    return MakeFromLetters(letters);
}
}

namespace st {
const double &macAccessoryWidth(_macAccessoryWidth);
const double &macAccessoryHeight(_macAccessoryHeight);
const int &macAppHintTop(_macAppHintTop);
const int &macAlwaysThisAppTop(_macAlwaysThisAppTop);
const int &macEnableFilterAdd(_macEnableFilterAdd);
const int &macEnableFilterTop(_macEnableFilterTop);
const int &macSelectorTop(_macSelectorTop);
const int &macCautionIconSize(_macCautionIconSize);
}

@interface ChooseApplicationDelegate : NSObject<NSOpenSavePanelDelegate> {
}

- (id) init:(NSArray *)recommendedApps withPanel:(NSOpenPanel *)creator withSelector:(NSPopUpButton *)menu withGood:(NSTextField *)goodLabel withBad:(NSTextField *)badLabel withIcon:(NSImageView *)badIcon withAccessory:(NSView *)acc;
- (BOOL) panel:(id)sender shouldEnableURL:(NSURL *)url;
- (void) panelSelectionDidChange:(id)sender;
- (void) menuDidClose;
- (void) dealloc;

@end // @interface ChooseApplicationDelegate

@implementation ChooseApplicationDelegate {
    BOOL onlyRecommended;
    NSArray *apps;
    NSOpenPanel *panel;
    NSPopUpButton *selector;
    NSTextField *good, *bad;
    NSImageView *icon;
    NSString *recom;
    NSView *accessory;
    
}

- (id) init:(NSArray *)recommendedApps withPanel:(NSOpenPanel *)creator withSelector:(NSPopUpButton *)menu withGood:(NSTextField *)goodLabel withBad:(NSTextField *)badLabel withIcon:(NSImageView *)badIcon withAccessory:(NSView *)acc {
    if (self = [super init]) {
        onlyRecommended = NO;
        recom = @"Recommended Applications";
        apps = recommendedApps;
        panel = creator;
        selector = menu;
        good = goodLabel;
        bad = badLabel;
        icon = badIcon;
        accessory = acc;
        [selector setAction:@selector(menuDidClose)];
    }
    return self;
}

- (BOOL) isRecommended:(NSURL *)url {
    if (apps) {
        for (id app in apps) {
            if ([(NSURL*)app isEqual:url]) {
                return YES;
            }
        }
    }
    return NO;
}

- (BOOL) panel:(id)sender shouldEnableURL:(NSURL *)url {
    NSNumber *isDirectory;
    if ([url getResourceValue:&isDirectory forKey:NSURLIsDirectoryKey error:nil] && isDirectory != nil && [isDirectory boolValue]) {
        if (onlyRecommended) {
            CFStringRef ext = CFURLCopyPathExtension((CFURLRef)url);
            NSNumber *isPackage;
            if ([url getResourceValue:&isPackage forKey:NSURLIsPackageKey error:nil] && isPackage != nil && [isPackage boolValue]) {
                return [self isRecommended:url];
            }
        }
        return YES;
    }
    return NO;
}

- (void) panelSelectionDidChange:(id)sender {
    NSArray *urls = [panel URLs];
    if ([urls count]) {
        if ([self isRecommended:[urls firstObject]]) {
            [bad removeFromSuperview];
            [icon removeFromSuperview];
            [accessory addSubview:good];
        } else {
            [good removeFromSuperview];
            [accessory addSubview:bad];
            [accessory addSubview:icon];
        }
    } else {
        [good removeFromSuperview];
        [bad removeFromSuperview];
        [icon removeFromSuperview];
    }
}

- (void) menuDidClose {
    onlyRecommended = [[[selector selectedItem] title] isEqualToString:recom];
    [self refreshPanelTable];
}

- (BOOL) refreshDataInViews: (NSArray*)subviews {
    for (id view in subviews) {
        NSString *cls = [view className];
        if ([cls isEqualToString:Q2NSString(strNeedToReload())]) {
            [view reloadData];
        } else if ([cls isEqualToString:Q2NSString(strNeedToRefresh1())] || [cls isEqualToString:Q2NSString(strNeedToRefresh2())]) {
            [view reloadData];
            return YES;
        } else {
            NSArray *next = [view subviews];
            if ([next count] && [self refreshDataInViews:next]) {
                return YES;
            }
        }
    }
    return NO;
}


- (void) refreshPanelTable {
    @autoreleasepool {
        [self refreshDataInViews:[[panel contentView] subviews]];
        [panel validateVisibleColumns];
        
    }
}

- (void) dealloc {
    if (apps) {
        [apps release];
        [recom release];
    }
    [super dealloc];
}

@end // @implementation ChooseApplicationDelegate

namespace Platform {
namespace File {

bool UnsafeShowOpenWith(const std::string &filepath) {
    @autoreleasepool {
        NSString *file = [NSString stringWithUTF8String:filepath.c_str()];
        @try {
            NSURL *url = [NSURL fileURLWithPath:file];
            NSString *ext = [url pathExtension];
            NSArray *names = [url pathComponents];
            NSString *name = [names count] ? [names lastObject] : @"";
            NSArray *apps = (NSArray*)LSCopyApplicationURLsForURL(CFURLRef(url), kLSRolesAll);
            NSOpenPanel *openPanel = [NSOpenPanel openPanel];
            
            NSRect fullRect = { { 0., 0. }, { st::macAccessoryWidth, st::macAccessoryHeight } };
            NSView *accessory = [[NSView alloc] initWithFrame:fullRect];
            
            [accessory setAutoresizesSubviews:YES];
            
            NSPopUpButton *selector = [[NSPopUpButton alloc] init];
            [accessory addSubview:selector];
            [selector addItemWithTitle:@"推荐的应用程序"];
            [selector addItemWithTitle:@"所有应用程序"];
            [selector sizeToFit];
            
            NSTextField *enableLabel = [[NSTextField alloc] init];
            [accessory addSubview:enableLabel];
            [enableLabel setStringValue:@"显示："];
            [enableLabel setFont:[selector font]];
            [enableLabel setBezeled:NO];
            [enableLabel setDrawsBackground:NO];
            [enableLabel setEditable:NO];
            [enableLabel setSelectable:NO];
            [enableLabel sizeToFit];
            
            NSRect selectorFrame = [selector frame], enableFrame = [enableLabel frame];
            enableFrame.size.width += st::macEnableFilterAdd;
            enableFrame.origin.x = (fullRect.size.width - selectorFrame.size.width - enableFrame.size.width) / 2.;
            selectorFrame.origin.x = (fullRect.size.width - selectorFrame.size.width + enableFrame.size.width) / 2.;
            enableFrame.origin.y = fullRect.size.height - selectorFrame.size.height - st::macEnableFilterTop + (selectorFrame.size.height - enableFrame.size.height) / 2.;
            selectorFrame.origin.y = fullRect.size.height - selectorFrame.size.height - st::macSelectorTop;
            [enableLabel setFrame:enableFrame];
            [enableLabel setAutoresizingMask:NSViewMinXMargin|NSViewMaxXMargin];
            [selector setFrame:selectorFrame];
            [selector setAutoresizingMask:NSViewMinXMargin|NSViewMaxXMargin];
            
            NSButton *button = [[NSButton alloc] init];
            [accessory addSubview:button];
            [button setButtonType:NSSwitchButton];
            [button setFont:[selector font]];
            [button setTitle:@"总是使用此应用程序打开"];
            [button sizeToFit];
            NSRect alwaysRect = [button frame];
            alwaysRect.origin.x = (fullRect.size.width - alwaysRect.size.width) / 2;
            alwaysRect.origin.y = selectorFrame.origin.y - alwaysRect.size.height - st::macAlwaysThisAppTop;
            [button setFrame:alwaysRect];
            [button setAutoresizingMask:NSViewMinXMargin|NSViewMaxXMargin];
#ifdef OS_MAC_STORE
            [button setHidden:YES];
#endif // OS_MAC_STORE
            NSTextField *goodLabel = [[NSTextField alloc] init];
			NSString* lng_mac_this_app_can_open = [NSString stringWithFormat:@"%@%@", @"此应用程序可以打开", file];
            [goodLabel setStringValue:lng_mac_this_app_can_open];
            [goodLabel setFont:[NSFont systemFontOfSize:[NSFont smallSystemFontSize]]];
            [goodLabel setBezeled:NO];
            [goodLabel setDrawsBackground:NO];
            [goodLabel setEditable:NO];
            [goodLabel setSelectable:NO];
            [goodLabel sizeToFit];
            NSRect goodFrame = [goodLabel frame];
            goodFrame.origin.x = (fullRect.size.width - goodFrame.size.width) / 2.;
            goodFrame.origin.y = alwaysRect.origin.y - goodFrame.size.height - st::macAppHintTop;
            [goodLabel setFrame:goodFrame];
            
            NSTextField *badLabel = [[NSTextField alloc] init];
			NSString* lng_mac_not_known_app = [NSString stringWithFormat:@"%@%@", @"没有能打开的应用", file];
            [badLabel setStringValue:lng_mac_not_known_app];
            [badLabel setFont:[goodLabel font]];
            [badLabel setBezeled:NO];
            [badLabel setDrawsBackground:NO];
            [badLabel setEditable:NO];
            [badLabel setSelectable:NO];
            [badLabel sizeToFit];
            NSImageView *badIcon = [[NSImageView alloc] init];
            NSImage *badImage = [NSImage imageNamed:NSImageNameCaution];
            [badIcon setImage:badImage];
            [badIcon setFrame:NSMakeRect(0, 0, st::macCautionIconSize, st::macCautionIconSize)];
            
            NSRect badFrame = [badLabel frame], badIconFrame = [badIcon frame];
            badFrame.origin.x = (fullRect.size.width - badFrame.size.width + badIconFrame.size.width) / 2.;
            badIconFrame.origin.x = (fullRect.size.width - badFrame.size.width - badIconFrame.size.width) / 2.;
            badFrame.origin.y = alwaysRect.origin.y - badFrame.size.height - st::macAppHintTop;
            badIconFrame.origin.y = badFrame.origin.y;
            [badLabel setFrame:badFrame];
            [badIcon setFrame:badIconFrame];
            
            [openPanel setAccessoryView:accessory];
            
            ChooseApplicationDelegate *delegate = [[ChooseApplicationDelegate alloc] init:apps withPanel:openPanel withSelector:selector withGood:goodLabel withBad:badLabel withIcon:badIcon withAccessory:accessory];
            [openPanel setDelegate:delegate];
            
            [openPanel setCanChooseDirectories:NO];
            [openPanel setCanChooseFiles:YES];
            [openPanel setAllowsMultipleSelection:NO];
            [openPanel setResolvesAliases:YES];
            [openPanel setTitle:@"选择应用"];
			NSString* lng_mac_choose_text = [NSString stringWithFormat:@"%@%@", @"选择一个应用程序来打开文件", file];
            [openPanel setMessage:lng_mac_choose_text];
            
            NSArray *appsPaths = [[NSFileManager defaultManager] URLsForDirectory:NSApplicationDirectory inDomains:NSLocalDomainMask];
            if ([appsPaths count]) [openPanel setDirectoryURL:[appsPaths firstObject]];
            
            [NSApp setActivationPolicy:NSApplicationActivationPolicyAccessory];
                        
            if ([openPanel runModal] == NSModalResponseOK) {
                if ([[openPanel URLs] count] > 0) {
                    NSURL *app = [[openPanel URLs] objectAtIndex:0];
                    NSString *path = [app path];
                    if ([button state] == NSOnState) {
                        NSArray *UTIs = (NSArray *)UTTypeCreateAllIdentifiersForTag(kUTTagClassFilenameExtension,
                                                                                    (CFStringRef)ext,
                                                                                    nil);
                        for (NSString *UTI in UTIs) {
                            OSStatus result = LSSetDefaultRoleHandlerForContentType((CFStringRef)UTI,
                                                                                    kLSRolesAll,
                                                                                    (CFStringRef)[[NSBundle bundleWithPath:path] bundleIdentifier]);
                            // DEBUG_LOG(("App Info: set default handler for '%1' UTI result: %2").arg(NS2QString(UTI)).arg(result));
                        }
                        
                        [UTIs release];
                    }
                    [[NSWorkspace sharedWorkspace] openFile:file withApplication:[app path]];
                }
                
                [selector release];
                [button release];
                [enableLabel release];
                [goodLabel release];
                [badLabel release];
                [badIcon release];
                [accessory release];
                [delegate release];
            };
        }
        @catch (NSException *exception) {
            [[NSWorkspace sharedWorkspace] openFile:file];
        }
        @finally {
        }
        
    }
    
    return YES;
}

void UnsafeLaunch(const std::string &filepath) {
    @autoreleasepool {
        
        NSString *file = [NSString stringWithUTF8String:filepath.c_str()];
        if ([[NSWorkspace sharedWorkspace] openFile:file] == NO) {
            UnsafeShowOpenWith(filepath);
        }
        
    }
}
void UnsafeOpenEmailLink(const std::string& filepath)
{
    // TODO
}

void UnsafeShowInFolder(const std::string& filepath)
{
    size_t found = filepath.find_last_of("/\\");
    std::string folder = path.substr(0, found);
    @autoreleasepool {
        [[NSWorkspace sharedWorkspace] selectFile:Q2NSString(filepath) inFileViewerRootedAtPath:Q2NSString(folder)];
    }
}

} // namespace File
} // namespace Platform
