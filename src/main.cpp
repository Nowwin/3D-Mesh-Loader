// C++ Standard Libraries
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <filesystem>

// Third Party
#if defined(LINUX) || defined(MINGW)
    #include <SDL2/SDL.h>
#else // This works for Mac
    #include <SDL.h>
#endif

//Third Party
#include <glad/glad.h>
#include <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/ext/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale
#include <glm/ext/matrix_clip_space.hpp> // glm::perspective
#include <glm/ext/scalar_constants.hpp> // glm::pi
#include <glm/gtc/type_ptr.hpp>

#include "objfileloader.hpp"

//Globals
int gScreenHeight = 480;
int gScreenWidth = 640;
SDL_Window* gGraphicsApplicationWindow = nullptr;
SDL_GLContext gOpenGlContext = nullptr;
std::string gFileLocation = "src/ObjFiles/tetrahedron.obj";

bool gQuit = false;

GLuint gVertexArrayObject = 0;
GLuint gVertexBufferObject = 0;
GLuint gIndexBufferObject = 0;
GLuint gGraphicsPipelineShaderProgram = 0;
GLsizei numberOfVertices = 0;

float gUOffset = -2.0f;
float gURotation = 0.0f;

//Error Handling

static void GLClearAllErrors() {
    while (glGetError() != GL_NO_ERROR)
    {
        /* code */
    }    
}

static bool GLCheckErrorStatus(const char* function, int line) {
    while (GLenum error = glGetError())
    {
        std::cout << "OpenGL Error: " << error << "\tLine: " << line
        << "\tFunction: " << function << std::endl;
        return true;
    }

    return false;
    
}

#define GLCheck(x) GLClearAllErrors(); x; GLCheckErrorStatus(#x, __LINE__);

//Error Handling end


std::string LoadShaderAsString(const std::string& filename) {
    std::string result = "";

    std::string line = "";
    std::ifstream myFile(filename.c_str());

    if (myFile.is_open())
    {
        while (std::getline(myFile, line))
        {
            result += line + '\n';
        }
        myFile.close();   
    }

    

    return result;
    
}


void GetOpenGLVersionInfo() {
    std::cout << "Vendor: " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "Shading Language: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
}

void VertexSpecification() {
    
    std::vector<glm::vec3> vertices;  // for 'v' 
    std::vector<glm::vec3> normals;   // for 'vn'
    std::vector<glm::vec2> texCoords; // for 'vt'
    std::vector<glm::ivec3> faces;    // for 'f', stores indices for vertices, normals, and texCoords

    LoadObj(gFileLocation, vertices, normals, texCoords, faces);

    std::vector<GLfloat> vertexData;
    for (const auto& face : faces) {
        // Add vertex positions
        vertexData.push_back(vertices[face.x].x);
        vertexData.push_back(vertices[face.x].y);
        vertexData.push_back(vertices[face.x].z);

        // Add normals
        vertexData.push_back(normals[face.y].x);
        vertexData.push_back(normals[face.y].y);
        vertexData.push_back(normals[face.y].z);

        // Add texture coordinates
        vertexData.push_back(texCoords[face.z].x);
        vertexData.push_back(texCoords[face.z].y);
    }

    glGenVertexArrays(1, &gVertexArrayObject);
    glBindVertexArray(gVertexArrayObject);
    glGenBuffers(1, &gVertexBufferObject);
    glBindBuffer(GL_ARRAY_BUFFER, gVertexBufferObject);
    glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(GLfloat), vertexData.data(), GL_STATIC_DRAW);

    //Position Attribute
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * 8, (void*)0);

    //Normal Attribute
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * 8, (GLvoid*)(sizeof(GL_FLOAT)*3));

    //Texture Attribute
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * 8, (GLvoid*)(sizeof(GL_FLOAT)*6));

    //Setting number of vertices
    numberOfVertices = 3 * faces.size();

    //Clean-Up
    glBindVertexArray(0);    
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);

}

GLuint CompileShader(GLuint type, const std::string &source) {
    
    GLuint shaderPbject;
    
    if (type == GL_VERTEX_SHADER)
    {
        shaderPbject = glCreateShader(GL_VERTEX_SHADER);
    } else if(type == GL_FRAGMENT_SHADER) {
        shaderPbject = glCreateShader(GL_FRAGMENT_SHADER);
    }

    const char* src = source.c_str();
    glShaderSource(shaderPbject, 1, &src, nullptr);
    glCompileShader(shaderPbject);

    return shaderPbject;
    
}

GLuint CreateShaderProgram(const std::string &vertexshadersource, const std::string &fragmentshadersource) {
    GLuint programObject = glCreateProgram();

    GLuint myVertexShader = CompileShader(GL_VERTEX_SHADER, vertexshadersource);
    GLuint myFragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragmentshadersource);

    glAttachShader(programObject, myVertexShader);
    glAttachShader(programObject, myFragmentShader);
    glLinkProgram(programObject);

    //Validation
    glValidateProgram(programObject);

    return programObject;
}

void CreateGraphicsPipeline() {
    std::string vertexShaderSource = LoadShaderAsString(std::filesystem::current_path().string() + "/shaders/vert.glsl");
    std::string fragmentShaderSource = LoadShaderAsString(std::filesystem::current_path().string() + "/shaders/frag.glsl");
    gGraphicsPipelineShaderProgram = CreateShaderProgram(vertexShaderSource, fragmentShaderSource);
}

void IntializeProgram() {

    std::cout << std::filesystem::current_path().string() << std::endl; 

    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        std::cout << "SDL did not initiliaze" << std::endl;
        exit(1);
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    gGraphicsApplicationWindow = SDL_CreateWindow(
        "SDL2 Window",               // window title
        0,      // x position, centered
        0,      // y position, centered
        gScreenWidth,                         // width, in pixels
        gScreenHeight,                         // height, in pixels
        SDL_WINDOW_OPENGL             // flags
    );

    if (gGraphicsApplicationWindow == nullptr)
    {
        std::cout << "Window did not setup" << std::endl;
        exit(1);
    }

    gOpenGlContext = SDL_GL_CreateContext(gGraphicsApplicationWindow);

    if (gOpenGlContext == nullptr)
    {
        std::cout << "Graphics did not load" << std::endl;
        exit(1);
    }
    
    if (!gladLoadGLLoader(SDL_GL_GetProcAddress))
    {
        std::cout << "glad was not initialized" << std::endl;
        exit(1);
    }

    GetOpenGLVersionInfo();
    
}

void Input(){
    SDL_Event event;

    // (1) Handle Input
    // Start our event loop
    while(SDL_PollEvent(&event)){
        // Handle each specific event
        if(event.type == SDL_QUIT){
            gQuit= true;
        }
        
    }

    gURotation = gURotation + 0.5f;

}

void PreDraw() {
    //Setting opengl state
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glViewport(0, 0, gScreenWidth, gScreenHeight);
    glClearColor(1.f, 1.f, 0.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(gGraphicsPipelineShaderProgram);

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::rotate(model, glm::radians(gURotation), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)gScreenWidth/(float)gScreenHeight, 0.1f, 100.0f);


    GLint modelLoc = glGetUniformLocation(gGraphicsPipelineShaderProgram, "model");
    GLint viewLoc = glGetUniformLocation(gGraphicsPipelineShaderProgram, "view");
    GLint projLoc = glGetUniformLocation(gGraphicsPipelineShaderProgram, "projection");
    GLint lightPosLoc = glGetUniformLocation(gGraphicsPipelineShaderProgram, "lightPos");
    GLint colorLoc = glGetUniformLocation(gGraphicsPipelineShaderProgram, "color");

    glUniform3f(lightPosLoc, 1.0f, 1.0f, 0.0f);
    glUniform4f(colorLoc, 1.0f, 0.0f, 0.0f, 1.0f);
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

}

void Draw() {
    // Bind VAO and draw the object
    glBindVertexArray(gVertexArrayObject);
    glDrawArrays(GL_TRIANGLES, 0, numberOfVertices);  // numberOfVertices should be set to the number of vertices you have
    glBindVertexArray(0);

}

void MainLoop() {
    while (!gQuit)
    {
        Uint64 start = SDL_GetPerformanceCounter();

        Input();

        PreDraw();

        Draw();

        SDL_GL_SwapWindow(gGraphicsApplicationWindow);

        Uint64 end = SDL_GetPerformanceCounter();
        float elapsedMS = (end - start) / (float)SDL_GetPerformanceFrequency() * 1000.0f;
        SDL_Delay(floor(11.111f - elapsedMS));
    }
    
}

void CleanUp() {
    SDL_DestroyWindow(gGraphicsApplicationWindow);
    SDL_Quit();
}

int main() {
    IntializeProgram();

    VertexSpecification();

    CreateGraphicsPipeline();

    MainLoop();

    CleanUp();

    return 0;
}