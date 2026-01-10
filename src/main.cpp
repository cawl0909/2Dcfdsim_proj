#include <iostream>
#include <vector>
#include <string>
#include <random>
#include <memory>
#include <algorithm>

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
    const size_t GRID_SIZE_X = 150;
    const size_t GRID_SIZE_Y = 100;
    const double CELL_LENGTH = 1.0;
    constexpr double TIME_STEP = 1.0/60.0;
    const double OVER_RELAXATION = 1.7;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> distrib(1,255);

    // Fluid Class initialising

    std::unique_ptr<Fluid> fluidobj = std::make_unique<Fluid>(1000.0,GRID_SIZE_X,GRID_SIZE_Y,CELL_LENGTH,OVER_RELAXATION);
    //fluidobj->randomise_velocities(gen);


    // Setting up fluid sim bc and setup
    double inlet_velocity = 20.0;
    for(int i =0; i<fluidobj->numX;i++)
    {
        for(int j = 0; j<fluidobj->numY;j++)
        {
            double tS = 1.0;
            if(i == 0 || j == 0 || j == fluidobj->numY-1)
            {
                tS = 0.0;
            }
            fluidobj->solid[i][j] = tS;

            if(i == 1)
            {
                fluidobj->u_grid[i][j] = inlet_velocity;
            }
        }
    }
    double inlet_width = 10.0;
    double fluid_height = (fluidobj->numY)*CELL_LENGTH;
    double mid_point  = fluid_height*0.5;
    int bot_j =  std::floor((mid_point - inlet_width*0.5)/CELL_LENGTH);
    int top_j =  std::floor((mid_point + 0.5*inlet_width)/CELL_LENGTH);
    std::cout<<bot_j<<","<<top_j<<std::endl;
    for(int j = bot_j; j<top_j;j++)
    {
        fluidobj->mass[0][j] = 0.0;
    }

    fluidobj->set_circle_obstacle(35,mid_point,10);

    // GUI Paramters
    const float PIXEL_SCALE = 8.0;
    const size_t WINDOW_SIZE_X = (GRID_SIZE_X)*PIXEL_SCALE;
    const size_t WINDOW_SIZE_Y = (GRID_SIZE_Y)*PIXEL_SCALE;

    const char WINDOW_NAME[] = "CFD SIM";

    const size_t TARGET_FPS = 60;
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
    int frame_count = 0;

    size_t start_tick;
    while (running) 
    {
        start_tick = SDL_GetTicks();
        frame_count +=1;
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
                std::cout<<"Key pressed"<<SDL_GetKeyName(e.key.key)<<" Frame: "<<frame_count<<std::endl;
                break;
            }
            
        }
        SDL_SetRenderDrawColor(renderer,255,255,255,255);
        SDL_RenderClear(renderer);
        
        //SDL_SetRenderDrawColor(renderer,255,0,0,SDL_ALPHA_OPAQUE);
        //SDL_RenderLine(renderer,0,0,WINDOW_SIZE_X,WINDOW_SIZE_Y);
        //SDL_RenderPoint(renderer,0,0);

        fluidobj->simulate(TIME_STEP,0.0,40);
        //std::cout<<"Frame count: "<<frame_count<<std::endl;

        for(size_t i = 1; i < GRID_SIZE_X+2-1; i++)
        {
            for(size_t j = 1; j < GRID_SIZE_Y+2-1; j++)
            {
                int win_x = i-1;
                int win_y  = j-1;
                int ty = abs(j-(GRID_SIZE_Y+2-1)); //transforminf from the top left reference frame of the SDL renderer to the cartesian system in sim
                int smoke_per = 255*((fluidobj->mass[i][j])/1.0);
                int smoke_c = std::clamp(smoke_per,0,255);
                SDL_SetRenderDrawColor(renderer,smoke_c,smoke_c,smoke_c,255);
                SDL_FRect temp_rect = {win_x*PIXEL_SCALE,win_y*PIXEL_SCALE,PIXEL_SCALE,PIXEL_SCALE};
                SDL_RenderFillRect(renderer,&temp_rect);

                SDL_Color solid_map = {0,0,0,255};
                if(fluidobj->solid[i][ty] == 0.0)
                {
                    solid_map = SDL_Color{255,0,0,255};
                    SDL_SetRenderDrawColor(renderer,solid_map.r,solid_map.g,solid_map.b,solid_map.a);
                    SDL_FRect temp_rect2 = {win_x*PIXEL_SCALE,win_y*PIXEL_SCALE,PIXEL_SCALE,PIXEL_SCALE};
                    SDL_RenderFillRect(renderer,&temp_rect2);
                }
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