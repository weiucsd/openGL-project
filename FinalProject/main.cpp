#include <iostream>

// GLEW 1.13
#define GLEW_STATIC
#include <GL/glew.h>

// GLFW 3.1
#include <GLFW/glfw3.h>

// Simple OpenGL Image Library
#include <SOIL.h>

// GLM Mathematics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// audio
#include "irrKlang/irrKlang.h"

// Other includes
#include "Shader.h"
#include "Camera.h"

using std::cout;
using std::endl;
using namespace irrklang;

// Function prototypes
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void do_movement(GLdouble);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void loadOurTexture(GLuint textureNum, char* imageName);
void getJumpHeight(GLdouble deltaTime);
void renderRobot(Shader &ourShader, GLuint* VAO, glm::vec3* robotPositions, GLfloat rotateAngle, glm::vec3* robotScale);
void renderFloor(Shader &ourShader, GLuint* VAO);
void renderBoxes(Shader &ourShader, GLuint* VAO);
void OnError(int errorCode, const char* msg) {
    throw std::runtime_error(msg);
}

// globals
const GLuint WIDTH = 800, HEIGHT = 600;

// Camera
Camera camera(glm::vec3(0.0f, 1.5f, 3.0f));

// Keyboard  control
bool keys[1024];

// Mouse control
bool firstMouse = true;
GLfloat lastX = 400, lastY = 300;

// Jump
GLuint jumpStatus = 0; //0 for no jump, 1 for up, 2 for down
GLfloat jumpHeight = 0.0f;
GLfloat turnAngle = 0.0f;
GLfloat jumpMax = 0.8f;
GLfloat jumpMin = 0.0f;

//box positions
glm::vec3 boxPos[3] =
    {glm::vec3(0.0f, 0.0f, 0.0f),glm::vec3(5.0f, -0.5f, 0.0f), glm::vec3(0.0f, 2.0f, 2.0f)};

//sound
ISoundEngine* engine;

//controls
bool shadowOn = true;
bool collisionOn = true;

// The MAIN function, from here we start the application and run the game loop
int main()
{
    // initialise GLFW
    glfwSetErrorCallback(OnError);
    if(!glfwInit())
        throw std::runtime_error("glfwInit failed");
    
    // open a window with GLFW
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    
    GLFWwindow* gWindow = NULL;
    gWindow = glfwCreateWindow(WIDTH, HEIGHT, "CSE167 Final Project", NULL, NULL);
    if(!gWindow)
        throw std::runtime_error("glfwCreateWindow failed. Can your hardware handle OpenGL 3.2?");
    
    // GLFW settings
    glfwMakeContextCurrent(gWindow);
    
    // Set the required callback functions
    glfwSetKeyCallback(gWindow, key_callback);
    glfwSetCursorPosCallback(gWindow, mouse_callback);
    glfwSetScrollCallback(gWindow, scroll_callback);
    // Hide mouse
    glfwSetInputMode(gWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
    // initialise GLEW
    glewExperimental = GL_TRUE; //stops glew crashing on OSX :-/
    if(glewInit() != GLEW_OK)
        throw std::runtime_error("glewInit failed");
    
    // print out some info about the graphics drivers
    std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GLSL version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
    std::cout << "Vendor: " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
    
    // make sure OpenGL version 3.2 API is available
    if(!GLEW_VERSION_3_2)
        throw std::runtime_error("OpenGL 3.2 API is not available.");
    
    // Define the viewport dimensions
    //glViewport(0, 0, WIDTH, HEIGHT);
    
    engine = createIrrKlangDevice();
    
    if (!engine)
    {
        printf("Could not startup engine\n");
        return 0; // error starting up the engine
    }
    
    engine->play2D("/Users/wei/Documents/CSE167FinalProject/FinalProject2/FinalProject/bgm.wav", true);
    engine->setSoundVolume(0.1);

        // Build and compile our shader program
    Shader ourShader("/Users/wei/Documents/CSE167FinalProject/FinalProject2/FinalProject/shader.vs", "/Users/wei/Documents/CSE167FinalProject/FinalProject2/FinalProject/shader.frag");
    Shader simpleDepthShader("/Users/wei/Documents/CSE167FinalProject/FinalProject2/FinalProject/simpleDepthShader.vs", "/Users/wei/Documents/CSE167FinalProject/FinalProject2/FinalProject/simpleDepthShader.frag");
    
    // Set up vertex data (and buffer(s)) and attribute pointers
    GLfloat planeVertices[] = {
        // Positions          // Normals         // Texture Coords
        25.0f, -0.5f, 25.0f, 0.0f, 1.0f, 0.0f, 25.0f, 0.0f,
        -25.0f, -0.5f, -25.0f, 0.0f, 1.0f, 0.0f, 0.0f, 25.0f,
        -25.0f, -0.5f, 25.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
        
        25.0f, -0.5f, 25.0f, 0.0f, 1.0f, 0.0f, 25.0f, 0.0f,
        25.0f, -0.5f, -25.0f, 0.0f, 1.0f, 0.0f, 25.0f, 25.0f,
        -25.0f, -0.5f, -25.0f, 0.0f, 1.0f, 0.0f, 0.0f, 25.0f
    };
    
    GLfloat cubeVertices[] = {
        // Back face
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, // Bottom-left
        0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f, // top-right
        0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, // bottom-right
        0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,  // top-right
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,  // bottom-left
        -0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f,// top-left
        // Front face
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom-left
        0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,  // bottom-right
        0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,  // top-right
        0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, // top-right
        -0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,  // top-left
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,  // bottom-left
        // Left face
        -0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // top-right
        -0.5f, 0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // top-left
        -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f,  // bottom-left
        -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-left
        -0.5f, -0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // bottom-right
        -0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // top-right
        // Right face
        0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // top-left
        0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-right
        0.5f, 0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // top-right
        0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,  // bottom-right
        0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,  // top-left
        0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, // bottom-left
        // Bottom face
        -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, // top-right
        0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f, // top-left
        0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,// bottom-left
        0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, // bottom-left
        -0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, // bottom-right
        -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, // top-right
        // Top face
        -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,// top-left
        0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // bottom-right
        0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, // top-right
        0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // bottom-right
        -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,// top-left
        -0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f // bottom-left
    };
    
    glm::vec3 robotPositions[] = {
        glm::vec3(0.0f, 0.9f, 0.0f),//body
        glm::vec3(0.0f, 1.4f, 0.0f),//head
        glm::vec3(0.1f, 0.3f, 0.0f),//leg1
        glm::vec3(-0.1f, 0.3f, 0.0f),//leg2
        glm::vec3(0.3f, 0.9f, 0.0f),//arm1
        glm::vec3(-0.3f, 0.9f, 0.0f)//arm2
    };
    
    glm::vec3 robotScale[] = {
        glm::vec3(0.4f, 0.6f, 0.2f),//body
        glm::vec3(0.4f, 0.4f, 0.4f),//head
        glm::vec3(0.2f, 0.6f, 0.2f),//leg1
        glm::vec3(0.2f, 0.6f, 0.2f),//leg2
        glm::vec3(0.2f, 0.6f, 0.2f),//arm1
        glm::vec3(0.2f, 0.6f, 0.2f)//arm2
    };
    
    // Generate VAO, VBO, EBO
    GLuint VAO[3], VBO[3];
    glGenVertexArrays(3, VAO);
    glGenBuffers(3, VBO);
    //glGenBuffers(1, &EBO);
    
    // cube
    glBindVertexArray(VAO[0]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);
    // Normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);
    // TexCoord attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);
    glBindVertexArray(0); // Unbind VAO
    
    // floor
    glBindVertexArray(VAO[1]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
    
    glBindVertexArray(0); // Unbind VAO
    
    
    // Load and create a texture
    GLuint texture[3];
    glGenTextures(3, texture);
    char textureName0[]="/Users/wei/Documents/CSE167FinalProject/FinalProject2/FinalProject/woodFloor.jpg";
    char textureName1[] ="/Users/wei/Documents/CSE167FinalProject/FinalProject2/FinalProject/blue.jpg";
    char textureName2[] ="/Users/wei/Documents/CSE167FinalProject/FinalProject2/FinalProject/container.jpg";
    loadOurTexture(texture[0], textureName0);
    loadOurTexture(texture[1], textureName1);
    loadOurTexture(texture[2], textureName2);
    glBindTexture(GL_TEXTURE_2D, 0); // Unbind texture when done, so we won't accidentily mess up our texture.
    
    glEnable(GL_DEPTH_TEST);
    
    // Shadow
    // Depth map
    GLuint depthMapFBO;
    glGenFramebuffers(1, &depthMapFBO);
    // Create a texture for shadow
    const GLuint SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
    GLuint depthMap;
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    GLfloat borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    // Give the texture to the frame buffer
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    // 1. Render depth of scene to texture (from light's perspective)
    // - Get light projection/view matrix.
    glm::mat4 lightProjection, lightView;
    glm::mat4 lightSpaceMatrix;
    GLfloat near_plane = 1.0f, far_plane = 7.5f;
    lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
    // Light source
    glm::vec3 lightPos(2.0f, 4.0f, 1.0f);
    lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(1.0));
    lightSpaceMatrix = lightProjection * lightView;
    
    // Matrices and vectors
    glm::mat4 model;
    //model = glm::rotate(model, -0.96f, glm::vec3(1.0f, 0.0f, 0.0f));
    glm::mat4 view;
    view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));
    glm::mat4 projection;
    projection = glm::perspective(glm::radians(camera.Zoom), (GLfloat)WIDTH / (GLfloat)HEIGHT, 0.1f, 100.0f);
    // rotation control for robot's arms and legs
    glm::vec3 lastCameraPosition = camera.Position;
    GLfloat rotateAngle = 0.0f;
    GLfloat coef = 57.0f;
    // used to calculate FPS
    GLdouble currTime, lastFrameTime = 0.0;
    // jump
   
    
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

    
    // run while the window is open
    while(!glfwWindowShouldClose(gWindow)){
        // Check if any events have been activiated (key pressed, mouse moved etc.) and call corresponding response functions
        glfwPollEvents();
        
        // FPS
        currTime = glfwGetTime();
        if (keys[GLFW_KEY_F]) {
            cout << "FPS: " << 1.0 / (currTime - lastFrameTime) << endl;
        }
        // Move
        do_movement(currTime - lastFrameTime);
        getJumpHeight(currTime - lastFrameTime);
        lastFrameTime = currTime;
        
        // Shadow part
        simpleDepthShader.Use();
        glUniformMatrix4fv(glGetUniformLocation(simpleDepthShader.Program, "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));
        
        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);
        //RenderScene(simpleDepthShader);
        // shadow
        // floor
        renderFloor(simpleDepthShader, VAO);
        // boxes
        renderBoxes(simpleDepthShader, VAO);
        //robot
        renderRobot(simpleDepthShader, VAO, robotPositions, rotateAngle, robotScale);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        
        // Normal part
        // Define the viewport dimensions
        glViewport(0, 0, WIDTH *2, HEIGHT*2);
        // Render
        // Clear the colorbuffer
        //glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // Activate shader
        ourShader.Use();
        
        // General Uniform
        view = glm::lookAt(camera.Position, camera.Position + camera.Front, camera.Up);
        projection = glm::perspective(glm::radians(camera.Zoom), (GLfloat)WIDTH / (GLfloat)HEIGHT, 0.1f, 100.0f);
        glUniformMatrix4fv(glGetUniformLocation(ourShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(ourShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniform1i(glGetUniformLocation(ourShader.Program, "shadowOn"), shadowOn);
        
        // Robot
        // calculate some parameters
        GLfloat distDiff = glm::distance(glm::vec3(camera.Position.x, 0.0, camera.Position.z), glm::vec3(lastCameraPosition.x, 0.0, lastCameraPosition.z));
        if (rotateAngle > 57 || rotateAngle < -57) {
            rotateAngle = coef;
            coef = -coef;
            engine->play2D("/Users/wei/Documents/CSE167FinalProject/FinalProject2/FinalProject/walk2.wav");
        }
        rotateAngle += distDiff * coef;
        lastCameraPosition = camera.Position;
        camera.Position.y = 1.5f + jumpHeight;
        
        // Set light uniforms
        glUniform3fv(glGetUniformLocation(ourShader.Program, "lightPos"), 1, &lightPos[0]);
        glUniform3fv(glGetUniformLocation(ourShader.Program, "viewPos"), 1, &camera.Position[0]);
        glUniformMatrix4fv(glGetUniformLocation(ourShader.Program, "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, depthMap);
        glUniform1i(glGetUniformLocation(ourShader.Program, "shadowMap"), 3);
        
        // Bind Texture
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texture[1]);
        glUniform1i(glGetUniformLocation(ourShader.Program, "ourTexture1"), 1);
        renderRobot(ourShader, VAO, robotPositions, rotateAngle, robotScale);
        
        // floor
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture[0]);
        glUniform1i(glGetUniformLocation(ourShader.Program, "ourTexture1"), 0);
        renderFloor(ourShader, VAO);
        //glDrawArraysInstanced(GL_TRIANGLES, 0, 6, 10000); // 100 triangles of 6 vertices each
        
        // Box
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, texture[2]);
        glUniform1i(glGetUniformLocation(ourShader.Program, "ourTexture1"), 2);
        renderBoxes(ourShader, VAO);
        // Others
        
        // Unbind
        glBindVertexArray(0);
        
        // Swap the screen buffers
        glfwSwapBuffers(gWindow);
    }
    // Properly de-allocate all resources once they've outlived their purpose
    glDeleteVertexArrays(2, VAO);
    glDeleteBuffers(2, VBO);
    // clean up and exit
    glfwTerminate();
    return 0;
}

// Is called whenever a key is pressed/released via GLFW
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
    if (key >= 0 && key < 1024)
    {
        if (action == GLFW_PRESS)
            keys[key] = true;
        else if (action == GLFW_RELEASE)
            keys[key] = false;
    }
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }
    
    GLfloat xoffset = xpos - lastX;
    GLfloat yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;
    
    camera.ProcessMouseMovement(xoffset, yoffset);
}

void do_movement(GLdouble deltaTime)
{
    // Record current position
    glm::vec3 currentPos = camera.Position;
    // Camera controls
    //GLfloat cameraSpeed = -5.0f * deltaTime;
    if (keys[GLFW_KEY_LEFT_SHIFT]) deltaTime *= 2.0f;
    if ((keys[GLFW_KEY_A] || keys[GLFW_KEY_D]) && (keys[GLFW_KEY_W] || keys[GLFW_KEY_S])) deltaTime /= 1.414f;
    if (keys[GLFW_KEY_W]) {
        camera.ProcessKeyboard(FORWARD, deltaTime);
        turnAngle = 0.0f;
    }
    if (keys[GLFW_KEY_S]) {
        camera.ProcessKeyboard(BACKWARD, deltaTime);
        turnAngle = 0.0f;
    }
    if (keys[GLFW_KEY_A]) {
        camera.ProcessKeyboard(LEFT, deltaTime);
        turnAngle = 90.0f;
    }
    if (keys[GLFW_KEY_D]) {
        camera.ProcessKeyboard(RIGHT, deltaTime);
        turnAngle = 90.0f;
    }
    if ((keys[GLFW_KEY_A] && keys[GLFW_KEY_S]) || (keys[GLFW_KEY_D] && keys[GLFW_KEY_W])) {
        turnAngle = 45.0f;
    }
    if ((keys[GLFW_KEY_A] && keys[GLFW_KEY_W]) || (keys[GLFW_KEY_D] && keys[GLFW_KEY_S])) {
        turnAngle = -45.0f;
    }
    if (keys[GLFW_KEY_SPACE])
        if (jumpStatus == 0) {
            jumpStatus = 1;
            engine->play2D("/Users/wei/Documents/CSE167FinalProject/FinalProject2/FinalProject/jump.wav");
        }

    jumpMax = 0.8f;
    jumpMin = 0.0f;
    
    //bool collided = false;
    // Colision Detection
    if(collisionOn)
    for (int i = 0; i < 3; i++) {
        glm::vec3 cubePos = boxPos[i] + 3.0f * camera.Front;
        glm::vec3 robotPos = camera.Position + 3.0f * camera.Front;
        if ( glm::abs(cubePos.x-robotPos.x)<0.7 && glm::abs(cubePos.z-robotPos.z)<0.7 ) {
            // change jumpmax and jumpmin
            if(boxPos[i].y > 1.6)
                jumpMax = boxPos[i].y - 1.6;
            else if(jumpHeight > 0 && boxPos[i].y < 0.8)
                jumpMin = boxPos[i].y + 1.0;
            // collision
            if(jumpHeight < boxPos[i].y + 1.0 && boxPos[i].y < 1.6) {
                //collided = true;
                camera.Position = currentPos;
                if (jumpMax < boxPos[i].y + 1.0)
                    jumpMin = 0.0;
                break;
            }
        }
    }
    
    if (keys[GLFW_KEY_1]) {
        shadowOn = !shadowOn;
    }
    if (keys[GLFW_KEY_2]) {
        collisionOn = !collisionOn;
    }
//    if(collided) {
//        camera.Position = currentPos;
//    }
//    cubePos.y = 0.0f;
//    robotPos.y = 0.0f;
//    if (glm::distance(robotPos, cubePos) < 1.0f)
//        camera.Position = currentPos;
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}

void loadOurTexture(GLuint textureNum, char* imageName)
{
    glBindTexture(GL_TEXTURE_2D, textureNum); // All upcoming GL_TEXTURE_2D operations now have effect on this texture object
    // Set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// Set texture wrapping to GL_REPEAT (usually basic wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // Set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // Load image, create texture and generate mipmaps
    int width, height;
    unsigned char* image = SOIL_load_image(imageName, &width, &height, 0, SOIL_LOAD_RGB);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    glGenerateMipmap(GL_TEXTURE_2D);
    SOIL_free_image_data(image);
    glBindTexture(GL_TEXTURE_2D, 0); // Unbind texture when done, so we won't accidentily mess up our texture.
    
}

void getJumpHeight(GLdouble deltaTime)
{
    //GLfloat heightLimit = 0.8f;
    GLfloat speed = deltaTime * 3.5f;
    if (jumpStatus == 1) {
        if (jumpHeight < jumpMax)
            jumpHeight += speed;
        else
            jumpStatus = 2;
    }
    else if (jumpStatus == 2) {
        if (jumpHeight > jumpMin)
            jumpHeight -= speed;
        else
            jumpStatus = 0;
    }
    else if (jumpHeight > jumpMin) {
            jumpStatus = 2;
    }
    else {
        jumpHeight = jumpMin;
    }
}

void renderRobot(Shader &ourShader, GLuint* VAO, glm::vec3* robotPositions, GLfloat rotateAngle, glm::vec3* robotScale)
{
    glm::vec3 tempVec = camera.Position + 3.0f * camera.Front;
    tempVec.y = camera.Position.y;
    glUniform3f(glGetUniformLocation(ourShader.Program, "cameraPosition"), 0,0,0);
    //glUniform3f(glGetUniformLocation(ourShader.Program, "cameraPosition"), tempVec.x, camera.Position.y, tempVec.z);
    // Draw robot
    glBindVertexArray(VAO[0]);
    for (GLuint i = 0; i < 6; i++) {
        glm::mat4 robotModel;
        robotModel = glm::translate(robotModel, tempVec);
        robotModel = glm::rotate(robotModel, -(camera.Yaw + 90.f + turnAngle), glm::vec3(0.0f, 1.0f, 0.0f));
        robotModel = glm::translate(robotModel, robotPositions[i]);
        robotModel = glm::translate(robotModel, glm::vec3(0.0f, -1.8f, 0.0f));
        if (i == 3 || i == 4) { //arm1 and leg2
            robotModel = glm::rotate(robotModel, rotateAngle, glm::vec3(1.0f, 0.0f, 0.0f));
        }
        else if (i == 2 || i == 5) { //arm2 and leg1
            robotModel = glm::rotate(robotModel, -rotateAngle, glm::vec3(1.0f, 0.0f, 0.0f));
        }
        robotModel = glm::translate(robotModel, glm::vec3(0.0f, -0.2f, 0.0f));
        robotModel = glm::scale(robotModel, robotScale[i]);
        glUniformMatrix4fv(glGetUniformLocation(ourShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(robotModel));
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
}

void renderFloor(Shader &ourShader, GLuint* VAO)
{
    glm::mat4 staticModel;
    glm::vec3 tempVec = 3.0f * camera.Front;
    tempVec.y = 0.0f;
    staticModel = glm::translate(staticModel, tempVec);
    glUniformMatrix4fv(glGetUniformLocation(ourShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(staticModel));
    //glUniform3f(glGetUniformLocation(ourShader.Program, "cameraPosition"), 3.0f * camera.Front.x, 0.0, 3.0f * camera.Front.z);
    // Draw floor
    glBindVertexArray(VAO[1]); // First VAO
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void renderBoxes(Shader &ourShader, GLuint* VAO)
{
    
    glm::vec3 tempVec = 3.0f * camera.Front;
    tempVec.y = 0.0f;
    for (GLuint i = 0; i < 3; i++) {
        glm::mat4 boxModel;
        boxModel = glm::translate(boxModel, tempVec);
        boxModel = glm::translate(boxModel, boxPos[i]);
        glUniformMatrix4fv(glGetUniformLocation(ourShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(boxModel));
        glBindVertexArray(VAO[0]);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
}