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

/* PBR pipeline */
Scene* createPBRScene(int screenWidth, int screenHeight, Camera *camera) {
    Scene* newScene = new Scene();
    newScene->setScreenSize(screenWidth, screenHeight);
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
    newScene->setScreenSize(screenWidth, screenHeight);
    newScene->addCamera(camera);
    camera->Position = glm::vec3(-8.0, 10.0f, 12.0f);
    // build and compile shaders
    newScene->shaders["skybox"] = new Shader("shaders/skybox.vs", "shaders/skybox.fs");
    newScene->shaders["simple_texture"] = new Shader("shaders/simpleTexture.vs", "shaders/simpleTexture.fs");
    newScene->shaders["envmap"] = new Shader("shaders/envmap.vs", "shaders/envmap.fs");
    newScene->shaders["nano"] = new Shader("shaders/modelShader.vs", "shaders/nanoShader.fs");
    newScene->shaders["hdr_skybox"] = new Shader("shaders/hdrSkybox.vs", "shaders/hdrSkybox.fs");
    newScene->shaders["cubemap"] = new Shader("shaders/cubemap.vs", "shaders/cubemap.fs"); //equirectangular skybox
    
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
    newScene->addGeometry("cube0", CUBE);
    newScene->addGeometry("quad0", QUAD);
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
    model = glm::translate(model, glm::vec3(-1.0f, -2.0f, 0.0));
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
    
//    shader = scene->shaders["envmap"];
//    model = glm::mat4(1.0f);
//    model = glm::translate(model, glm::vec3(-5.0f, 5.0f, 0.0));
//    model = glm::rotate(model, glm::radians(30.0f), glm::vec3(0.0f, 0.0f, 1.0));
//    model = glm::scale(model, glm::vec3(2.0f, 2.0f, 2.0));
//    shader->use();
//    shader->setMat4("model", model);
//    shader->setMat4("view", view);
//    shader->setMat4("projection", projection);
//    shader->setVec3("cameraPos", cam->Position);
//    glActiveTexture(GL_TEXTURE0);
//    glBindTexture(GL_TEXTURE_CUBE_MAP, scene->textures["env_cubemap"]);
//    scene->primitives["cube0"]->draw();
    
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
    newScene->setScreenSize(screenWidth, screenHeight);
    newScene->addCamera(camera);
    camera->Position = glm::vec3(-5.0, 5.0f, 5.0f);
    
    // build and compile shaders
    newScene->shaders["shadow"] = new Shader("shaders/Shadows/shadowMapping.vs", "shaders/Shadows/shadowMapping.fs");
    newScene->shaders["depthmap"] = new Shader("shaders/Shadows/depthMap.vs", "shaders/Shadows/depthMap.fs");
    newScene->shaders["depthmap_vis"] = new Shader("shaders/Shadows/depthMapVis.vs", "shaders/Shadows/depthMapVis.fs");
    newScene->shaders["skybox"] = new Shader("shaders/skybox.vs", "shaders/skybox.fs");
    
    newScene->shaders["depthmap_vis"]->use();
    newScene->shaders["depthmap_vis"]->setInt("depthMap", 0);
    newScene->shaders["skybox"]->use();
    newScene->shaders["skybox"]->setInt("skybox", 0);
    
    // load textures
    newScene->addTexture("skybox", loadCubemap("resources/hw_nightsky/", "tga"));
    newScene->addTexture("container", loadTexture("resources/container2.png", true));
    newScene->addTexture("brick", loadTexture("resources/brickwall.jpg", true));
    newScene->addTexture("brick_normal", loadTexture("resources/brickwall_normal.jpg", false));
    newScene->addTexture("depthmap", addNullDepthTexture(1024, 1024));
    newScene->addTexture("colormap", addNullTexture(1024, 1024));
    
    // load models
    newScene->addModel("nanosuit", "resources/nanosuit/nanosuit.obj");
    newScene->addModel("moon", "resources/44-moon-photorealistic-2k/Moon 2K.obj");
    
    // add primitives
    newScene->addGeometry("cube0", CUBE);
    newScene->addGeometry("quad0", QUAD);
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
    glm::mat4 cubeTransform = glm::mat4(1.0f), nanoTransform = glm::mat4(1.0f), quadTransform = glm::mat4(1.0f);
    cubeTransform = glm::translate(cubeTransform, glm::vec3(0.0, 2.0, -2.0));
    
    nanoTransform = glm::translate(nanoTransform, glm::vec3(3.0, 0.0, 0.0));
    nanoTransform = glm::scale(nanoTransform, glm::vec3(0.5, 0.5, 0.5));
    
    quadTransform = glm::rotate(quadTransform, glm::radians(-90.0f), glm::vec3(1.0, 0.0, 0.0));
    quadTransform = glm::scale(quadTransform, glm::vec3(30.0, 30.0, 30.0));
    
    // first pass for depth map
    glBindFramebuffer(GL_FRAMEBUFFER, scene->FBOs["depthFBO"]);
    glViewport(0, 0, 1024, 1024);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    Shader* shader = scene->shaders["depthmap"];
    shader->use();
    shader->setMat4("lightSpaceMatrix", lightProjection * lightView);
    
    shader->setMat4("model", cubeTransform);
    scene->primitives["cube0"]->draw();
    
    shader->setMat4("model", nanoTransform);
    scene->models["nanosuit"]->draw(shader);
    
    shader->setMat4("model", quadTransform);
    scene->primitives["quad0"]->draw();

    checkGLErrors();
    checkFramebufferStatus();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    // second pass
    glViewport(0, 0, scene->screenWidth, scene->screenHeight);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    shader = scene->shaders["shadow"];
    shader->use();
    shader->setInt("depthMap", 5);
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, scene->textures["depthmap"]);
    shader->setMat4("view", view);
    shader->setMat4("projection", projection);
    shader->setVec3("cameraPos", cam->Position);
    shader->setVec3("lightPos", lightPosition);
    shader->setMat4("lightSpaceMatrix", lightProjection * lightView);
    
    shader->setMat4("model", cubeTransform);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, scene->textures["container"]);
    scene->primitives["cube0"]->draw();
    
    shader->setMat4("model", nanoTransform);
    scene->models["nanosuit"]->draw(shader);
    
    shader->setMat4("model", quadTransform);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, scene->textures["brick"]);
    scene->primitives["quad0"]->draw();
    
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
    
}

/* complex scene with shadow for single light */
Scene* complexScene(int screenWidth, int screenHeight, Camera *camera) {
    Scene* newScene = new Scene();
    newScene->setScreenSize(screenWidth, screenHeight);
    newScene->addCamera(camera);
    camera->Position = glm::vec3(3.0f, 4.0f, 10.0f);
    
    // build and compile shaders
    newScene->shaders["skybox"] = new Shader("shaders/skybox.vs", "shaders/skybox.fs");
    newScene->shaders["house"] = new Shader("shaders/Shadows/shadowMapping.vs", "shaders/Shadows/shadowMapping.fs");
    newScene->shaders["depthmap"] = new Shader("shaders/Shadows/depthMap.vs", "shaders/Shadows/depthMap.fs");
    newScene->shaders["depthmap_vis"] = new Shader("shaders/Shadows/depthMapVis.vs", "shaders/Shadows/depthMapVis.fs");
    
    newScene->shaders["skybox"]->use();
    newScene->shaders["skybox"]->setInt("skybox", 0);
    newScene->shaders["depthmap_vis"]->use();
    newScene->shaders["depthmap_vis"]->setInt("depthMap", 0);
    
    // load textures
    newScene->addTexture("skybox", loadCubemap("resources/hw_nightsky/", "tga"));
    newScene->addTexture("depthmap", addNullDepthTexture(1024, 1024));
    newScene->addTexture("wood", loadTexture("resources/wood.png", true));
    
    // load models
    newScene->addModel("building", "resources/building/Street environment_V01.obj");
    newScene->addModel("nanosuit", "resources/nanosuit/nanosuit.obj");
    
    // add primitives
    newScene->addGeometry("cube", CUBE);
    newScene->addGeometry("quad", QUAD);
    newScene->addGeometry("skybox", SKYBOX);
    
    unsigned int FBO;
    glGenFramebuffers(1, &FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    newScene->FBOs["depthFBO"] = FBO;
    //attach depth texture to this framebuffer's depth buffer
    glBindTexture(GL_TEXTURE_2D, newScene->textures["depthmap"]);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, newScene->textures["depthmap"], 0);
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
    
    glm::mat4 buildingTransform = glm::mat4(1.0f), nanoTransform = glm::mat4(1.0f);
    buildingTransform = glm::scale(buildingTransform, glm::vec3(2.0, 2.0, 2.0));
    
    nanoTransform = glm::translate(nanoTransform, glm::vec3(0.0, 0.1, 0.0));
    nanoTransform = glm::rotate(nanoTransform, glm::radians(-45.0f), glm::vec3(0.0, 1.0, 0.0));
    nanoTransform = glm::scale(nanoTransform, glm::vec3(0.2, 0.2, 0.2));
    
    // first pass for shadow map
    glViewport(0, 0, 1024, 1024);
    glBindFramebuffer(GL_FRAMEBUFFER, scene->FBOs["depthFBO"]);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    Shader* shader = scene->shaders["depthmap"];
    shader->use();
    shader->setMat4("lightSpaceMatrix", lightProjection * lightView);
    
    shader->setMat4("model", buildingTransform);
    scene->models["building"]->draw(shader);
    
    shader->setMat4("model", nanoTransform);
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

    shader->setMat4("model", buildingTransform);
    scene->models["building"]->draw(shader);
    
    shader->setMat4("model", nanoTransform);
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
//    scene->primitives["quad"]->draw();
}

/*deferred lighting with multiple lights*/
Scene* deferredScene(int screenWidth, int screenHeight, Camera *camera) {
    Scene* newScene = new Scene();
    newScene->setScreenSize(screenWidth, screenHeight);
    newScene->addCamera(camera);
    camera->Position = glm::vec3(3.0f, 4.0f, 10.0f);
    
    // build and compile shaders
    newScene->shaders["GBuffer"] = new Shader("shaders/DeferredLighting/GBuffer.vs", "shaders/DeferredLighting/GBuffer.fs");
    newScene->shaders["GBuffer_normal"] = new Shader("shaders/DeferredLighting/GBuffer_normal.vs", "shaders/DeferredLighting/GBuffer.fs");
    newScene->shaders["lighting"] = new Shader("shaders/DeferredLighting/deferredLighting.vs", "shaders/DeferredLighting/deferredLighting.fs");
    newScene->shaders["lights"] = new Shader("shaders/DeferredLighting/lights.vs", "shaders/DeferredLighting/lights.fs");
    newScene->shaders["skybox"] = new Shader("shaders/skybox.vs", "shaders/skybox.fs");
    newScene->shaders["blur"] = new Shader("shaders/DeferredLighting/Bloom_final.vs", "shaders/DeferredLighting/Bloom_blur.fs");
    newScene->shaders["bloom"] = new Shader("shaders/DeferredLighting/Bloom_final.vs", "shaders/DeferredLighting/Bloom_final.fs");
    
    newScene->shaders["skybox"]->use();
    newScene->shaders["skybox"]->setInt("skybox", 0);
    
    newScene->shaders["lighting"]->use();
    newScene->shaders["lighting"]->setInt("texure_gPosition", 0);
    newScene->shaders["lighting"]->setInt("texure_gNormal", 1);
    newScene->shaders["lighting"]->setInt("texture_gAlbedoSpec", 2);
    
    // load textures
    newScene->addTexture("skybox", loadCubemap("resources/hw_nightsky/", "tga"));
    
    // load models
    newScene->addModel("building", "resources/building/Street environment_V01.obj");
    newScene->addModel("nanosuit", "resources/nanosuit/nanosuit.obj");
    newScene->addModel("sphere", "resources/sphere_lowpoly.obj");
    
    // add primitives
    newScene->addGeometry("cube", CUBE);
    newScene->addGeometry("quad", QUAD);
    newScene->addGeometry("skybox", SKYBOX);
    
    // add random lights
    for (int i = 0; i < 32; ++i) {
        float x = randomFloat();
        float y = randomFloat();
        float z = randomFloat();
        newScene->addLight(POINT_LIGHT, glm::vec3(x * 15.0, y * 3.0, z * 15.0 - 5.0), glm::vec3(2.5 + randomFloat() * 2.5, 2.5 + randomFloat() * 2.5, 2.5 + randomFloat() * 2.5));
    }
    
    // add HDR color buffer and brightness buffer
    newScene->addTexture("hdr_color", addNullTexture(screenWidth, screenHeight, GL_RGB16F, GL_RGB, GL_FLOAT));
    newScene->addTexture("hdr_bright", addNullTexture(screenWidth, screenHeight, GL_RGB16F, GL_RGB, GL_FLOAT));
    
    unsigned int bufferAttachments[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    unsigned int hdrFBO;
    glGenFramebuffers(1, &hdrFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
    newScene->FBOs["hdr"] = hdrFBO;
    glBindTexture(GL_TEXTURE_2D, newScene->textures["hdr_color"]);
    glFramebufferTexture2D(GL_FRAMEBUFFER, bufferAttachments[0], GL_TEXTURE_2D, newScene->textures["hdr_color"], 0);
    
    glBindTexture(GL_TEXTURE_2D, newScene->textures["hdr_bright"]);
    glFramebufferTexture2D(GL_FRAMEBUFFER, bufferAttachments[1], GL_TEXTURE_2D, newScene->textures["hdr_bright"], 0);
    glDrawBuffers(2, bufferAttachments);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    // add G buffer for deferred lighting
    newScene->addGBuffer();
    
    // add pingpong buffers for blur effect
    newScene->addPingpongBuffer();
    
    checkGLErrors();
    return newScene;
}

void renderDeferredScene(Scene* scene) {
    Camera* cam = scene->camera;
    float nearPlane = 0.1f;
    float farPlane = 100.0f;
    glm::mat4 view = cam->GetViewMatrix();
    glm::mat4 projection = glm::perspective(glm::radians(cam->Zoom), (float)scene->screenWidth/ (float)scene->screenHeight, nearPlane, farPlane);
    
    glm::mat4 buildingTransform = glm::mat4(1.0f);
    buildingTransform = glm::scale(buildingTransform, glm::vec3(2.0, 2.0, 2.0));
    vector<glm::mat4> nanoMatrix;
    for (int i = 0; i < 5; ++i) {
        for (int j = 0; j < 5; ++j) {
            glm::mat4 model = glm::mat4(1.0);
            model = glm::translate(model, glm::vec3(3.0 * i, 0.1, 3.0 * j));
            model = glm::rotate(model, glm::radians(-15.0f * i + 5.0f * j), glm::vec3(0.0, 1.0, 0.0));
            model = glm::scale(model, glm::vec3(0.2, 0.2, 0.2));
            nanoMatrix.push_back(model);
        }
    }
    
    // first pass for G buffer
    glViewport(0, 0, scene->screenWidth, scene->screenHeight);
    glBindFramebuffer(GL_FRAMEBUFFER, scene->getGBuffer());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    Shader* shader = scene->shaders["GBuffer"];
    shader->use();
    shader->setMat4("projection", projection);
    shader->setMat4("view", view);

    shader->setMat4("model", buildingTransform);
    scene->models["building"]->draw(shader);

    shader = scene->shaders["GBuffer_normal"];
    shader->use();
    shader->setMat4("projection", projection);
    shader->setMat4("view", view);
    
    for (int i = 0; i < nanoMatrix.size(); ++i) {
        shader->setMat4("model", nanoMatrix[i]);
        scene->models["nanosuit"]->draw(shader);
    }

    checkGLErrors();
    checkFramebufferStatus();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    // second pass for color and bright buffer
    //glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, scene->FBOs["hdr"]);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    Shader* quadShader = scene->shaders["lighting"];
    quadShader->use();
    quadShader->setVec3("cameraPos", cam->Position);

    for (int i = 0; i < 32; ++i) {
        quadShader->setVec3(("lights[" + std::to_string(i) + "].lightPos").c_str(), scene->lights[i]->lightPos);
        quadShader->setVec3(("lights[" + std::to_string(i) + "].lightColor").c_str(), scene->lights[i]->lightColor);
        //quadShader->setFloat(("lights[" + std::to_string(i) + "].linear").c_str(), 0.7);
        //quadShader->setFloat(("lights[" + std::to_string(i) + "].quadratic").c_str(), 1.8);
    }

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, scene->textures["G_position"]);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, scene->textures["G_normal"]);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, scene->textures["G_albedo_specular"]);
    scene->primitives["quad"]->draw();
    checkGLErrors();
    checkFramebufferStatus();
    
    // copy depth buffer to default framebuffer's depth buffer
    glBindFramebuffer(GL_READ_FRAMEBUFFER, scene->getGBuffer());
    //glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); // write to default framebuffer
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, scene->FBOs["hdr"]); // write to hdr
    // blit to default framebuffer. Note that this may or may not work as the internal formats of both the FBO and default framebuffer have to match.
    // the internal formats are implementation defined. This works on all of my systems, but if it doesn't on yours you'll likely have to write to the
    // depth buffer in another shader stage (or somehow see to match the default framebuffer's internal format with the FBO's internal format).
    glBlitFramebuffer(0, 0, scene->screenWidth, scene->screenHeight, 0, 0, scene->screenWidth, scene->screenHeight, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
    //glBindFramebuffer(GL_FRAMEBUFFER, 0);
    //glBindFramebuffer(GL_FRAMEBUFFER, scene->FBOs["hdr"]);
    
    // render lights
    Shader* lightShader = scene->shaders["lights"];
    lightShader->use();
    lightShader->setMat4("projection", projection);
    lightShader->setMat4("view", view);

    glm::mat4 lightModel;
    for (int i = 0; i < 32; ++i) {
        lightModel = glm::mat4(1.0f);
        lightModel = glm::translate(lightModel, scene->lights[i]->lightPos);
        lightModel = glm::scale(lightModel, glm::vec3(0.2, 0.2, 0.2));
        lightShader->setMat4("model", lightModel);
        lightShader->setVec3("lightColor", scene->lights[i]->lightColor);
        scene->models["sphere"]->draw(lightShader);
    }
    checkGLErrors();
    checkFramebufferStatus();
    
//    //skybox
//    glDepthFunc(GL_LEQUAL);
//    Shader* skyShader = scene->shaders["skybox"];
//    skyShader->use();
//    skyShader->setMat4("view", view);
//    skyShader->setMat4("projection", projection);
//    glActiveTexture(GL_TEXTURE0);
//    glBindTexture(GL_TEXTURE_CUBE_MAP, scene->textures["skybox"]);
//    scene->primitives["skybox"]->draw();
//    glDepthFunc(GL_LESS);
    
    // third pass for blurring
    unsigned int pingpongFBO[2], pingpongBuffer[2];
    pingpongFBO[0] = scene->FBOs["pingpong0"];
    pingpongFBO[1] = scene->FBOs["pingpong1"];
    pingpongBuffer[0] = scene->textures["pingpong0"];
    pingpongBuffer[1] = scene->textures["pingpong1"];
    
    Shader* blurShader = scene->shaders["blur"];
    blurShader->use();
    blurShader->setInt("image", 0);
    
    bool horizontal = true, first_iter = true;
    int amount = 10;
    for (unsigned int i = 0; i < amount; ++i) {
        glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);
        blurShader->setInt("horizontal", horizontal);
        glBindTexture(GL_TEXTURE_2D, first_iter ? scene->textures["hdr_bright"] : pingpongBuffer[!horizontal]);
        scene->primitives["quad"]->draw();
        horizontal = !horizontal;
        if (first_iter) {
            first_iter = false;
        }
    }
    
    // draw full-screen quad
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    quadShader = scene->shaders["bloom"];
    quadShader->use();
    quadShader->setInt("texture_color", 0);
    quadShader->setInt("texture_bright", 1);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, scene->textures["hdr_color"]);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, pingpongBuffer[!horizontal]);
    scene->primitives["quad"]->draw();
}

#endif /* world_h */
