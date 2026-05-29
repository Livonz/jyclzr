// AppDelegate.m - iOS app delegate

#import "AppDelegate.h"
#import "JYViewController.h"

@implementation AppDelegate

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
    self.window = [[UIWindow alloc] initWithFrame:[UIScreen mainScreen].bounds];
    
    JYViewController *viewController = [[JYViewController alloc] init];
    self.window.rootViewController = viewController;
    [self.window makeKeyAndVisible];
    
    return YES;
}

- (void)applicationWillResignActive:(UIApplication *)application {
    // Pause game
}

- (void)applicationDidEnterBackground:(UIApplication *)application {
    // Save game state
}

- (void)applicationWillEnterForeground:(UIApplication *)application {
    // Resume game
}

- (void)applicationDidBecomeActive:(UIApplication *)application {
    // Resume game
}

- (void)applicationWillTerminate:(UIApplication *)application {
    // Save and cleanup
}

@end
