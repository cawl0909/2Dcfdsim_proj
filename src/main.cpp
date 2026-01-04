#include <iostream>
#include <vector>
#include <string>
#include <random>


#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "vectors.h"

void cleanup(SDL_Window* window, SDL_Renderer* renderer)
{
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    std::cout<<"Cleanup complete"<<std::endl;
}

int main(int argc, char *argv[])
{
    const int WINDOW_SIZE_X = 640;
    const int WINDOW_SIZE_Y = 320;
    const char WINDOW_NAME[] = "CFD SIM";

    const double TARGET_FPS = 60.0;
    const double TARGET_FRAME_TIME = 1000.0/60.0;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> distrib(1,255);

    if(!(SDL_Init(SDL_INIT_VIDEO)))
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,"INIT ERROR","SDL FAILED TO INIT",nullptr);
        std::cerr<<"SDL failed to init video"<<"\n";
        return -1;
    }

    SDL_Window *window = SDL_CreateWindow(WINDOW_NAME,WINDOW_SIZE_X,WINDOW_SIZE_Y,0);
    SDL_Renderer *renderer = SDL_CreateRenderer(window,nullptr);

    if(window == nullptr)
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,"INIT ERROR","SDL FAILED TO INIT WINDOW",nullptr);
        cleanup(window,renderer);
        return -1;
    }

    if(renderer == nullptr)
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,"INIT ERROR","SDL FAILED TO INIT RENDERER",nullptr);
        cleanup(window,renderer);
        return -1;
    }



    bool running = true;

    size_t start_tick;
    while (running) 
    {
        start_tick = SDL_GetTicks();
        SDL_Event e;
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_EVENT_QUIT)
            {
                running = false;
                break;
            }
        }
        SDL_SetRenderDrawColor(renderer,255,255,255,255);
        SDL_RenderClear(renderer);
        
        SDL_SetRenderDrawColor(renderer,255,0,0,SDL_ALPHA_OPAQUE);
        //SDL_RenderLine(renderer,0,0,WINDOW_SIZE_X,WINDOW_SIZE_Y);
        SDL_RenderPoint(renderer,0,0);
        for(size_t i = 0; i < WINDOW_SIZE_X; i++)
        {
            for(size_t j = 0; j < WINDOW_SIZE_Y; j++)
            {
                SDL_SetRenderDrawColor(renderer,distrib(gen),distrib(gen),distrib(gen),SDL_ALPHA_OPAQUE);
                SDL_RenderPoint(renderer,i,j);
            }
        }
        SDL_RenderPresent(renderer);

    }

    cleanup(window,renderer);
    return 0;
}