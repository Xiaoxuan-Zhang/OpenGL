#include <stdio.h>
#include <stdlib.h>
#include <random>
#include <chrono>
#include <string>
#include <GL/glew.h> //OpenGL Loading Library, must be loaded first. (other options: GLAD)
#include <GLFW/glfw3.h> //window and keyboard
#include <glm/glm.hpp> //3D Math library
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <shaderProgram.hpp>
#include <ImageLoader.hpp>
#include <camera.hpp>
#include <model.hpp>
#include <light.hpp>
#include <scene.hpp>
#include <geometry.hpp>

using namespace std;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}
void processInput(GLFWwindow *window);

GLFWwindow* window; //window handler

// settings
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;
bool bloom = true;
bool bloomKeyPressed = false;
float exposure = 1.0f;

// camera
Camera camera(glm::vec3(0.0f, 12.0f, 12.0f));
float lastX = (float)SCR_WIDTH / 2.0;
float lastY = (float)SCR_HEIGHT / 2.0;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

int main()
{
    glewExperimental = GL_TRUE; // for earlier version glew 1.13.0
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize glfw!");
        return -1;
    }
    //4x antialiasing
    glfwWindowHint(GLFW_SAMPLES, 4); //4 subsamples for each pixel to improve image quality
    //To use OpenGL3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_RESIZABLE,GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); //specific to MAC OS
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "openGL demo", NULL, NULL);
    if (window == NULL) {
        fprintf(stderr, "Failed to create window!");
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW!");
        return -1;
    }
    glfwSwapInterval(1);
    glClearColor(0.0f, 0.0f, 0.1f, 0.0f); //set background
    
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE); //enable keysx
    //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetErrorCallback(error_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    
    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);
    
    Scene newScene;
    newScene.screenSize(SCR_WIDTH, SCR_HEIGHT);
    newScene.loadData();
    
    Model moon = *newScene.models["moon"];
    Model rock= *newScene.models["rock"];
    Model nano = *newScene.models["nanosuit"];
    
    newScene.addLight(POINT_LIGHT, glm::vec3(0.0f, 0.5f,  1.5f), glm::vec3(2.0f, 2.0f, 2.0f));
    newScene.addLight(POINT_LIGHT, glm::vec3(-4.0f, 0.5f, -3.0f), glm::vec3(1.5f, 0.0f, 0.0f));
    newScene.addLight(POINT_LIGHT, glm::vec3(3.0f, 0.5f,  1.0f), glm::vec3(0.0f, 0.0f, 1.5f));
    newScene.addLight(POINT_LIGHT, glm::vec3(-.8f,  2.4f, -1.0f), glm::vec3(0.0f, 1.5f, 0.0f));
    
    Shader skyboxShader = *newScene.shaders["skybox"];
    Shader texShader = *newScene.shaders["simple_texture"];
    Shader envShader = *newScene.shaders["envmap"];
    Shader nanoShader = *newScene.shaders["nano"];
     
    Geometry *cube = new Cube();
    Geometry *quad = new Quad();
    Geometry *skybox = new Skybox();
    
    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        
        // input
        // -----
        processInput(window);
        
        // render
        // ------
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        
        // 1. render scene into floating point framebuffer
        // -----------------------------------------------
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 model;
        
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-3.0f, 0.0f, 0.0));
        nanoShader.use();
        nanoShader.setMat4("model", model);
        nanoShader.setMat4("view", view);
        nanoShader.setMat4("projection", projection);
        nano.draw(nanoShader);
        
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(3.0f, 2.0f, 0.0));
        texShader.use();
        texShader.setMat4("model", model);
        texShader.setMat4("view", view);
        texShader.setMat4("projection", projection);
        moon.draw(texShader);
        
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(2.0f, 9.0f, -3.0));
        model = glm::scale(model, glm::vec3(2.0f, 2.0f, 2.0));
        model = glm::rotate(model, glm::radians(45.0f), glm::vec3(0.0, 0.0, 1.0));
        envShader.use();
        envShader.setMat4("model", model);
        envShader.setMat4("view", view);
        envShader.setMat4("projection", projection);
        envShader.setVec3("cameraPos", camera.Position);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, newScene.skyboxTexture);
        cube->draw();

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0, 0.0, 0.0));
        model = glm::scale(model, glm::vec3(20.0f, 20.0f, 20.0));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0, 0.0, 0.0));
        texShader.use();
        texShader.setMat4("model", model);
        texShader.setMat4("view", view);
        texShader.setMat4("projection", projection);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, newScene.textures["brick_wall"]);
        quad->draw();
        
        //skybox
        glDepthFunc(GL_LEQUAL);
        skyboxShader.use();
        view = glm::mat4(glm::mat3(camera.GetViewMatrix()));
        skyboxShader.setMat4("view", view);
        skyboxShader.setMat4("projection", projection);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, newScene.skyboxTexture);
        skybox->draw();
        glDepthFunc(GL_LESS);
        
        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
    
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !bloomKeyPressed)
    {
        bloom = !bloom;
        bloomKeyPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE)
    {
        bloomKeyPressed = false;
    }
    
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
    {
        if (exposure > 0.0f)
            exposure -= 0.001f;
        else
            exposure = 0.0f;
    }
    else if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
    {
        exposure += 0.001f;
    }
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }
    
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top
    
    lastX = xpos;
    lastY = ypos;
    
    camera.ProcessMouseMove(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}

