#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <unistd.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include "def.h"
#include "screen.h"
#include "sdlutils.h"
#include "resourceManager.h"
#include "commander.h"

// Globals
SDL_Surface *ScreenSurface;

SDL_Window *Globals::g_sdlwindow=NULL;
SDL_Surface *Globals::g_screen = NULL;
const SDL_Color Globals::g_colorTextNormal = {COLOR_TEXT_NORMAL};
const SDL_Color Globals::g_colorTextTitle = {COLOR_TEXT_TITLE};
const SDL_Color Globals::g_colorTextDir = {COLOR_TEXT_DIR};
const SDL_Color Globals::g_colorTextSelected = {COLOR_TEXT_SELECTED};
std::vector<CWindow *> Globals::g_windows;

namespace {

SDL_Surface *SetVideoMode(int width, int height, int bpp, std::uint32_t flags) {
	// fprintf(stderr, "Setting video mode %dx%d bpp=%u flags=0x%08X\n", width, height, bpp, flags);
    // fflush(stderr);
	// auto *result = SDL_SetVideoMode(width, height, bpp, flags);
	// const auto &current = *SDL_GetVideoInfo();
	// fprintf(stderr, "Video mode is now %dx%d bpp=%u flags=0x%08X\n",
	//     current.current_w, current.current_h, current.vfmt->BitsPerPixel, SDL_GetVideoSurface()->flags);
	// fflush(stderr);
    // return result;
    return NULL;
}

} // namespace

int main(int argc, char** argv)
{
    // Avoid crash due to the absence of mouse
    {
        char l_s[]="SDL_NOMOUSE=1";
        putenv(l_s);
    }

    // Init SDL
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK);
    if (IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG | IMG_INIT_TIF | IMG_INIT_WEBP) == 0) {
        std::cerr << "IMG_Init failed" << std::endl;
    } else {
        // Clear the errors for image libraries that did not initialize.
        SDL_ClearError();
    }

    // Check for joystick
    if (SDL_NumJoysticks() > 0) {
        // Open joystick
        SDL_Joystick *joy = SDL_JoystickOpen(0);

        if (joy) {
            printf("Opened Joystick 0\n");
            printf("Name: %s\n", SDL_JoystickNameForIndex(0));
            printf("Number of Axes: %d\n", SDL_JoystickNumAxes(joy));
            printf("Number of Buttons: %d\n", SDL_JoystickNumButtons(joy));
            printf("Number of Balls: %d\n", SDL_JoystickNumBalls(joy));
        } else {
            printf("Couldn't open Joystick 0\n");
        }
       
    }

    // Hide cursor before creating the output surface.
    SDL_ShowCursor(SDL_DISABLE);

    // Screen
	//const auto &best = *SDL_GetVideoInfo();
	//fprintf(stderr, "Best video mode reported as: %dx%d bpp=%d hw_available=%u\n",
	    //best.current_w, best.current_h, best.vfmt->BitsPerPixel, best.hw_available);
    SDL_Rect best{0,0,SCREEN_WIDTH,SCREEN_HEIGHT};
    
    int display_count = SDL_GetNumVideoDisplays();

    SDL_Log("Number of displays: %i", display_count);

    for (int display_index = 0; display_index <= display_count; display_index++)
    {
        SDL_Log("Display %i:", display_index);

        int modes_count = SDL_GetNumDisplayModes(display_index);

        for (int mode_index = 0; mode_index <= modes_count; mode_index++)
        {
            SDL_DisplayMode mode = {SDL_PIXELFORMAT_UNKNOWN, 0, 0, 0, 0};

            if (SDL_GetDisplayMode(display_index, mode_index, &mode) == 0)
            {
                SDL_Log(" %i bpp\t%i x %i @ %iHz",
                        SDL_BITSPERPIXEL(mode.format), mode.w, mode.h, mode.refresh_rate);

                if(display_index==0){
                    best.w = mode.w;
                    best.h = mode.h;
                }
            }
        }
    }

//     // Detect non 320x240/480 screens.
#if AUTOSCALE == 1
// 
    if (best.w >= SCREEN_WIDTH * 2)
    {
        // E.g. 640x480. Upscale to the smaller of the two.
        double scale = std::min(
            best.w / static_cast<double>(SCREEN_WIDTH),
            best.h / static_cast<double>(SCREEN_HEIGHT));
        scale = std::min(scale, 2.0);
        screen.ppu_x = screen.ppu_y = scale;
        screen.w = best.w / scale;
        screen.h = best.h / scale;
        screen.actual_w = best.w;
        screen.actual_h = best.h;
    }
    else if (best.w != SCREEN_WIDTH)
    {
        // E.g. RS07 with 480x272 screen.
        screen.actual_w = screen.w = best.w;
        screen.actual_h = screen.h = best.h;
        screen.ppu_x = screen.ppu_y = 1;
    }
#endif
    Globals::g_sdlwindow = SDL_CreateWindow("Commander",  
                              SDL_WINDOWPOS_UNDEFINED,  
                              SDL_WINDOWPOS_UNDEFINED,  
                              screen.actual_w,screen.actual_h,  
                              SDL_WINDOW_OPENGL);  
    ScreenSurface = SDL_GetWindowSurface(Globals::g_sdlwindow);
    // ScreenSurface = SetVideoMode(screen.actual_w, screen.actual_h, SCREEN_BPP, SURFACE_FLAGS);

    Globals::g_screen = ScreenSurface;
    if (Globals::g_screen == NULL)
    {
        std::cerr << "SDL_SetVideoMode failed: " << SDL_GetError() << std::endl;
        return 1;
    }

    // Init font
    if (TTF_Init() == -1)
    {
        std::cerr << "TTF_Init failed: " << SDL_GetError() << std::endl;
        return 1;
    }


    // Create instances
    CResourceManager::instance();

    std::string l_path = PATH_DEFAULT;
    std::string r_path = PATH_DEFAULT_RIGHT;
    if (access(l_path.c_str(), F_OK) != 0) l_path = "/";
#ifdef PATH_DEFAULT_RIGHT_FALLBACK
    if (l_path == r_path || access(r_path.c_str(), F_OK) != 0) r_path = PATH_DEFAULT_RIGHT_FALLBACK;
#endif
    if (access(r_path.c_str(), F_OK) != 0) r_path = "/";

    CCommander l_commander(l_path, r_path);

    // Main loop
    l_commander.execute();

    //Quit
    SDL_utils::hastalavista();

    return 0;
}
