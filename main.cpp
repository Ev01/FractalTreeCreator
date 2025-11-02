#define SDL_MAIN_USE_CALLBACKS 1
#include<SDL3/SDL.h>
#include<SDL3/SDL_main.h>

#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>

#include "vendored/imgui/imgui.h"
#include "vendored/imgui/imgui_impl_sdl3.h"
//#include "vendored/imgui/imgui_impl_sdlrenderer3.h"
#include "vendored/imgui/imgui_impl_opengl3.h"
#include "glad/glad.h"

#include "ui.h"
#include "tree.h"
#include "debug.h"


static SDL_Window *window;
static SDL_Renderer *renderer;
static SDL_GLContext context;

static double lastFrame, delta;

/*
float vertices[] = {
    0.0f, -0.5f, 0.0f,
    0.0f, 0.0f, 0.0f,
    -0.5f, 0.5f, 0.0f,
    0.5f, 0.5f, 0.0f,
};

unsigned int indices[] = {
    0, 1,
    1, 2,
    1, 3,
};
*/

float vertices[MAX_VERTICES];
unsigned int indices[MAX_INDICES];
int verticesSize = 0;
int indicesSize = 0;

unsigned int VBO;
unsigned int EBO;
unsigned int VAO;

const char *vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "uniform mat4 projection;\n"
    "uniform mat4 view;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = projection * view * vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
    "}\0";

const char *fragmentShaderSource = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "void main()\n"
    "{\n"
        "FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
    "}\n\0";

unsigned int vertexShader;
unsigned int fragmentShader;
unsigned int shaderProgram;

glm::mat4 projection;

void rebuildTree(const TreeSpecies &species, float sway, int maxDepth)
{
    vertices[0] = 0.0;
    vertices[1] = -0.5;
    verticesSize = 2;
    indicesSize = 0;
    buildTree(species, vertices, verticesSize, indices, indicesSize, sway, maxDepth);
}

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
    for (int i = 0; i < verticesSize; i+=2) {
        SDL_Log("Vertex: (%f, %f)", vertices[i], vertices[i + 1]);
    }
    for (int i = 0; i < indicesSize; i+=2) {
        SDL_Log("Line with indices: (%d, %d)", indices[i], indices[i + 1]);
    }
    */

    // Generate stuff for triangle
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float)*2, (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    int success;
    char infoLog[512];

    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        SDL_Log("Vertex shader compilation failed: %s", infoLog);
    }

    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        SDL_Log("Fragment shader compilation failed: %s", infoLog);
    }

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glGetShaderiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        SDL_Log("Shader linking failed: %s", infoLog);
    }

    glViewport(0, 0, 800, 600);
    projection = glm::ortho(0.0f, 800.0f, 0.0f, 600.0f, 0.0f, 2.0f);

    lastFrame = SDL_NS_TO_SECONDS((double) SDL_GetTicksNS());

    return SDL_APP_CONTINUE;
}


SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
    ImGui_ImplSDL3_ProcessEvent(event);
    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;
    }
    else if (event->type == SDL_EVENT_WINDOW_RESIZED) {
        // Set the drawable area to the window's new width and height
        float newWidth = event->window.data1;
        float newHeight = event->window.data2;
        glViewport(0, 0, newWidth, newHeight);
        projection = glm::ortho(0.0f, newWidth, 0.0f, newHeight, 0.0f, 2.0f);
    }
    return SDL_APP_CONTINUE;
}


SDL_AppResult SDL_AppIterate(void *appstate) {
    delta = SDL_NS_TO_SECONDS((double) SDL_GetTicksNS()) - lastFrame;
    lastFrame = SDL_NS_TO_SECONDS((double) SDL_GetTicksNS());
    
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    
    debugInfoWindow(delta);
    Debug_newFrame();

    static TreeSpecies species = {3, 0.3, 50.0, {0, 0, 0}, 1.0, 1.0};
    static int depth = 4;
    static double sway = 0;
    int oldDepth = depth;
    double oldSway = sway;
    bool configChanged = treeConfigWindow(species, depth, sway, window)
        || oldDepth != depth
        || oldSway != sway;
        //|| SDL_fabs(oldSway - sway) > 0.0001;

    if (configChanged) {
        rebuildTree(species, sway, depth);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_DYNAMIC_DRAW);
        SDL_Log("Rebuild, %d indices", indicesSize);
    }


    int windowWidth, windowHeight;
    SDL_GetWindowSize(window, &windowWidth, &windowHeight);
    static int treeX = windowWidth / 2;
    static int treeY = windowHeight * 0.05;

    ImGui::Begin("Tree Position");
    ImGui::SliderInt("X", &treeX, 0, windowWidth);
    ImGui::SliderInt("Y", &treeY, 0, windowHeight);
    ImGui::End();

    ImGui::Render();
    
    //drawTreeRecursive(renderer, treeX, treeY, SDL_PI_D / 2.0, species, 
    //        sway, depth);

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glm::mat4 view = glm::mat4(1.0f);
    view = glm::translate(view, glm::vec3((float)treeX, (float)treeY, 0.0f));


    glUseProgram(shaderProgram);
    int projectionLoc = glGetUniformLocation(shaderProgram, "projection");
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
    int viewLoc = glGetUniformLocation(shaderProgram, "view");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glBindVertexArray(VAO);
    //glDrawArrays(GL_LINE_STRIP, 0, 3);
    glDrawElements(GL_LINES, indicesSize, GL_UNSIGNED_INT, 0);

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    SDL_GL_SwapWindow(window);

    return SDL_APP_CONTINUE;
}


void SDL_AppQuit(void *appstate, SDL_AppResult result) {
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
}
