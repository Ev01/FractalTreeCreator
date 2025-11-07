#include "render.h"
#include "tree.h"

#include "glad/glad.h"
#include "vendored/imgui/imgui.h"
#include "vendored/imgui/imgui_impl_opengl3.h"

#include<SDL3/SDL.h>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>


SDL_Window *window;
SDL_GLContext context;

DrawInfo treeDrawInfo;

unsigned int vertexShader;
unsigned int fragmentShader;
unsigned int shaderProgram;
glm::mat4 projection;
int projectionLoc;
int viewLoc;

static unsigned int createShaderFromFile(const char *filename, GLenum shaderType)
{
    int success;
    char infoLog[512];

    char *shaderSource = (char*)SDL_LoadFile(filename, NULL);

    unsigned int shaderID = glCreateShader(shaderType);
    glShaderSource(shaderID, 1, &shaderSource, NULL);
    glCompileShader(shaderID);
    glGetShaderiv(shaderID, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shaderID, 512, NULL, infoLog);
        SDL_Log("Vertex shader compilation failed: %s", infoLog);
    }

    SDL_free(shaderSource);
    return shaderID;
}

static unsigned int createProgramFromShaders(unsigned int* shaders, unsigned int count)
{
    int success;
    char infoLog[512];

    unsigned int programID = glCreateProgram();
    for (int i = 0; i < count; i++) {
        glAttachShader(programID, shaders[i]);
    }
    glLinkProgram(programID);
    glGetShaderiv(programID, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(programID, 512, NULL, infoLog);
        SDL_Log("Shader linking failed: %s", infoLog);
    }

    return programID;
}

bool Render::Init()
{
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    int flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;
    
    window = SDL_CreateWindow("Fractal Tree", 800, 600, flags);
    if (window == NULL) {
        SDL_Log("Couldn't create window: %s", SDL_GetError());
        return false;
    }

    context = SDL_GL_CreateContext(window);

    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
        SDL_Log("Failed to initialise GLAD");
        return false;
    }

    // Generate Vertex Array Object to store the tree's draw info.
    glGenVertexArrays(1, &(treeDrawInfo.VAO));
    glGenBuffers(1, &(treeDrawInfo.VBO));
    glGenBuffers(1, &(treeDrawInfo.EBO));

    glBindVertexArray(treeDrawInfo.VAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, treeDrawInfo.EBO);
    glBindBuffer(GL_ARRAY_BUFFER, treeDrawInfo.VBO);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float)*2, (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    // Load shaders
    vertexShader = createShaderFromFile("shaders/vertex.glsl", GL_VERTEX_SHADER);
    fragmentShader = createShaderFromFile("shaders/fragment.glsl", GL_FRAGMENT_SHADER);
    unsigned int shaders[2] = {vertexShader, fragmentShader};
    shaderProgram = createProgramFromShaders(shaders, 2);
    projectionLoc = glGetUniformLocation(shaderProgram, "projection");
    viewLoc = glGetUniformLocation(shaderProgram, "view");

    glViewport(0, 0, 800, 600);
    projection = glm::ortho(0.0f, 800.0f, 0.0f, 600.0f, 0.0f, 2.0f);

    return true;
}


void Render::UpdateViewportSize(float width, float height)
{
    glViewport(0, 0, width, height);
    projection = glm::ortho(0.0f, width, 0.0f, height, 0.0f, 2.0f);
}


void Render::DrawFrame(int treeX, int treeY, float zoom)
{
    glm::mat4 view = glm::mat4(1.0f);
    view = glm::translate(view, glm::vec3((float)treeX, (float)treeY, 0.0f));
    view = glm::scale(view, glm::vec3(zoom, zoom, zoom));

    
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Render the tree
    glUseProgram(shaderProgram);
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glBindVertexArray(treeDrawInfo.VAO);
    //glDrawArrays(GL_LINE_STRIP, 0, 3);
    glDrawElements(GL_LINES, treeDrawInfo.indicesSize, GL_UNSIGNED_INT, 0);

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    SDL_GL_SwapWindow(window);
}

DrawInfo& Render::GetTreeDrawInfo()
{
    return treeDrawInfo;
}

SDL_Window* Render::GetWindow()
{
    return window;
}

SDL_GLContext& Render::GetGLContext()
{
    return context;
}

void Render::CleanUp()
{
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    glDeleteProgram(shaderProgram);
}
