// JYViewController.h - Game view controller

#import <UIKit/UIKit.h>
#import <SDL2/SDL.h>

@interface JYViewController : UIViewController

@property (nonatomic, strong) SDL_Window *sdlWindow;
@property (nonatomic, strong) SDL_Renderer *sdlRenderer;

- (void)startGame;

@end
