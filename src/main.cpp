#include <iostream>
#include <vector>
#include <string>
#include <random>
#include <memory>
#include <algorithm>
#include <chrono>
#include <thread>
#include <cmath>

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"

#include "vectors.h"
#include "Fluid.h"


double max2D(const std::vector<std::vector<double>>& vec)
{
    double max = 0.0;
    for(int i = 0; i<vec.size();i++)
    {
        for(int j = 0; j<vec[i].size();j++)
        {
            if(vec[i][j] > max)
            {
                max = vec[i][j];
            }
        }
    }
    return max;
}

double sigmoid(double val,double k)
{
    return ((1)/(1+(std::exp((-val)*k))));
}


void cleanup(SDL_Window* window, SDL_Renderer* renderer, SDL_Texture* texture)
{
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_DestroyTexture(texture);
    SDL_Quit();
    std::cout<<"Cleanup complete"<<std::endl;
}

Uint32 rgb_scientific_colour_map(double value, double min, double max,double k_sig)
{
    float delta = (max-min);
    float temp = (value - min)/delta;
    temp = std::clamp(temp,0.0f,1.0f);
    float temp_normalised = (temp-0.5f)*2;
    temp = sigmoid(temp_normalised,k_sig);
    float r,g,b;

    r = temp;
    b = 1.0f-temp;
    g = 0.0f;

    Uint32 r_i = static_cast<Uint32>(r*255.0f + 0.5f);
    Uint32 g_i = static_cast<Uint32>(g*255.0f + 0.5f);
    Uint32 b_i = static_cast<Uint32>(b*255.0f + 0.5f);

    Uint32 alpha = 255;

    Uint32 colour = (alpha << 24) | (r_i << 16) | (g_i << 8) | b_i;

    return colour;
}

struct FluidSimRenderState
{
    bool show_streamlines = false;
    int sl_segments = 10;
    double sl_segement_len = 0.0;
    bool show_obstacles = false;
    // Pressure parameters
    bool show_pressure =  false;
    float p_max = 100000.0f;
    float p_sig_k = 1.0f;

    bool show_mass = false;
    bool show_velocity = false;
};

int main(int argc, char *argv[])
{
    // Simulation Paramters
    const size_t GRID_SIZE_X = 150;
    const size_t GRID_SIZE_Y = 150;
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

    // Extra space reserved for the left ImGui sidebar.
    // This keeps the simulation area from shrinking when the sidebar is shown.
    const int INITIAL_SIDEBAR_WIDTH_PX = 320;

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

    SDL_Window *window = SDL_CreateWindow(
        WINDOW_NAME,
        static_cast<int>(WINDOW_SIZE_X) + INITIAL_SIDEBAR_WIDTH_PX,
        static_cast<int>(WINDOW_SIZE_Y),
        0
    );
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

    //std::vector<Uint32> field_pixels(GRID_SIZE_X * GRID_SIZE_Y, 0xFFFFFFFFu);

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
    float inlet_velocity = 10.0;
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

    // ImGui UI state
    bool show_imgui_demo = false;
    bool pause_sim = false;
    float sidebar_width = static_cast<float>(INITIAL_SIDEBAR_WIDTH_PX);

    // what to render 

    FluidSimRenderState fs_render_state;

    fs_render_state.show_mass = true;
    fs_render_state.show_obstacles = true;
    

    fs_render_state.sl_segement_len = CELL_LENGTH/fs_render_state.sl_segments;


    //
    const Uint32 red  = 0xFFFF0000;

    size_t start_tick;
    while (running) 
    {
        start_tick = SDL_GetTicks();
        frame_count +=1;
        std::vector<Uint32> field_pixels(GRID_SIZE_X * GRID_SIZE_Y, 0xFFFFFFFFu);
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

        // Fixed left sidebar
        const ImVec2 display_size = io.DisplaySize;
        const float clamped_sidebar_width = std::clamp(sidebar_width, 200.0f, std::max(200.0f, display_size.x - 50.0f));

        ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(clamped_sidebar_width, display_size.y), ImGuiCond_Always);
        const ImGuiWindowFlags sidebar_flags = ImGuiWindowFlags_NoMove |
                                              ImGuiWindowFlags_NoResize |
                                              ImGuiWindowFlags_NoCollapse |
                                              ImGuiWindowFlags_NoSavedSettings;

        if (ImGui::Begin("Controls", nullptr, sidebar_flags))
        {
            ImGui::Text("CFD SIM");
            ImGui::Separator();
            if(ImGui::CollapsingHeader("Renderering Details"))
            {
                ImGui::Text("Frame: %d", frame_count);
                ImGui::Text("FPS: %.1f", io.Framerate);
            }
            ImGui::Checkbox("Show ImGui demo", &show_imgui_demo);
            if(ImGui::CollapsingHeader("Simulation rendering options"))
            {
                ImGui::Checkbox("Show Mass",&(fs_render_state.show_mass)); 
                ImGui::Checkbox("Show obstalces",&(fs_render_state.show_obstacles));
                ImGui::Checkbox("Show pressure",&(fs_render_state.show_pressure));
                if(fs_render_state.show_pressure == true)
                {
                    ImGui::SliderFloat("Max P",&(fs_render_state.p_max),0.0f,250000.0f);
                    ImGui::SliderFloat("P Sigmoid K",&(fs_render_state.p_sig_k),0.0f,20.0f);
                    ImGui::Separator();
                }
                ImGui::Checkbox("Show StreamLines",&(fs_render_state.show_streamlines));
            }
            if(ImGui::CollapsingHeader("Simulation Options",ImGuiTreeNodeFlags_DefaultOpen))
            {
                ImGui::Checkbox("Pause simulation", &pause_sim);
            }
            if(ImGui::CollapsingHeader("Simulation Details",ImGuiTreeNodeFlags_DefaultOpen))
            {
                ImGui::Text("Sim Grid: %i by %i",GRID_SIZE_X,GRID_SIZE_Y);
                ImGui::SliderFloat("Inlet Velocity",&inlet_velocity,0.0f,50.0f);
            }
        }
        ImGui::End();

        if (show_imgui_demo)
        {
            ImGui::ShowDemoWindow(&show_imgui_demo);
        }
        

        //main context for the drawing

        ImGui::Render();

        SDL_SetRenderDrawColor(renderer,255,255,255,255);
        SDL_RenderClear(renderer);
        
        //SDL_SetRenderDrawColor(renderer,255,0,0,SDL_ALPHA_OPAQUE);
        //SDL_RenderLine(renderer,0,0,WINDOW_SIZE_X,WINDOW_SIZE_Y);
        //SDL_RenderPoint(renderer,0,0);
        //size_t sim_tick1 = SDL_GetTicks();
        if (!pause_sim)
        {
            fluidobj->simulate(TIME_STEP,0.0,30);
        }
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
                const int ty = std::abs(static_cast<int>((j) - (GRID_SIZE_Y + 2 - 1))); // transforms top left to bottom left 
                if(fs_render_state.show_mass == true)
                {
                    double smoke = fluidobj->mass[i][j];
                    int smoke_c = std::clamp(smoke * 255.0, 0.0, 255.0);
                    Uint32 gray = 0xFF000000u | (static_cast<Uint32>(smoke_c) << 16) | (static_cast<Uint32>(smoke_c) << 8) | static_cast<Uint32>(smoke_c);
                    field_pixels[win_y*GRID_SIZE_X+win_x] = gray;
                }
                if(fs_render_state.show_pressure == true)
                {
                    double pressure_val = fluidobj->pressure[i][j];
                    Uint32 pressure_c = rgb_scientific_colour_map(pressure_val,0.0,fs_render_state.p_max,fs_render_state.p_sig_k);
                    field_pixels[win_y*GRID_SIZE_X + win_x] = pressure_c;
                }
                if(fs_render_state.show_streamlines == true)
                {
                    if(i%5 == 0 && j%5 == 0)
                    {
                        double x_pos = (i+0.5)*CELL_LENGTH;
                        double y_pos = (ty+0.5)*CELL_LENGTH;
                    }
                }
                if(fs_render_state.show_obstacles == true && fluidobj->solid[i][ty] == 0.0)
                {
                    field_pixels[win_y*GRID_SIZE_X+win_x] = red;
                }
                
            }
        }

        SDL_UpdateTexture(field_texture, nullptr, field_pixels.data(), static_cast<int>(GRID_SIZE_X * sizeof(Uint32)));

        int window_px_w = 0;
        int window_px_h = 0;
        SDL_GetWindowSize(window, &window_px_w, &window_px_h);
        const float sim_w = std::max(0.0f, static_cast<float>(window_px_w) - clamped_sidebar_width);
        SDL_FRect dst = {clamped_sidebar_width, 0.0f, sim_w, static_cast<float>(window_px_h)};
        SDL_RenderTexture(renderer, field_texture, nullptr, &dst);


        // this is bl origined Made to do post processing ontop of the base texture
        if(fs_render_state.show_streamlines == true)
        {
            for(int i = 1; i< GRID_SIZE_X +2 -1; i++)
            {
                int win_x = i-1;
                for(int j = 1; j< GRID_SIZE_Y +2 -1; j++)
                {
                    int win_j = j-1;
                    if(fs_render_state.show_streamlines == true)
                    {
                        if(i%5 == 0 && j%5 == 0)
                        {
                            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                            double x_start_sim = (i+0.5)*CELL_LENGTH;
                            double y_start_sim = (j+0.5)*CELL_LENGTH;

                            float x_start_window = (x_start_sim/CELL_LENGTH)*(PIXEL_SCALE) + sidebar_width -1;
                            float y_start_window = GRID_SIZE_Y*PIXEL_SCALE-(y_start_sim/CELL_LENGTH)*(PIXEL_SCALE) -1;

                            for(int segs = 0; segs<fs_render_state.sl_segments; segs++)
                            {
                                double u_sample = fluidobj->grid_interpolation(x_start_sim,y_start_sim,Fluid::Field::U);
                                double v_sample = fluidobj->grid_interpolation(x_start_sim,y_start_sim,Fluid::Field::V);

                                double sl_ts = 0.01;

                                x_start_sim += u_sample*sl_ts;
                                y_start_sim += v_sample*sl_ts;
                                
                                float x_win_temp = (x_start_sim/CELL_LENGTH)*(PIXEL_SCALE) +sidebar_width -1;
                                float y_win_temp = GRID_SIZE_Y*PIXEL_SCALE-(y_start_sim/CELL_LENGTH)*PIXEL_SCALE -1;

                                SDL_RenderLine(renderer,x_start_window,y_start_window,x_win_temp,y_win_temp);

                                x_start_window = x_win_temp;
                                y_start_window = y_win_temp;
                            }
                        }
                    }
                }
            }
        }

        // FINAL PRESENTATION
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
