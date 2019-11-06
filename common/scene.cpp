//
//  scene.cpp
//  openGL_app
//
//  Created by Zooey Zhang on 10/8/19.
//  Copyright © 2019 Zooey Zhang. All rights reserved.
//

#include "scene.hpp"

void Scene::screenSize(unsigned int width, unsigned height) {
    this->screenWidth = width;
    this->screenHeight = height;
};

void Scene::loadData() {
    //shaders
    this->shaders["Bloom"] = new Shader("shaders/BloomShader/Bloom.vs", "shaders/BloomShader/Bloom.fs");
    this->shaders["Bloom_lights"] = new Shader("shaders/BloomShader/Bloom.vs", "shaders/BloomShader/Bloom_lights.fs");
    this->shaders["Bloom_blur"] = new Shader("shaders/BloomShader/Bloom_blur.vs", "shaders/BloomShader/Bloom_blur.fs");
    this->shaders["Bloom_final"] = new Shader("shaders/BloomShader/Bloom_final.vs", "shaders/BloomShader/Bloom_final.fs");

    //textures
    this->textures["wood"] = load_Texture("resources/wood.png", true); // note that we're loading the texture as an SRGB texture
    this->textures["container"] = load_Texture("resources/container2.png", true); // note that we're loading the texture as an SRGB texture

    //models
    this->models["moon"] = new Model("resources/44-moon-photorealistic-2k/Moon 2K.obj");
    this->models["rock"] = new Model("resources/rock/rock.obj");
    
};

void Scene::addLights() {
//     lighting info
    int lightId = 0;
    this->lights.push_back(new Light(lightId++, POINT_LIGHT, glm::vec3(0.0f, 0.5f,  1.5f), glm::vec3(2.0f, 2.0f, 2.0f)));
    this->lights.push_back(new Light(lightId++, POINT_LIGHT, glm::vec3(-4.0f, 0.5f, -3.0f), glm::vec3(1.5f, 0.0f, 0.0f)));
    this->lights.push_back(new Light(lightId++, POINT_LIGHT, glm::vec3(3.0f, 0.5f,  1.0f), glm::vec3(0.0f, 0.0f, 1.5f)));
    this->lights.push_back(new Light(lightId++, POINT_LIGHT, glm::vec3(-.8f,  2.4f, -1.0f), glm::vec3(0.0f, 1.5f, 0.0f)));
};

void Scene::configFrameBuffers() {
    // configure (floating point) framebuffers
    // ---------------------------------------
    glGenFramebuffers(1, &this->hdrFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, this->hdrFBO);
    // create 2 floating point color buffers (1 for normal rendering, other for brightness treshold values)
    glGenTextures(2, this->colorBuffers);
    for (unsigned int i = 0; i < 2; i++)
    {
        glBindTexture(GL_TEXTURE_2D, this->colorBuffers[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, this->screenWidth, this->screenHeight, 0, GL_RGB, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);  // we clamp to the edge as the blur filter would otherwise sample repeated texture values!
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        // attach texture to framebuffer
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, this->colorBuffers[i], 0);
    }
    // create and attach depth buffer (renderbuffer)
    glGenRenderbuffers(1, &this->rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, this->rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, this->screenWidth, this->screenHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, this->rboDepth);
    
    // tell OpenGL which color attachments we'll use (of this framebuffer) for rendering
    glDrawBuffers(2, this->attachments);
    // finally check if framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        printf("Framebuffer not complete!");
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    // ping-pong-framebuffer for blurring
    glGenFramebuffers(2, this->pingpongFBO);
    glGenTextures(2, this->pingpongColorbuffers);
    for (unsigned int i = 0; i < 2; i++)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, this->pingpongFBO[i]);
        glBindTexture(GL_TEXTURE_2D, this->pingpongColorbuffers[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, this->screenWidth, this->screenHeight, 0, GL_RGB, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // we clamp to the edge as the blur filter would otherwise sample repeated texture values!
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->pingpongColorbuffers[i], 0);
        // also check if framebuffers are complete (no need for depth buffer)
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            printf("Framebuffer not complete!");
    }
};

