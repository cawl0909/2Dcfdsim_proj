#include <iostream>
#include <vector>
#include <string>
#include <random>
#include <memory>

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "vectors.h"
#include "Fluid.h"

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
    const double CELL_LENGTH = 1.0;
    constexpr double TIME_STEP = 1.0/60;
    const double OVER_RELAXATION = 1.7;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> distrib(1,255);

    // Fluid Class initialising

    std::unique_ptr<Fluid> fluidobj = std::make_unique<Fluid>(1.0,GRID_SIZE_X,GRID_SIZE_Y,CELL_LENGTH,OVER_RELAXATION);
    std::cout<<"Test"<<std::endl;
    fluidobj->integrate(TIME_STEP,9.81);

    // GUI Paramters
    const float PIXEL_SCALE = 32.0;
    const size_t WINDOW_SIZE_X = (GRID_SIZE_X)*PIXEL_SCALE;
    const size_t WINDOW_SIZE_Y = (GRID_SIZE_Y)*PIXEL_SCALE;

    const char WINDOW_NAME[] = "CFD SIM";

    const size_t TARGET_FPS = 6;
    const size_t TARGET_FRAME_TIME = 1000/TARGET_FPS;

    // Random 
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
            else if (e.type == SDL_EVENT_KEY_DOWN)
            {
                std::cout<<"Key pressed"<<SDL_GetKeyName(e.key.key)<<std::endl;
                break;
            }
            
        }
        SDL_SetRenderDrawColor(renderer,255,255,255,255);
        SDL_RenderClear(renderer);
        
        SDL_SetRenderDrawColor(renderer,255,0,0,SDL_ALPHA_OPAQUE);
        //SDL_RenderLine(renderer,0,0,WINDOW_SIZE_X,WINDOW_SIZE_Y);
        SDL_RenderPoint(renderer,0,0);
        for(size_t i = 0; i < GRID_SIZE_X; i++)
        {
            for(size_t j = 0; j < GRID_SIZE_Y; j++)
            {
                int scale = distrib(gen);
                SDL_SetRenderDrawColor(renderer,scale,0,0,255);
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