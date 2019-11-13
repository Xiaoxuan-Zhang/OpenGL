//
//  scene.cpp
//  openGL_app
//
//  Created by Zooey Zhang on 10/8/19.
//  Copyright © 2019 Zooey Zhang. All rights reserved.
//

#include "scene.hpp"

Scene::Scene() {
    
};

Scene::~Scene() {
    for (auto m : this->models) {
        delete m.second;
    }
    
    for (auto p : this->primitives) {
        delete p.second;
    }
    
    for (auto l : this->lights) {
        delete l;
    }
    
    for (auto f : this->FBOs) {
        glDeleteFramebuffers(1, &f.second);
    }
    
    for (auto r : this->RBOs) {
        glDeleteRenderbuffers(1, &r.second);
    }
};

void Scene::screenSize(unsigned int width, unsigned height) {
    this->screenWidth = width;
    this->screenHeight = height;
};

void Scene::addCamera(Camera* cam) {
    this->camera = cam;
};

Geometry* Scene::addGeometry(string name, PrimitiveType type) {
    if (type == CUBE) {
        this->primitives[name] = new Cube();
    } else if (type == QUAD) {
        this->primitives[name] = new Quad();
    } else if (type == SKYBOX) {
        this->primitives[name] = new Skybox();
    } else if (type == SPHERE) {
        this->primitives[name] = new Sphere();
    }
    return this->primitives[name];
};

Model* Scene::addModel(string name, string filePath) {
    this->models[name] = new Model(filePath.c_str());
    return this->models[name];
};

void Scene::addLight(LightType type, const glm::vec3& pos, const glm::vec3& color) {
//     lighting info
    int lightId = this->lights.size() - 1;
    this->lights.push_back(new Light(lightId, type, pos, color));
};

void Scene::addTexture(string name, GLuint id) {
    this->textures[name] = id;
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


