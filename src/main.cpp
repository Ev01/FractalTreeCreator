#define SDL_MAIN_USE_CALLBACKS 1
#include<SDL3/SDL.h>
#include<SDL3/SDL_main.h>

#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>

#include "../vendored/imgui/imgui.h"
#include "../vendored/imgui/imgui_impl_sdl3.h"
//#include "vendored/imgui/imgui_impl_sdlrenderer3.h"
#include "../vendored/imgui/imgui_impl_opengl3.h"
#include "glad/glad.h"

#include "ui.h"
#include "tree.h"
#include "render.h"

#define PROJECT_VERSION "0.1.0a"


static double lastFrame, delta;

static TreeSpecies species = {3, 0.3, 50.0, {0, 0, 0}, 1.0, 1.0};
static int depth = 4;
static double sway = 0;

static float treeX = 400;
static float treeY = 20;


static void RebuildTree(const TreeSpecies &species, float sway, int maxDepth)
{
    DrawInfo &treeDrawInfo = Render::GetTreeDrawInfo();
    treeDrawInfo.vertices[0] = 0.0;
    treeDrawInfo.vertices[1] = 0.0;
    treeDrawInfo.verticesSize = 2;
    treeDrawInfo.indicesSize = 0;
    BuildTree(species, treeDrawInfo.vertices, treeDrawInfo.verticesSize,
              treeDrawInfo.indices, treeDrawInfo.indicesSize, sway, maxDepth);
    glBindVertexArray(treeDrawInfo.VAO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(treeDrawInfo.vertices),
                 treeDrawInfo.vertices, GL_DYNAMIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(treeDrawInfo.indices),
                 treeDrawInfo.indices, GL_DYNAMIC_DRAW);
    //SDL_Log("Rebuild, %d indices (lines), %d vertices (Points)", 
    //        treeDrawInfo.indicesSize, treeDrawInfo.verticesSize);
}

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD);
    SDL_SetAppMetadata("Fractal Tree Creator", PROJECT_VERSION, "com.evanbarac.fractaltree");

    Render::Init();

    // Initiate IMGUI
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    ImGui::StyleColorsDark();
    
    ImGui_ImplSDL3_InitForOpenGL(Render::GetWindow(), &Render::GetGLContext());
    ImGui_ImplOpenGL3_Init(nullptr);

    // First build of the tree. Tree will only rebuild again when the
    // configuration changes
    RebuildTree(species, sway, depth);

    lastFrame = SDL_NS_TO_SECONDS((double) SDL_GetTicksNS());
    SDL_Log("Base path:");
    SDL_Log("%s", SDL_GetBasePath());

    return SDL_APP_CONTINUE;
}


SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
    ImGuiIO &io = ImGui::GetIO(); (void)io;
    ImGui_ImplSDL3_ProcessEvent(event);

    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;
    }
    else if (event->type == SDL_EVENT_WINDOW_RESIZED) {
        // Set the drawable area to the window's new width and height
        float newWidth = event->window.data1;
        float newHeight = event->window.data2;
        Render::UpdateViewportSize(newWidth, newHeight);
    }

    if (io.WantCaptureMouse) {
        return SDL_APP_CONTINUE;
    }

    bool isMiddleMousePressed = SDL_GetMouseState(NULL, NULL) & SDL_BUTTON_MIDDLE;
    if (isMiddleMousePressed && event->type == SDL_EVENT_MOUSE_MOTION) {
        treeX += event->motion.xrel;
        treeY -= event->motion.yrel;
    }
    else if (event->type == SDL_EVENT_MOUSE_WHEEL) {
        int windowWidth, windowHeight;
        SDL_GetWindowSize(Render::GetWindow(), &windowWidth, &windowHeight);
        float zoomFactor = (event->wheel.y * 0.1 + 1.0);
        // Difference between base of the tree and the mouse position
        float xDiff = treeX - event->wheel.mouse_x;
        float yDiff = treeY - ((float)windowHeight - event->wheel.mouse_y);
        //SDL_Log("xDiff: %f, yDiff: %f", xDiff, yDiff);
        species.baseBranchLen *= zoomFactor;
        // Move the tree so it zooms in on the mouse, not the base of the tree.
        treeX += xDiff * (zoomFactor-1.0);
        treeY += yDiff * (zoomFactor-1.0);
        //rebuildTree(species, sway, depth);
    }



    return SDL_APP_CONTINUE;
}


SDL_AppResult SDL_AppIterate(void *appstate) {
    delta = SDL_NS_TO_SECONDS((double) SDL_GetTicksNS()) - lastFrame;
    lastFrame = SDL_NS_TO_SECONDS((double) SDL_GetTicksNS());
    
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    DebugInfoWindow(delta);
    //Debug_newFrame();

    int oldDepth = depth;
    double oldSway = sway;
    bool configChanged = TreeConfigWindow(species, depth, sway, Render::GetWindow())
        || oldDepth != depth
        || oldSway != sway;
        //|| SDL_fabs(oldSway - sway) > 0.0001;

    if (configChanged) {
        RebuildTree(species, sway, depth);
    }
    // If the user loaded a new tree, rebuild the tree.
    if (GetTreeRecentlyLoaded()) {
        RebuildTree(species, sway, depth);
        // Make sure getTreeRecentlyLoaded() will return false until the user
        // loads again
        ClearTreeRecentlyLoaded();
    }


    int windowWidth, windowHeight;
    SDL_GetWindowSize(Render::GetWindow(), &windowWidth, &windowHeight);

    // UI Stuff
    ImGui::Begin("Tree Position");
    ImGui::SliderFloat("X", &treeX, 0.0f, (float)windowWidth);
    ImGui::SliderFloat("Y", &treeY, 0.0f, (float)windowHeight);
    ImGui::End();

    ImGui::Render();

    Render::DrawFrame((int)treeX, (int)treeY, species.baseBranchLen);

    return SDL_APP_CONTINUE;
}


void SDL_AppQuit(void *appstate, SDL_AppResult result) {
    Render::CleanUp();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
    SDL_Quit();
}
