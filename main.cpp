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
const unsigned int SCR_WIDTH = 1440;
const unsigned int SCR_HEIGHT = 900;
bool bloom = true;
bool bloomKeyPressed = false;
float exposure = 1.0f;

// camera
Camera camera(glm::vec3(0.0f, 3.0f, 9.0f));
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
    window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "my_first_example", NULL, NULL);
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
    
    newScene.addLight(POINT_LIGHT, glm::vec3(0.0f, 0.5f,  1.5f), glm::vec3(2.0f, 2.0f, 2.0f));
    newScene.addLight(POINT_LIGHT, glm::vec3(-4.0f, 0.5f, -3.0f), glm::vec3(1.5f, 0.0f, 0.0f));
    newScene.addLight(POINT_LIGHT, glm::vec3(3.0f, 0.5f,  1.0f), glm::vec3(0.0f, 0.0f, 1.5f));
    newScene.addLight(POINT_LIGHT, glm::vec3(-.8f,  2.4f, -1.0f), glm::vec3(0.0f, 1.5f, 0.0f));
    
    Shader bloomShader = *newScene.shaders["bloom"];
    Shader bloomLightShader = *newScene.shaders["bloom_lights"];
    Shader blurShader = *newScene.shaders["bloom_blur"];
    Shader finalShader = *newScene.shaders["bloom_final"];
    
    bloomShader.use();
    bloomShader.setInt("diffuseTexture", 0);
    blurShader.use();
    blurShader.setInt("image", 0);
    finalShader.use();
    finalShader.setInt("scene", 0);
    finalShader.setInt("bloomBlur", 1);
    
    Geometry *cube = new Cube();
    Geometry *quad = new Quad();
    
    // configure (floating point) framebuffers
    // ---------------------------------------
    unsigned int hdrFBO;
    glGenFramebuffers(1, &hdrFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
    // create 2 floating point color buffers (one for normal rendering, the other for brightness treshold values)
    unsigned int colorBuffers[2];
    glGenTextures(2, colorBuffers);
    for (unsigned int i = 0; i < 2; i++)
    {
        glBindTexture(GL_TEXTURE_2D, colorBuffers[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);  // we clamp to the edge as the blur filter would otherwise sample repeated texture values!
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        // attach texture to framebuffer
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorBuffers[i], 0);
    }

    // create and attach depth buffer (renderbuffer)
    unsigned int rboDepth;
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
    
    // tell OpenGL which color attachments we'll use (of this framebuffer) for rendering
    unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    glDrawBuffers(2, attachments);
    // finally check if framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        printf("Framebuffer not complete!");
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    // ping-pong-framebuffer for blurring
    unsigned int pingpongFBO[2];
    unsigned int pingpongColorbuffers[2];
    glGenFramebuffers(2, pingpongFBO);
    glGenTextures(2, pingpongColorbuffers);
    for (unsigned int i = 0; i < 2; i++)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
        glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // we clamp to the edge as the blur filter would otherwise sample repeated texture values!
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongColorbuffers[i], 0);
        // also check if framebuffers are complete (no need for depth buffer)
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            printf("Framebuffer not complete!");
    }
   
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
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // 1. render scene into floating point framebuffer
        // -----------------------------------------------
        glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 model;
        bloomShader.use();
        bloomShader.setMat4("projection", projection);
        bloomShader.setMat4("view", view);
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, newScene.textures["wood"]);
        // set lighting uniforms
        char variableStr[64];
        for (unsigned int i = 0; i < newScene.lights.size(); i++) {
            sprintf(variableStr, "lights[%d].Position", i);
            bloomShader.setVec3(variableStr, newScene.lights[i]->lightPos);
            sprintf(variableStr, "lights[%d].Color", i);
            bloomShader.setVec3(variableStr, newScene.lights[i]->lightColor);
        }
        bloomShader.setVec3("viewPos", camera.Position);
        // create one large cube that acts as the floor
        model = glm::mat4();
        model = glm::translate(model, glm::vec3(0.0f, -1.0f, 0.0));
        model = glm::scale(model, glm::vec3(12.5f, 0.5f, 12.5f));
        bloomShader.setMat4("model", model);
        cube->draw();
        
        // then create multiple cubes as the scenery
        model = glm::mat4();
        model = glm::translate(model, glm::vec3(0.0f, 1.5f, 0.0));
        model = glm::scale(model, glm::vec3(0.2f));
        bloomShader.setMat4("model", model);
        rock.Draw(bloomShader);
        
        model = glm::mat4();
        model = glm::translate(model, glm::vec3(2.0f, 0.0f, 1.0));
        model = glm::scale(model, glm::vec3(0.2f));
        bloomShader.setMat4("model", model);
        rock.Draw(bloomShader);
        
        model = glm::mat4();
        model = glm::translate(model, glm::vec3(-1.0f, -1.0f, 2.0));
        model = glm::rotate(model, glm::radians(60.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
        bloomShader.setMat4("model", model);
        rock.Draw(bloomShader);
        
        model = glm::mat4();
        model = glm::translate(model, glm::vec3(0.0f, 2.7f, 4.0));
        model = glm::rotate(model, glm::radians(23.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
        model = glm::scale(model, glm::vec3(0.2f));
        bloomShader.setMat4("model", model);
        rock.Draw(bloomShader);
        
        model = glm::mat4();
        model = glm::translate(model, glm::vec3(-2.0f, 1.0f, -3.0));
        model = glm::rotate(model, glm::radians(124.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
        model = glm::scale(model, glm::vec3(0.2f));
        bloomShader.setMat4("model", model);
        rock.Draw(bloomShader);
        
        model = glm::mat4();
        model = glm::translate(model, glm::vec3(-3.0f, 0.0f, 0.0));
        model = glm::scale(model, glm::vec3(0.5f));
        bloomShader.setMat4("model", model);
        rock.Draw(bloomShader);

        // finally show all the light sources as bright balls
        bloomLightShader.use();
        bloomLightShader.setMat4("projection", projection);
        bloomLightShader.setMat4("view", view);
        
        for (int i = 0; i < newScene.lights.size(); i++) {
            model = glm::mat4();
            model = glm::translate(model, glm::vec3(newScene.lights[i]->lightPos));
            model = glm::scale(model, glm::vec3(0.1f));
            bloomLightShader.setMat4("model", model);
            bloomLightShader.setVec3("lightColor", newScene.lights[i]->lightColor);
            moon.Draw(bloomLightShader);
        }
        glBindTexture(GL_TEXTURE_2D, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        
        // 2. blur bright fragments with two-pass Gaussian Blur
        // --------------------------------------------------
        bool horizontal = true, first_iteration = true;
        unsigned int amount = 10;
        blurShader.use();
        for (unsigned int i = 0; i < amount; i++)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);
            blurShader.setInt("horizontal", horizontal);
            // bind texture of other framebuffer (or scene if first iteration)
            glBindTexture(GL_TEXTURE_2D, first_iteration ? colorBuffers[1] : pingpongColorbuffers[!horizontal]);
            quad->draw();
            horizontal = !horizontal;
            if (first_iteration)
                first_iteration = false;
        }
        glBindTexture(GL_TEXTURE_2D, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        
        // 3. now render floating point color buffer to 2D quad and tonemap HDR colors to default framebuffer's (clamped) color range
        // --------------------------------------------------------------------------------------------------------------------------
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        finalShader.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, colorBuffers[0]);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[!horizontal]);
        finalShader.setInt("bloom", bloom);
        finalShader.setFloat("exposure", exposure);
        quad->draw();
        glBindTexture(GL_TEXTURE_2D, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        
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

