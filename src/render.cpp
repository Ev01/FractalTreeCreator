#include "render.h"
#include "tree.h"

#include "glad/glad.h"
#include "../vendored/imgui/imgui.h"
#include "../vendored/imgui/imgui_impl_opengl3.h"
#include "glerr.h"

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
unsigned int screenProg;
glm::mat4 projection;
int projectionLoc;
int viewLoc;

unsigned int fbo;
unsigned int fboTexture;
static float screenQuad[] = {
    // Positions    // Tex coords
    -1.0f, -1.0f,   0.0f, 0.0f,     // Bottom left
    -1.0f,  1.0f,   0.0f, 1.0f,     // Top left
     1.0f,  1.0f,   1.0f, 1.0f,     // Top right

    -1.0f, -1.0f,   0.0f, 0.0f,     // Bottom left
     1.0f,  1.0f,   1.0f, 1.0f,     // Top right
     1.0f, -1.0f,   1.0f, 0.0f      // Bottom right
};
unsigned int screenQuadVBO;
unsigned int screenQuadVAO;

int screenWidth = 800;
int screenHeight = 600;

static unsigned int CreateShaderFromFile(const char *filename, GLenum shaderType)
{
    int success;
    char infoLog[512];

    char absolutePath[512];
    SDL_snprintf(absolutePath, 512, "%s../%s", SDL_GetBasePath(), filename);
    char *shaderSource = (char*)SDL_LoadFile(absolutePath, NULL);

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

static unsigned int CreateProgramFromShaders(unsigned int* shaders, unsigned int count)
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


static void CreateMSFramebuffer(int samples=8)
{
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    // Create texture for framebuffer
    glGenTextures(1, &fboTexture);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, fboTexture);
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, GL_RGB, 
                            screenWidth, screenHeight, GL_TRUE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // Bind texture to framebuffer
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
                           GL_TEXTURE_2D_MULTISAMPLE, fboTexture, 0);
    // Check if framebuffer was created properly
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        SDL_Log("Error: Framebuffer is not complete!");
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


bool Render::Init()
{
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    //SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    //SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

    int flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;
    
    window = SDL_CreateWindow("Fractal Tree Creator", screenWidth, screenHeight, flags);
    if (window == NULL) {
        SDL_Log("Couldn't create window: %s", SDL_GetError());
        return false;
    }

    context = SDL_GL_CreateContext(window);

    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
        SDL_Log("Failed to initialise GLAD");
        return false;
    }

    // Generate screen quad vertex array object
    glGenVertexArrays(1, &screenQuadVAO);
    glGenBuffers(1, &screenQuadVBO);

    glBindVertexArray(screenQuadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, screenQuadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(screenQuad), &screenQuad, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float)*4, (void*)0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float)*4, (void*)(sizeof(float)*2));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    // Generate Vertex Array Object to store the tree's draw info.
    glGenVertexArrays(1, &(treeDrawInfo.VAO));
    glGenBuffers(1, &(treeDrawInfo.VBO));
    glGenBuffers(1, &(treeDrawInfo.EBO));

    glBindVertexArray(treeDrawInfo.VAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, treeDrawInfo.EBO);
    glBindBuffer(GL_ARRAY_BUFFER, treeDrawInfo.VBO);
    /*
    glBufferData(GL_ARRAY_BUFFER, sizeof(treeDrawInfo.vertices),
                 treeDrawInfo.vertices, GL_DYNAMIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(treeDrawInfo.indices),
                 treeDrawInfo.indices, GL_DYNAMIC_DRAW);
    */
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float)*2, (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    // Load shaders
    vertexShader = CreateShaderFromFile("shaders/vertex.glsl", GL_VERTEX_SHADER);
    fragmentShader = CreateShaderFromFile("shaders/fragment.glsl", GL_FRAGMENT_SHADER);
    unsigned int shaders[2] = {vertexShader, fragmentShader};
    shaderProgram = CreateProgramFromShaders(shaders, 2);
    projectionLoc = glGetUniformLocation(shaderProgram, "projection");
    viewLoc = glGetUniformLocation(shaderProgram, "view");
    
    unsigned int vScreenShader = CreateShaderFromFile("shaders/v_screen.glsl", GL_VERTEX_SHADER);
    unsigned int fScreenShader = CreateShaderFromFile("shaders/f_screen.glsl", GL_FRAGMENT_SHADER);
    unsigned int shaders2[2] = {vScreenShader, fScreenShader};
    screenProg = CreateProgramFromShaders(shaders2, 2);
    glDeleteShader(vScreenShader);
    glDeleteShader(fScreenShader);
    

    // Create Framebuffer
    CreateMSFramebuffer();
    

    glViewport(0, 0, screenWidth, screenHeight);
    projection = glm::ortho(0.0f, (float)screenWidth, 0.0f, (float)screenHeight, 0.0f, 2.0f);

    glEnable(GL_MULTISAMPLE);

    return true;
}


void Render::UpdateViewportSize(float width, float height)
{
    glViewport(0, 0, width, height);
    projection = glm::ortho(0.0f, width, 0.0f, height, 0.0f, 2.0f);
    screenWidth = (int)width;
    screenHeight = (int)height;

    // Recreate framebuffer to use new screen size
    glDeleteFramebuffers(1, &fbo);
    glDeleteTextures(1, &fboTexture);
    CreateMSFramebuffer();
}


void Render::DrawFrame(int treeX, int treeY, float zoom)
{
    glm::mat4 view = glm::mat4(1.0f);
    view = glm::translate(view, glm::vec3((float)treeX, (float)treeY, 0.0f));
    view = glm::scale(view, glm::vec3(zoom, zoom, zoom));

    
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Render the tree
    glUseProgram(shaderProgram);
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glBindVertexArray(treeDrawInfo.VAO);
    //glDrawArrays(GL_LINE_STRIP, 0, 3);
    //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDrawElements(GL_LINES, treeDrawInfo.indicesSize, GL_UNSIGNED_INT, 0);

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, screenWidth, screenHeight, 0, 0, screenWidth, screenHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);

    /*
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    
    glUseProgram(screenProg);
    glBindVertexArray(screenQuadVAO);
    glBindTexture(GL_TEXTURE_2D, fboTexture);
    glDisable(GL_DEPTH_TEST);
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    */
    


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
    glDeleteFramebuffers(1, &fbo);
    glDeleteTextures(1, &fboTexture);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    glDeleteProgram(shaderProgram);
    glDeleteProgram(screenProg);
}
