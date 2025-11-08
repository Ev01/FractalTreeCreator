#pragma once

#include<SDL3/SDL.h>

#define MAX_VERTICES 10000000
#define MAX_INDICES 10000000



struct DrawInfo {
    float vertices[MAX_VERTICES];
    unsigned int indices[MAX_INDICES];
    int verticesSize = 0;
    int indicesSize = 0;

    unsigned int VBO;
    unsigned int EBO;
    unsigned int VAO;
};


namespace Render {
    bool Init();
    void UpdateViewportSize(float width, float height);
    void DrawFrame(int treeX, int treeY, float zoom);
    DrawInfo& GetTreeDrawInfo();
    SDL_Window* GetWindow();
    SDL_GLContext& GetGLContext();
    void CleanUp();
};
