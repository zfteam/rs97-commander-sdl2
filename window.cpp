#include <iostream>
#include "window.h"
#include "def.h"
#include "sdlutils.h"
#include <string> 
#include <map>

#define KEYHOLD_TIMER_FIRST   6
#define KEYHOLD_TIMER         2

extern SDL_Surface *ScreenSurface;

CWindow::CWindow(void):
    m_timer(0),
    m_lastPressed(SDLK_0),
    m_retVal(0)
{
    // Add window to the lists for render
    Globals::g_windows.push_back(this);
}

CWindow::~CWindow(void)
{
    // Remove last window
    Globals::g_windows.pop_back();
}

const int CWindow::execute(void)
{
    m_retVal = 0;
    Uint32 l_time(0);
    SDL_Event l_event;
    bool l_loop(true);
    bool l_render(true);
    // Main loop
    while (l_loop)
    {
        l_time = SDL_GetTicks();
        // Handle key press
        while (SDL_PollEvent(&l_event))
        {
            if (l_event.type == SDL_KEYDOWN)
            {
                l_render = this->keyPress(l_event);
                if (m_retVal)
                    l_loop = false;
            }
            else if (l_event.type == SDL_QUIT) 
            {
                return m_retVal;
            }
            else if(l_event.type == SDL_JOYBUTTONDOWN)
            {
#ifdef ODROID_GO_ADVANCE
                // printf("key:%d\n",l_event.jbutton.button);
                std::map<std::string,int> keyMap;
                keyMap[Globals::g_iniConfig["key"]["up"]]= MYKEY_UP;
                keyMap[Globals::g_iniConfig["key"]["down"]]= MYKEY_DOWN;
                keyMap[Globals::g_iniConfig["key"]["left"]]= MYKEY_LEFT;
                keyMap[Globals::g_iniConfig["key"]["right"]]= MYKEY_RIGHT;
                keyMap[Globals::g_iniConfig["key"]["a"]]= MYKEY_PARENT;
                keyMap[Globals::g_iniConfig["key"]["b"]]= MYKEY_OPEN;
                keyMap[Globals::g_iniConfig["key"]["x"]]= MYKEY_OPERATION;
                keyMap[Globals::g_iniConfig["key"]["y"]]= MYKEY_SYSTEM;
                keyMap[Globals::g_iniConfig["key"]["l"]]= MYKEY_PAGEUP;
                keyMap[Globals::g_iniConfig["key"]["r"]]= MYKEY_PAGEDOWN;
                keyMap[Globals::g_iniConfig["key"]["select"]]= MYKEY_SELECT;
                keyMap[Globals::g_iniConfig["key"]["start"]]= MYKEY_TRANSFER;

                SDL_Event key_event;
                std::string actionKey= std::to_string(l_event.jbutton.button);
                std::map<std::string, int>::iterator iter = keyMap.find(actionKey);
                if (iter != keyMap.end()){
                    key_event.key.keysym.sym = iter->second;
                }

                l_render = this->keyPress(key_event);
                if (m_retVal)
                    l_loop = false;
#endif
            }
        }
        // Handle key hold
        if (l_loop)
            l_render = this->keyHold() || l_render;
        // Render if necessary
        if (l_render && l_loop)
        {
            SDL_utils::renderAll();
            // Flip twice to avoid graphical glitch on Dingoo
            //SDL_Flip(Globals::g_screen);
            //SDL_Flip(Globals::g_screen);
            // SDL_Flip(ScreenSurface);
            SDL_UpdateWindowSurface(Globals::g_sdlwindow);
            SDL_UpdateWindowSurface(Globals::g_sdlwindow);

            l_render = false;
            INHIBIT(std::cout << "Render time: " << SDL_GetTicks() - l_time << "ms"<< std::endl;)
        }
        // Cap the framerate
        l_time = MS_PER_FRAME - (SDL_GetTicks() - l_time);
        if (l_time <= MS_PER_FRAME) SDL_Delay(l_time);
    }
    return m_retVal;
}

const bool CWindow::keyPress(const SDL_Event &p_event)
{
    // Reset timer if running
    if (m_timer)
        m_timer = 0;
    m_lastPressed = p_event.key.keysym.sym;
    return false;
}

const bool CWindow::keyHold(void)
{
    // Default behavior
    return false;
}

const bool CWindow::tick(const Uint8 p_held)
{
    bool l_ret(false);
    if (p_held)
    {
        if (m_timer)
        {
            --m_timer;
            if (!m_timer)
            {
                // Trigger!
                l_ret = true;
                // Timer continues
                m_timer = KEYHOLD_TIMER;
            }
        }
        else
        {
            // Start timer
            m_timer = KEYHOLD_TIMER_FIRST;
        }
    }
    else
    {
        // Stop timer if running
        if (m_timer)
            m_timer = 0;
    }
    return l_ret;
}

const int CWindow::getReturnValue(void) const
{
    return m_retVal;
}

bool CWindow::isFullScreen(void) const
{
    // Default behavior
    return false;
}
