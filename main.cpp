#define SDL_MAIN_USE_CALLBACKS 1
#include<SDL3/SDL.h>
#include<SDL3/SDL_main.h>

#include "vendored/imgui/imgui.h"
#include "vendored/imgui/imgui_impl_sdl3.h"
//#include "vendored/imgui/imgui_impl_sdlrenderer3.h"
#include "vendored/imgui/imgui_impl_opengl3.h"
#include "glad/glad.h"

#include "ui.h"
#include "tree.h"
#include "debug.h"

SDL_Window *window;
SDL_Renderer *renderer;

static SDL_GLContext context;


double lastFrame, delta;


SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    int flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;
    
    window = SDL_CreateWindow("Fractal Tree", 800, 600, flags);
    if (window == NULL) {
        SDL_Log("Couldn't create window: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    context = SDL_GL_CreateContext(window);

    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
        SDL_Log("Failed to initialise GLAD");
        return SDL_APP_FAILURE;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    ImGui::StyleColorsDark();
    
    ImGui_ImplSDL3_InitForOpenGL(window, &context);
    ImGui_ImplOpenGL3_Init(nullptr);

    /*
    if (!SDL_CreateWindowAndRenderer(
                "Fractal Tree", 800, 600, SDL_WINDOW_RESIZABLE, &window, &renderer)) {
        SDL_Log("Couldn't create window and renderer: %s", SDL_GetError());
    }
    SDL_SetRenderVSync(renderer, 1);


    ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer3_Init(renderer);
    */

    lastFrame = SDL_NS_TO_SECONDS((double) SDL_GetTicksNS());

    return SDL_APP_CONTINUE;
}


SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
    ImGui_ImplSDL3_ProcessEvent(event);
    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;
    }
    return SDL_APP_CONTINUE;
}


SDL_AppResult SDL_AppIterate(void *appstate) {
    delta = SDL_NS_TO_SECONDS((double) SDL_GetTicksNS()) - lastFrame;
    lastFrame = SDL_NS_TO_SECONDS((double) SDL_GetTicksNS());
    
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    
    //debugInfoWindow(delta);
    //Debug_newFrame();

    static TreeSpecies species = {3, 0.3, 50, {0, 0, 0}, 1.0, 1.0};
    static int depth = 4;
    static double sway = 0;
    treeConfigWindow(species, depth, sway, window);


    int windowWidth, windowHeight;
    SDL_GetWindowSize(window, &windowWidth, &windowHeight);
    static int treeX = windowWidth / 2;
    static int treeY = windowHeight * 0.95;

    ImGui::Begin("Tree Position");
    ImGui::SliderInt("X", &treeX, 0, windowWidth);
    ImGui::SliderInt("Y", &treeY, 0, windowHeight);
    ImGui::End();

    ImGui::Render();
    
    //SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    //SDL_RenderClear(renderer);

    //SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    //drawTreeRecursive(renderer, treeX, treeY, SDL_PI_D / 2.0, species, 
    //        sway, depth);

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    //SDL_RenderPresent(renderer);
    
    SDL_GL_SwapWindow(window);

    return SDL_APP_CONTINUE;
}


void SDL_AppQuit(void *appstate, SDL_AppResult result) {
    
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
}
