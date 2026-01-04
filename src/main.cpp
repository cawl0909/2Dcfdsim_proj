#include <iostream>
#include <vector>
#include <string>

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "vectors.h"

const int WINDOW_SIZE_X = 320;
const int WINDOW_SIZE_Y = 240;
const char WINDOW_NAME[] = "CFD SIM";

int main(int argc, char *argv[])
{
    if(!(SDL_Init(SDL_INIT_VIDEO)))
    {
        SDL_Log("Failed to start");
        std::cerr<<"SDL failed to init video"<<"\n";
        return -1;
    }

    SDL_Window *window = SDL_CreateWindow("SDL3",800, 600,0);

    bool running = true;
    while (running) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT)
                running = false;
        }
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}