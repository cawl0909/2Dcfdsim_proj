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
    // Simulation Paramters
    const size_t GRID_SIZE_X = 20;
    const size_t GRID_SIZE_Y = 20;
    const double TIME_STEP = 0.1;

    // MAC grid initialised with 0s
    
    std::vector<std::vector<double>> u_grid(GRID_SIZE_Y,std::vector<double>(GRID_SIZE_X+1,0));
    std::vector<std::vector<double>> v_grid(GRID_SIZE_Y+1,std::vector<double>(GRID_SIZE_X,0));
    std::vector<std::vector<double>> p_grid(GRID_SIZE_Y,std::vector<double>(GRID_SIZE_X,0));
    
    // GUI Paramters
    const size_t WINDOW_SIZE_X = 640;
    const size_t WINDOW_SIZE_Y = 640;
    const float PIXEL_SCALE = 32.0;

    const char WINDOW_NAME[] = "CFD SIM";

    const size_t TARGET_FPS = 60;
    const size_t TARGET_FRAME_TIME = 1000/TARGET_FPS;

    // Random 
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> distrib(1,255);

    // ----

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


    // Main Event Loop

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
        for(size_t i = 0; i < WINDOW_SIZE_X/PIXEL_SCALE; i++)
        {
            for(size_t j = 0; j < WINDOW_SIZE_Y/PIXEL_SCALE; j++)
            {
                SDL_SetRenderDrawColor(renderer,distrib(gen),distrib(gen),distrib(gen),SDL_ALPHA_OPAQUE);
                SDL_FRect temp_rect = {i*PIXEL_SCALE,j*PIXEL_SCALE,PIXEL_SCALE,PIXEL_SCALE};
                SDL_RenderFillRect(renderer,&temp_rect);
                //SDL_RenderPoint(renderer,i,j);
            }
        }
        SDL_RenderPresent(renderer);
        size_t current_tick = SDL_GetTicks();
        size_t tick_delta = current_tick-start_tick;
        if(tick_delta < TARGET_FRAME_TIME)
        {
            SDL_Delay(TARGET_FRAME_TIME-tick_delta);
        }

    }

    cleanup(window,renderer);
    return 0;
}