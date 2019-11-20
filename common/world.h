//
//  world.h
//  openGL_app
//
//  Created by Zooey Zhang on 11/8/19.
//  Copyright © 2019 Zooey Zhang. All rights reserved.
//

#ifndef world_h
#define world_h

#include <scene.hpp>
#include <iostream>
#include "utilities.h"


/* PBR pipeline */
Scene* createPBRScene(int screenWidth, int screenHeight, Camera *camera) {
    Scene* newScene = new Scene();
    newScene->screenSize(screenWidth, screenHeight);
    newScene->addCamera(camera);

    // build and compile shaders
    newScene->shaders["pbr"] = new Shader("shaders/PBR/pbrShader.vs", "shaders/PBR/pbrShader.fs");
    newScene->shaders["cubemap"] = new Shader("shaders/cubemap.vs", "shaders/cubemap.fs"); //equirectangular skybox
    newScene->shaders["hdr_skybox"] = new Shader("shaders/hdrSkybox.vs", "shaders/hdrSkybox.fs");
    newScene->shaders["irradiance_convolution"] = new Shader("shaders/cubemap.vs", "shaders/PBR/convolution.fs");
    newScene->shaders["pbr_IBL"] = new Shader("shaders/PBR/pbrShader.vs", "shaders/PBR/IBLPbr.fs");
    newScene->shaders["pbr"] = new Shader("shaders/PBR/pbrShader.vs", "shaders/PBR/nonIBLPbr.fs");
    newScene->shaders["prefilter_envmap"] = new Shader("shaders/cubemap.vs", "shaders/PBR/prefilterEnvmap.fs");
    newScene->shaders["brdf_envmap"] = new Shader("shaders/ndc.vs", "shaders/PBR/integrateBRDF.fs");
    newScene->shaders["ndc"] = new Shader("shaders/ndc.vs", "shaders/ndc.fs");
    
    newScene->shaders["cubemap"]->use();
    newScene->shaders["cubemap"]->setInt("hdrTexture", 0);
    newScene->shaders["hdr_skybox"]->use();
    newScene->shaders["hdr_skybox"]->setInt("hdrSkybox", 0);
    newScene->shaders["irradiance_convolution"]->use();
    newScene->shaders["irradiance_convolution"]->setInt("hdrSkybox", 0);
    newScene->shaders["prefilter_envmap"]->use();
    newScene->shaders["prefilter_envmap"]->setInt("hdrSkybox", 0);
    
    // load textures
    newScene->addTexture("brick_wall", loadTexture("resources/brickwall.jpg", true));
    newScene->addTexture("pbr_albedo", loadTexture("resources/rustediron1/basecolor.png", false));
    newScene->addTexture("pbr_metallic", loadTexture("resources/rustediron1/metallic.png", false));
    newScene->addTexture("pbr_roughness", loadTexture("resources/rustediron1/roughness.png", false));
    newScene->addTexture("pbr_normal", loadTexture("resources/rustediron1/normal.png", false));
    newScene->addTexture("hdr_env", loadHDRTexture("resources/Newport_Loft/Newport_Loft_Ref.hdr"));

    // load models
    newScene->addModel("sphere", "resources/sphere.obj");

    // add primitives
    Geometry* cube = newScene->addGeometry("cube0", CUBE);
    Geometry* quad = newScene->addGeometry("quad0", QUAD);
    newScene->addGeometry("skybox", SKYBOX);

    // environent cubemap
    newScene->addTexture("env_cubemap", addNullHdrCubemapTexture(512, 512));
    // irradiance cubemap
    newScene->addTexture("irradiance_cubemap", addNullHdrCubemapTexture(32, 32));
    //enviroment mipmap
    newScene->addTexture("env_mipmap", addNullHdrCubemapMipmap(128, 128));
    // brdf LUT
    newScene->addTexture("brdf_LUT", addNullTexture(512, 512, GL_RG16F, GL_RG, GL_FLOAT));

    //render once before loop
    unsigned int FBO, RBO;
    glGenFramebuffers(1, &FBO);
    glGenRenderbuffers(1, &RBO);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    glBindRenderbuffer(GL_RENDERBUFFER, RBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, RBO);
    newScene->FBOs["fbo0"] = FBO;
    newScene->RBOs["rbo0"] = RBO;

    glm::mat4 capProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
    glm::mat4 captureViews[] = {
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
    };
    
    //convert HDR equirectangular map to cubemap
    newScene->shaders["cubemap"]->use();
    newScene->shaders["cubemap"]->setMat4("projection", capProjection);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, newScene->textures["hdr_env"]);
    glViewport(0, 0, 512, 512);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);

    for (unsigned int i = 0; i < 6; ++i) {
        newScene->shaders["cubemap"]->setMat4("view", captureViews[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, newScene->textures["env_cubemap"], 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        newScene->primitives["cube0"]->draw();
    }
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    //generate mipmaps from first mip face
    glBindTexture(GL_TEXTURE_CUBE_MAP, newScene->textures["env_cubemap"]);
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    //generate irradiance convolution as a cubemap
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    glBindRenderbuffer(GL_RENDERBUFFER, RBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 32, 32);

    newScene->shaders["irradiance_convolution"]->use();
    newScene->shaders["irradiance_convolution"]->setMat4("projection", capProjection);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, newScene->textures["env_cubemap"]);
    glViewport(0, 0, 32, 32);
    for (unsigned int i = 0; i < 6; ++i) {
        newScene->shaders["irradiance_convolution"]->setMat4("view", captureViews[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, newScene->textures["irradiance_cubemap"], 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        newScene->primitives["cube0"]->draw();
    }
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    //capture prefilter mipmap levels
    newScene->shaders["prefilter_envmap"]->use();
    newScene->shaders["prefilter_envmap"]->setMat4("projection", capProjection);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, newScene->textures["env_cubemap"]);

    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    unsigned int maxMipLevel = 5;
    for (unsigned int mip = 0; mip < maxMipLevel; ++mip) {
        //resize framebuffer according to mip level
        unsigned int mipWidth = 128 * std::pow(0.5, mip);
        unsigned int mipHeight = 128 * std::pow(0.5, mip);
        glBindRenderbuffer(GL_RENDERBUFFER, RBO);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
        glViewport(0, 0, mipWidth, mipHeight);

        float roughness = (float)mip / (float)(maxMipLevel - 1.0);
        newScene->shaders["prefilter_envmap"]->setFloat("roughness", roughness);
        for (unsigned int i = 0; i < 6; ++i) {
            newScene->shaders["prefilter_envmap"]->setMat4("view", captureViews[i]);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, newScene->textures["env_mipmap"], mip);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            newScene->primitives["cube0"]->draw();
        }
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    glBindRenderbuffer(GL_RENDERBUFFER, RBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, newScene->textures["brdf_LUT"], 0);
    glViewport(0, 0, 512, 512);
    newScene->shaders["brdf_envmap"]->use();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    newScene->primitives["quad0"]->draw();

    // always remember to put everythign back in place
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, screenWidth, screenHeight);
    return newScene;
}

void renderPBRScene(Scene* scene) {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, scene->screenWidth, scene->screenHeight);
    Camera* cam = scene->camera;
    glm::mat4 projection = glm::perspective(glm::radians(cam->Zoom), (float)scene->screenWidth/ (float)scene->screenHeight, 0.1f, 100.0f);
    glm::mat4 view = cam->GetViewMatrix();
    glm::mat4 model;

    Shader* pbr = scene->shaders["pbr_IBL"];
    pbr->use();
    pbr->setMat4("view", view);
    pbr->setMat4("projection", projection);
    pbr->setVec3("cameraPos", cam->Position);
    pbr->setInt("texture_diffuse", 0);
    pbr->setInt("texture_normal", 1);
    pbr->setInt("texture_metallic", 2);
    pbr->setInt("texture_roughness", 3);
    pbr->setInt("irradianceMap", 4);
    pbr->setInt("env_mipmap", 5);
    pbr->setInt("brdf_LUT", 6);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, scene->textures["pbr_albedo"]);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, scene->textures["pbr_normal"]);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, scene->textures["pbr_metallic"]);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, scene->textures["pbr_roughness"]);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_CUBE_MAP, scene->textures["irradiance_cubemap"]);
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_CUBE_MAP, scene->textures["env_mipmap"]);
    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_2D, scene->textures["brdf_LUT"]);

    for (int i = 0; i < 5; ++i) {
        for (int j = 0; j < 5; ++j) {
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(-10.0f + i * 6.0f, j * 6.0f, 0.0f));
            model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            pbr->setMat4("model", model);
            scene->models["sphere"]->draw(pbr);
        }
    }

    pbr = scene->shaders["pbr"];
    pbr->use();
    pbr->setMat4("view", view);
    pbr->setMat4("projection", projection);
    pbr->setVec3("cameraPos", cam->Position);
    pbr->setVec3("albedo", glm::vec3(0.2, 0.3, 0.8));
    pbr->setFloat("ao", 1.0);
    pbr->setInt("irradianceMap", 0);
    pbr->setInt("env_mipmap", 1);
    pbr->setInt("brdf_LUT", 2);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, scene->textures["irradiance_cubemap"]);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_CUBE_MAP, scene->textures["env_mipmap"]);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, scene->textures["brdf_LUT"]);

    for (int i = 5; i < 10; ++i) {
        for (int j = 0; j < 5; ++j) {
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(-10.0f + i * 6.0f, j * 6.0f, 0.0f));
            model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            pbr->setMat4("model", model);
            pbr->setFloat("metallic", 1.0 - (i - 5) * 0.2);
            pbr->setFloat("roughness", 0.1 + j * 0.2);
            scene->models["sphere"]->draw(pbr);
        }
    }

    //skybox
    Shader* skyShader = scene->shaders["hdr_skybox"];
    glDepthFunc(GL_LEQUAL);
    skyShader->use();
    skyShader->setMat4("view", view);
    skyShader->setMat4("projection", projection);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, scene->textures["env_cubemap"]);
    scene->primitives["skybox"]->draw();
    glDepthFunc(GL_LESS);
}

/* reflection */
Scene* reflectionScene(int screenWidth, int screenHeight, Camera *camera) {
    Scene* newScene = new Scene();
    newScene->screenSize(screenWidth, screenHeight);
    newScene->addCamera(camera);
    camera->Position = glm::vec3(-5.0, 10.0f, 12.0f);
    // build and compile shaders
    newScene->shaders["skybox"] = new Shader("shaders/skybox.vs", "shaders/skybox.fs");
    newScene->shaders["simple_texture"] = new Shader("shaders/simpleTexture.vs", "shaders/simpleTexture.fs");
    newScene->shaders["envmap"] = new Shader("shaders/envmap.vs", "shaders/envmap.fs");
    newScene->shaders["nano"] = new Shader("shaders/modelShader.vs", "shaders/nanoShader.fs");
    newScene->shaders["hdr_skybox"] = new Shader("shaders/hdrSkybox.vs", "shaders/hdrSkybox.fs");
    newScene->shaders["cubemap"] = new Shader("shaders/cubemap.vs", "shaders/cubemap.fs"); //equirectangular skybox
    newScene->shaders["hdr_skybox"] = new Shader("shaders/hdrSkybox.vs", "shaders/hdrSkybox.fs");
    
    newScene->shaders["hdr_skybox"]->use();
    newScene->shaders["hdr_skybox"]->setInt("hdrSkybox", 0);
    newScene->shaders["skybox"]->use();
    newScene->shaders["skybox"]->setInt("skybox", 0);
    newScene->shaders["envmap"]->use();
    newScene->shaders["envmap"]->setInt("skybox", 0);
    newScene->shaders["simple_texture"]->use();
    newScene->shaders["simple_texture"]->setInt("texture_diffuse", 0);
    newScene->shaders["cubemap"]->use();
    newScene->shaders["cubemap"]->setInt("hdrTexture", 0);
    
    // load textures
    newScene->addTexture("skybox", loadCubemap("resources/mp_blood/", "tga"));
    newScene->addTexture("brick_wall", loadTexture("resources/brickwall.jpg", true));
    newScene->addTexture("hdr_env", loadHDRTexture("resources/Alexs_Apartment/Alexs_Apt_2k.hdr"));
    // environent cubemap
    newScene->addTexture("env_cubemap", addNullHdrCubemapTexture(512, 512));
    
    // load models
    newScene->addModel("nanosuit", "resources/nanosuit/nanosuit.obj");
    newScene->addModel("sphere", "resources/sphere.obj");
    
    // add primitives
    Geometry* cube = newScene->addGeometry("cube0", CUBE);
    Geometry* quad = newScene->addGeometry("quad0", QUAD);
    newScene->addGeometry("skybox", SKYBOX);
    
    // add lights
    newScene->addLight(POINT_LIGHT, glm::vec3(0.0f, 0.5f,  1.5f), glm::vec3(2.0f, 2.0f, 2.0f));
    newScene->addLight(POINT_LIGHT, glm::vec3(-4.0f, 0.5f, -3.0f), glm::vec3(1.5f, 0.0f, 0.0f));
    newScene->addLight(POINT_LIGHT, glm::vec3(3.0f, 0.5f,  1.0f), glm::vec3(0.0f, 0.0f, 1.5f));
    newScene->addLight(POINT_LIGHT, glm::vec3(-.8f,  2.4f, -1.0f), glm::vec3(0.0f, 1.5f, 0.0f));
    
    //render once before loop
    unsigned int FBO, RBO;
    glGenFramebuffers(1, &FBO);
    glGenRenderbuffers(1, &RBO);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    glBindRenderbuffer(GL_RENDERBUFFER, RBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, RBO);
    newScene->FBOs["fbo0"] = FBO;
    newScene->RBOs["rbo0"] = RBO;
    
    glm::mat4 capProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
    glm::mat4 captureViews[] = {
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
    };
    
    //convert HDR equirectangular map to cubemap
    newScene->shaders["cubemap"]->use();
    newScene->shaders["cubemap"]->setMat4("projection", capProjection);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, newScene->textures["hdr_env"]);
    glViewport(0, 0, 512, 512);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    
    for (unsigned int i = 0; i < 6; ++i) {
        newScene->shaders["cubemap"]->setMat4("view", captureViews[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, newScene->textures["env_cubemap"], 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        newScene->primitives["cube0"]->draw();
    }
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    // always remember to put everythign back in place
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return newScene;
}

void renderReflectionScene(Scene* scene) {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, scene->screenWidth, scene->screenHeight);
    Camera* cam = scene->camera;
    glm::mat4 projection = glm::perspective(glm::radians(cam->Zoom), (float)scene->screenWidth/ (float)scene->screenHeight, 0.1f, 100.0f);
    glm::mat4 view = cam->GetViewMatrix();
    glm::mat4 model;
    
    Shader* shader = scene->shaders["nano"];
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-5.0f, -2.0f, 0.0));
    model = glm::rotate(model, glm::radians(-45.0f), glm::vec3(0.0f, 1.0f, 0.0));
    shader->use();
    shader->setMat4("model", model);
    shader->setMat4("view", view);
    shader->setMat4("projection", projection);
    shader->setVec3("cameraPos", cam->Position);
    shader->setInt("cubemap", 10);
    glActiveTexture(GL_TEXTURE10);
    glBindTexture(GL_TEXTURE_CUBE_MAP, scene->textures["env_cubemap"]);
    scene->models["nanosuit"]->draw(shader);
    
    shader = scene->shaders["envmap"];
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(5.0f, 10.0f, 0.0));
    model = glm::rotate(model, glm::radians(45.0f), glm::vec3(0.0f, 0.0f, 1.0));
    model = glm::scale(model, glm::vec3(2.0f, 2.0f, 2.0));
    shader->use();
    shader->setMat4("model", model);
    shader->setMat4("view", view);
    shader->setMat4("projection", projection);
    shader->setVec3("cameraPos", cam->Position);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, scene->textures["env_cubemap"]);
    scene->primitives["cube0"]->draw();
    
    //skybox
    Shader* skyShader = scene->shaders["hdr_skybox"];
    glDepthFunc(GL_LEQUAL);
    skyShader->use();
    skyShader->setMat4("view", view);
    skyShader->setMat4("projection", projection);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, scene->textures["env_cubemap"]);
    scene->primitives["skybox"]->draw();
    glDepthFunc(GL_LESS);
}

/*
 Shadows:
 1. Choose orthogonal projection if the light source is a directional light, perspective projection if the light source is point light or spot light.
 2. There might be various of ways to improve the jagged shadow.
 */
Scene* shadowScene(int screenWidth, int screenHeight, Camera *camera) {
    Scene* newScene = new Scene();
    newScene->screenSize(screenWidth, screenHeight);
    newScene->addCamera(camera);
    camera->Position = glm::vec3(-5.0, 5.0f, 5.0f);
    
    // build and compile shaders
    newScene->shaders["house"] = new Shader("shaders/modelShader.vs", "shaders/houseShader.fs");
    newScene->shaders["depthmap"] = new Shader("shaders/depthMap.vs", "shaders/depthMap.fs");
    newScene->shaders["depthmap_vis"] = new Shader("shaders/depthMapVis.vs", "shaders/depthMapVis.fs");
    
    newScene->shaders["depthmap_vis"]->use();
    newScene->shaders["depthmap_vis"]->setInt("depthMap", 0);
    
    // load textures
    newScene->addTexture("container", loadTexture("resources/container2.png", false));
    newScene->addTexture("wood", loadTexture("resources/wood.png", false));
    newScene->addTexture("depthmap", addNullDepthTexture(1024, 1024));
    newScene->addTexture("colormap", addNullTexture(1024, 1024));
    
    // load models
    newScene->addModel("nanosuit", "resources/nanosuit/nanosuit.obj");
    newScene->addModel("moon", "resources/44-moon-photorealistic-2k/Moon 2K.obj");
    
    // add primitives
    Geometry* cube = newScene->addGeometry("cube0", CUBE);
    Geometry* quad = newScene->addGeometry("quad0", QUAD);
    
    unsigned int FBO;
    glGenFramebuffers(1, &FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    newScene->FBOs["depthFBO"] = FBO;
    //attach depth texture to this framebuffer's depth buffer
    //glBindTexture(GL_TEXTURE_2D, newScene->textures["colormap"]);
    glBindTexture(GL_TEXTURE_2D, newScene->textures["depthmap"]);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, newScene->textures["depthmap"], 0);
    //glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, newScene->textures["colormap"], 0);
    //since only depth info for this texture, we explicitly tell OpenGL we don't need to draw color buffer
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    checkGLErrors();
    return newScene;
}

void renderShadowScene(Scene* scene) {
    Camera* cam = scene->camera;
    float nearPlane = 0.1f;
    float farPlane = 100.0f;
    glm::mat4 projection = glm::perspective(glm::radians(cam->Zoom), (float)scene->screenWidth/ (float)scene->screenHeight, 0.1f, 100.0f);
    glm::mat4 view = cam->GetViewMatrix();
    glm::vec3 lightPosition = glm::vec3(-5.0f, 15.0f, 10.0f);
    glm::mat4 lightProjection = glm::ortho(-20.0f, 20.0f, -10.0f, 30.0f, nearPlane, farPlane);
    //    //glm::mat4 lightProjection = glm::perspective(glm::radians(90.0f), 1.0f, nearPlane, farPlane);
    glm::mat4 lightView = glm::lookAt(lightPosition, glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 model;
    
    // first pass for depth map
    glBindFramebuffer(GL_FRAMEBUFFER, scene->FBOs["depthFBO"]);
    glViewport(0, 0, 1024, 1024);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    Shader* shader = scene->shaders["depthmap"];
    shader->use();
    shader->setMat4("lightSpaceMatrix", lightProjection * lightView);
    
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0, 2.0, -2.0));
    shader->setMat4("model", model);
    scene->primitives["cube0"]->draw();
    
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(3.0, 0.0, 0.0));
    model = glm::scale(model, glm::vec3(0.5, 0.5, 0.5));
    shader->setMat4("model", model);
    scene->models["nanosuit"]->draw(shader);
    
    model = glm::mat4(1.0f);
    model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0, 0.0, 0.0));
    model = glm::scale(model, glm::vec3(30.0, 30.0, 30.0));
    shader->setMat4("model", model);
    scene->primitives["quad0"]->draw();

    checkGLErrors();
    checkFramebufferStatus();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    // second pass
    glViewport(0, 0, scene->screenWidth, scene->screenHeight);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    shader = scene->shaders["house"];
    shader->use();
    shader->setInt("depthMap", 5);
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, scene->textures["depthmap"]);
    shader->setMat4("view", view);
    shader->setMat4("projection", projection);
    shader->setVec3("cameraPos", cam->Position);
    shader->setVec3("lightPos", lightPosition);
    shader->setMat4("lightSpaceMatrix", lightProjection * lightView);
    
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0, 2.0, -2.0));
    shader->setMat4("model", model);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, scene->textures["container"]);
    scene->primitives["cube0"]->draw();
    
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(3.0, 0.0, 0.0));
    model = glm::scale(model, glm::vec3(0.5, 0.5, 0.5));
    shader->setMat4("model", model);
    scene->models["nanosuit"]->draw(shader);
    
    model = glm::mat4(1.0f);
    model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0, 0.0, 0.0));
    model = glm::scale(model, glm::vec3(30.0, 30.0, 30.0));
    shader->setMat4("model", model);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, scene->textures["wood"]);
    scene->primitives["quad0"]->draw();
}
/* complex scene */
Scene* complexScene(int screenWidth, int screenHeight, Camera *camera) {
    Scene* newScene = new Scene();
    newScene->screenSize(screenWidth, screenHeight);
    newScene->addCamera(camera);
    camera->Position = glm::vec3(3.0f, 4.0f, -10.0f);
    
    // build and compile shaders
    newScene->shaders["skybox"] = new Shader("shaders/skybox.vs", "shaders/skybox.fs");
    newScene->shaders["house"] = new Shader("shaders/modelShader.vs", "shaders/houseShader.fs");
    newScene->shaders["depthmap"] = new Shader("shaders/depthMap.vs", "shaders/depthMap.fs");
    newScene->shaders["depthmap_vis"] = new Shader("shaders/depthMapVis.vs", "shaders/depthMapVis.fs");
    
    newScene->shaders["skybox"]->use();
    newScene->shaders["skybox"]->setInt("skybox", 0);
    newScene->shaders["depthmap_vis"]->use();
    newScene->shaders["depthmap_vis"]->setInt("depthMap", 0);
    
    // load textures
    newScene->addTexture("skybox", loadCubemap("resources/hw_nightsky/", "tga"));
    newScene->addTexture("depthmap", addNullDepthTexture(1024, 1024));
    newScene->addTexture("colormap", addNullTexture(1024, 1024));
    newScene->addTexture("wood", loadTexture("resources/wood.png", true));
    
    // load models
    newScene->addModel("building", "resources/building/Street environment_V01.obj");
    newScene->addModel("nanosuit", "resources/nanosuit/nanosuit.obj");
    
    // add primitives
    Geometry* cube = newScene->addGeometry("cube0", CUBE);
    Geometry* quad = newScene->addGeometry("quad0", QUAD);
    newScene->addGeometry("skybox", SKYBOX);
    
    unsigned int FBO;
    glGenFramebuffers(1, &FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    newScene->FBOs["depthFBO"] = FBO;
    //attach depth texture to this framebuffer's depth buffer
    //glBindTexture(GL_TEXTURE_2D, newScene->textures["colormap"]);
    glBindTexture(GL_TEXTURE_2D, newScene->textures["depthmap"]);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, newScene->textures["depthmap"], 0);
    //glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, newScene->textures["colormap"], 0);
    //since only depth info for this texture, we explicitly tell OpenGL we don't need to draw color buffer
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    checkGLErrors();
    return newScene;
}

void renderComplexScene(Scene* scene) {
    Camera* cam = scene->camera;
    float nearPlane = 0.1f;
    float farPlane = 50.0;
    glm::mat4 projection = glm::perspective(glm::radians(cam->Zoom), (float)scene->screenWidth/ (float)scene->screenHeight, 0.1f, 500.0f);
    glm::mat4 view = cam->GetViewMatrix();
    glm::vec3 lightPosition = glm::vec3(5.0f, 4.0f, 5.0f);
    glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, nearPlane, farPlane);
    //glm::mat4 lightProjection = glm::perspective(glm::radians(90.0f), 1.0f, nearPlane, farPlane);
    glm::mat4 lightView = glm::lookAt(lightPosition, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 model;
    
    // first pass for depth map
    glViewport(0, 0, 1024, 1024);
    glBindFramebuffer(GL_FRAMEBUFFER, scene->FBOs["depthFBO"]);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    Shader* shader = scene->shaders["depthmap"];
    shader->use();
    shader->setMat4("lightSpaceMatrix", lightProjection * lightView);
    
    model = glm::mat4(1.0f);
    model = glm::scale(model, glm::vec3(2.0, 2.0, 2.0));
    shader->setMat4("model", model);
    scene->models["building"]->draw(shader);
    
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0, 0.1, 0.0));
    model = glm::rotate(model, glm::radians(-45.0f), glm::vec3(0.0, 1.0, 0.0));
    model = glm::scale(model, glm::vec3(0.2, 0.2, 0.2));
    shader->setMat4("model", model);
    scene->models["nanosuit"]->draw(shader);
    
    checkGLErrors();
    checkFramebufferStatus();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    // second pass
    glViewport(0, 0, scene->screenWidth, scene->screenHeight);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    shader = scene->shaders["house"];
    shader->use();
    shader->setInt("depthMap", 5);
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, scene->textures["depthmap"]);
    shader->setMat4("view", view);
    shader->setMat4("projection", projection);
    shader->setVec3("cameraPos", cam->Position);
    shader->setVec3("lightPos", lightPosition);
    shader->setMat4("lightSpaceMatrix", lightProjection * lightView);

    model = glm::mat4(1.0f);
    model = glm::scale(model, glm::vec3(2.0, 2.0, 2.0));
    shader->setMat4("model", model);
    scene->models["building"]->draw(shader);

    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0, 0.1, 0.0));
    model = glm::rotate(model, glm::radians(-45.0f), glm::vec3(0.0, 1.0, 0.0));
    model = glm::scale(model, glm::vec3(0.2, 0.2, 0.2));
    shader->setMat4("model", model);
    scene->models["nanosuit"]->draw(shader);

    //skybox
    glDepthFunc(GL_LEQUAL);
    Shader* skyShader = scene->shaders["skybox"];
    skyShader->use();
    skyShader->setMat4("view", view);
    skyShader->setMat4("projection", projection);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, scene->textures["skybox"]);
    scene->primitives["skybox"]->draw();
    glDepthFunc(GL_LESS);
    
//    glViewport(0, 0, scene->screenWidth, scene->screenHeight);
//    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//    Shader* quadShader = scene->shaders["depthmap_vis"];
//    quadShader->use();
//    quadShader->setFloat("near_plane", nearPlane);
//    quadShader->setFloat("far_plane", farPlane);
//    glActiveTexture(GL_TEXTURE0);
//    glBindTexture(GL_TEXTURE_2D, scene->textures["depthmap"]);
//    scene->primitives["quad0"]->draw();
}

#endif /* world_h */
