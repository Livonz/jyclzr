// JYViewController.m - Game view controller with SDL integration

#import "JYViewController.h"
#import <SDL2/SDL.h>

// External main function from jymain.c
extern int jy_main(int argc, char *argv[]);

@implementation JYViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    self.view.backgroundColor = [UIColor blackColor];
    
    // Disable idle timer for gaming
    [UIApplication sharedApplication].idleTimerDisabled = YES;
    
    // Set up layout for landscape
    [self setNeedsUpdateOfSupportedInterfaceOrientations];
}

- (BOOL)prefersStatusBarHidden {
    return YES;
}

- (UIInterfaceOrientationMask)supportedInterfaceOrientations {
    return UIInterfaceOrientationMaskLandscape;
}

- (BOOL)prefersHomeIndicatorAutoHidden {
    return YES;
}

- (void)viewDidAppear:(BOOL)animated {
    [super viewDidAppear:animated];
    [self startGame];
}

- (void)startGame {
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        // Copy game resources from bundle to documents directory if needed
        [self copyResourcesIfNeeded];
        
        // Change to game directory
        NSString *gamePath = [self getGamePath];
        chdir([gamePath UTF8String]);
        
        // Start the game engine
        char *argv[] = {"jyengine", NULL};
        jy_main(1, argv);
    });
}

- (NSString *)getGamePath {
    // Get the documents directory
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString *documentsPath = [paths firstObject];
    NSString *gamePath = [documentsPath stringByAppendingPathComponent:@"game"];
    
    // Create game directory if it doesn't exist
    NSFileManager *fileManager = [NSFileManager defaultManager];
    if (![fileManager fileExistsAtPath:gamePath]) {
        [fileManager createDirectoryAtPath:gamePath 
              withIntermediateDirectories:YES 
                               attributes:nil 
                                    error:nil];
    }
    
    return gamePath;
}

- (void)copyResourcesIfNeeded {
    NSString *gamePath = [self getGamePath];
    NSFileManager *fileManager = [NSFileManager defaultManager];
    
    // List of resource directories to copy
    NSArray *resourceDirs = @[@"data", @"script", @"pic", @"sound"];
    
    for (NSString *dir in resourceDirs) {
        NSString *targetPath = [gamePath stringByAppendingPathComponent:dir];
        if (![fileManager fileExistsAtPath:targetPath]) {
            NSString *bundlePath = [[NSBundle mainBundle] pathForResource:dir ofType:nil];
            if (bundlePath) {
                NSError *error;
                [fileManager copyItemAtPath:bundlePath toPath:targetPath error:&error];
                if (error) {
                    NSLog(@"Error copying %@: %@", dir, error.localizedDescription);
                }
            }
        }
    }
    
    // Copy individual files
    NSArray *resourceFiles = @[@"config.lua", @"hzmb.dat"];
    for (NSString *file in resourceFiles) {
        NSString *targetPath = [gamePath stringByAppendingPathComponent:file];
        if (![fileManager fileExistsAtPath:targetPath]) {
            NSString *bundlePath = [[NSBundle mainBundle] pathForResource:[file stringByDeletingPathExtension] 
                                                                  ofType:[file pathExtension]];
            if (bundlePath) {
                NSError *error;
                [fileManager copyItemAtPath:bundlePath toPath:targetPath error:&error];
                if (error) {
                    NSLog(@"Error copying %@: %@", file, error.localizedDescription);
                }
            }
        }
    }
}

- (void)dealloc {
    [UIApplication sharedApplication].idleTimerDisabled = NO;
}

@end
