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

void Scene::setScreenSize(unsigned int width, unsigned height) {
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
    int lightId = this->lights.size();
    this->lights.push_back(new Light(lightId, type, pos, color));
};

void Scene::addTexture(string name, GLuint id) {
    this->textures[name] = id;
};

unsigned int Scene::getGBuffer() {
    return this->FBOs["GBuffer"];
}

void Scene::addGBuffer() {
    unsigned int FBO;
    glGenFramebuffers(1, &FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);

    unsigned int bufferAttachments[3] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
    unsigned int posTex = addNullTexture(this->screenWidth, this->screenHeight, GL_RGB16F, GL_RGB, GL_FLOAT, GL_NEAREST, GL_REPEAT);
    unsigned int norTex = addNullTexture(this->screenWidth, this->screenHeight, GL_RGB16F, GL_RGB, GL_FLOAT, GL_NEAREST, GL_REPEAT);
    unsigned int colTex = addNullTexture(this->screenWidth, this->screenHeight, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, GL_NEAREST, GL_REPEAT);
    
    this->addTexture("G_position", posTex);
    this->addTexture("G_normal", norTex);
    this->addTexture("G_albedo_specular", colTex);

    glBindTexture(GL_TEXTURE_2D, posTex);
    glFramebufferTexture2D(GL_FRAMEBUFFER, bufferAttachments[0], GL_TEXTURE_2D, posTex, 0);
    glBindTexture(GL_TEXTURE_2D, norTex);
    glFramebufferTexture2D(GL_FRAMEBUFFER, bufferAttachments[1], GL_TEXTURE_2D, norTex, 0);
    glBindTexture(GL_TEXTURE_2D, colTex);
    glFramebufferTexture2D(GL_FRAMEBUFFER, bufferAttachments[2], GL_TEXTURE_2D, colTex, 0);
    glDrawBuffers(3, bufferAttachments);
    // create and attach depth buffer (renderbuffer)
    unsigned int rboDepth;
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, this->screenWidth, this->screenHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);

    this->FBOs["GBuffer"] = FBO;
    checkFramebufferStatus();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}



