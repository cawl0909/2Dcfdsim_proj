#include <iostream>
#include <vector>
#include <string>
#include <random>
#include <memory>
#include <algorithm>
#include <chrono>
#include <thread>

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"

#include "vectors.h"
#include "Fluid.h"

void cleanup(SDL_Window* window, SDL_Renderer* renderer, SDL_Texture* texture)
{
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_DestroyTexture(texture);
    SDL_Quit();
    std::cout<<"Cleanup complete"<<std::endl;
}

int main(int argc, char *argv[])
{
    // Simulation Paramters
    const size_t GRID_SIZE_X = 150;
    const size_t GRID_SIZE_Y = 90;
    const double CELL_LENGTH = 0.1;
    constexpr double TIME_STEP = 1.0/60.0;
    const double OVER_RELAXATION = 1.9;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> distrib(1,255);

    // Fluid Class initialising

    std::unique_ptr<Fluid> fluidobj = std::make_unique<Fluid>(1000.0,GRID_SIZE_X,GRID_SIZE_Y,CELL_LENGTH,OVER_RELAXATION);
    //fluidobj->randomise_velocities(gen);

    // GUI Parameters
    const float PIXEL_SCALE = 6.0;
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
    SDL_SetWindowResizable(window, true);
    SDL_Renderer *renderer = SDL_CreateRenderer(window,nullptr);
    SDL_Texture* field_texture = SDL_CreateTexture(
    renderer,
    SDL_PIXELFORMAT_ARGB8888,
    SDL_TEXTUREACCESS_STREAMING,
    static_cast<int>(GRID_SIZE_X),
    static_cast<int>(GRID_SIZE_Y)
    );

    if(window == nullptr)
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,"INIT ERROR","SDL FAILED TO INIT WINDOW",nullptr);
        cleanup(window,renderer,field_texture);
        return -1;
    }

    if(renderer == nullptr)
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,"INIT ERROR","SDL FAILED TO INIT RENDERER",nullptr);
        cleanup(window,renderer,field_texture);
        return -1;
    }

    if(field_texture == nullptr)
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,"INIT ERROR","SDL FAILED TO INIT FIELD TEXTURE",nullptr);
        cleanup(window,renderer,field_texture);
        return -1;
    }

    SDL_SetTextureScaleMode(field_texture, SDL_SCALEMODE_NEAREST);
    SDL_SetTextureBlendMode(field_texture, SDL_BLENDMODE_NONE);

    std::vector<Uint32> field_pixels(GRID_SIZE_X * GRID_SIZE_Y, 0xFFFFFFFFu);

    // IMGUI setup
    
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); 
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;   
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    ImGui::StyleColorsDark();

    ImGuiStyle& style = ImGui::GetStyle();

    // --

    ImGui_ImplSDL3_InitForSDLRenderer(window,renderer);
    ImGui_ImplSDLRenderer3_Init(renderer);

    // Fluid setup
    // Setting up fluid sim bc and setup
    double inlet_velocity = 10.0;
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
    // Define inlet band as a fraction of the vertical cell count (independent of CELL_LENGTH)
    double inlet_fraction = 0.1; // 10% of the channel height
    double inlet_cells = inlet_fraction * static_cast<double>(fluidobj->numY);
    int bot_j = static_cast<int>(std::floor(0.5 * fluidobj->numY - 0.5 * inlet_cells));
    int top_j = static_cast<int>(std::floor(0.5 * fluidobj->numY + 0.5 * inlet_cells));

    // Clamp to valid index range to avoid out-of-bounds when CELL_LENGTH or grid size changes
    bot_j = std::max(bot_j, 0);
    top_j = std::min(top_j, static_cast<int>(fluidobj->numY));

    for(int j = bot_j; j < top_j; ++j)
    {
        fluidobj->mass[0][j] = 0.0;
    }

    // Place the circular obstacle using normalized coordinates so it scales with CELL_LENGTH
    double domain_height = static_cast<double>(fluidobj->i_numY) * CELL_LENGTH;
    double domain_width  = static_cast<double>(fluidobj->i_numX) * CELL_LENGTH;
    double obstacle_x = 0.2 * domain_width;   // 
    double obstacle_y = 0.5 * domain_height;  // mid-height
    double obstacle_radius = 0.12 * domain_height; // radius as fraction of height
    fluidobj->set_circle_obstacle(obstacle_x, obstacle_y, obstacle_radius);
    // Main Event Loop

    bool running = true;
    int frame_count = 0;

    size_t start_tick;
    while (running) 
    {
        start_tick = SDL_GetTicks();
        frame_count +=1;

        // ------------------- EVENT LOOP ----------------------------

        SDL_Event e;
        while (SDL_PollEvent(&e))
        {
            ImGui_ImplSDL3_ProcessEvent(&e);
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


        // -------------------- RENDER LOOP SEGMENT --------------------------

        // imgui
        ImGui_ImplSDLRenderer3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();
        ImGui::ShowDemoWindow();
        

        //main context for the drawing

        ImGui::Render();

        SDL_SetRenderDrawColor(renderer,255,255,255,255);
        SDL_RenderClear(renderer);
        
        //SDL_SetRenderDrawColor(renderer,255,0,0,SDL_ALPHA_OPAQUE);
        //SDL_RenderLine(renderer,0,0,WINDOW_SIZE_X,WINDOW_SIZE_Y);
        //SDL_RenderPoint(renderer,0,0);
        //size_t sim_tick1 = SDL_GetTicks();
        fluidobj->simulate(TIME_STEP,0.0,40);
        //std::cout<<"Frame count: "<<frame_count<<std::endl;
        //const size_t sim_ms = (SDL_GetTicks() - sim_tick1);
        //std::cout<<"Sim Time(ms) = "<< sim_ms <<std::endl;

        const size_t draw_tick1 = SDL_GetTicks();

        for(int i = 1; i < GRID_SIZE_X + 2 - 1; i++)
        {
            const int win_x = i - 1;
            for(int j = 1; j < GRID_SIZE_Y + 2 - 1; j++)
            {
                const int win_y = j - 1;
                const int ty = std::abs(static_cast<int>((j) - (GRID_SIZE_Y + 2 - 1)));
 
                const double smoke = fluidobj->mass[i][j];
                const int smoke_c = std::clamp(smoke * 255.0, 0.0, 255.0);
                const Uint32 gray = 0xFF000000u | (static_cast<Uint32>(smoke_c) << 16) | (static_cast<Uint32>(smoke_c) << 8) | static_cast<Uint32>(smoke_c);
                const Uint32 red  = 0xFFFF0000u;

                if(fluidobj->solid[i][ty] == 0.0)
                {
                    field_pixels[win_y*GRID_SIZE_X+win_x] = red;
                }
                else
                {
                    field_pixels[win_y*GRID_SIZE_X+win_x] = gray;
                }
            }
        }

        SDL_UpdateTexture(field_texture, nullptr, field_pixels.data(), static_cast<int>(GRID_SIZE_X * sizeof(Uint32)));

        SDL_FRect dst = {0.0f, 0.0f, static_cast<float>(WINDOW_SIZE_X), static_cast<float>(WINDOW_SIZE_Y)};
        SDL_RenderTexture(renderer, field_texture, nullptr, &dst);

        ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(),renderer);
        SDL_RenderPresent(renderer);

        //const size_t draw_ms = (SDL_GetTicks() - draw_tick1);
        //std::cout<<"Draw Time(ms) = "<< draw_ms <<std::endl;


        // FIXES FPS WITH A WAIT IF TICK DELTA ISNT TARGET

        size_t current_tick = SDL_GetTicks();
        size_t tick_delta = current_tick-start_tick;
        //std::cout<<"Frame Time(ms) = "<<tick_delta<<std::endl;
        if(tick_delta < TARGET_FRAME_TIME)
        {
            SDL_Delay(TARGET_FRAME_TIME-tick_delta);
        }

    }

    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
    cleanup(window,renderer,field_texture);
    return 0;
}