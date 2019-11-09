//
//  world.h
//  openGL_app
//
//  Created by Sirius Ding on 11/8/19.
//  Copyright © 2019 Zooey Zhang. All rights reserved.
//

#ifndef world_h
#define world_h

#include <scene.hpp>

Scene* defaultScene(int screenWdith, int screenHight, Camera *camera) {
    Scene* newScene = new Scene();
    newScene->screenSize(screenWdith, screenHight);
    newScene->addCamera(camera);
    
    // build and compile shaders
    newScene->shaders["skybox"] = new Shader("shaders/cubemap.vs", "shaders/cubemap.fs");
    newScene->shaders["simple_texture"] = new Shader("shaders/simpleTexture.vs", "shaders/simpleTexture.fs");
    newScene->shaders["envmap"] = new Shader("shaders/envmap.vs", "shaders/envmap.fs");
    newScene->shaders["nano"] = new Shader("shaders/nanoShader.vs", "shaders/nanoShader.fs");
    newScene->shaders["pbr"] = new Shader("shaders/PBR/pbrShader.vs", "shaders/PBR/pbrShader.fs");
    
    newScene->shaders["skybox"]->setInt("skybox", 0);
    newScene->shaders["simple_texture"]->setInt("texture_diffuse", 0);
    
    // load textures
    newScene->loadSkyboxTexture();
    newScene->textures["wood"] = load_Texture("resources/wood.png", true); // note that we're loading the texture as an SRGB texture
    newScene->textures["container"] = load_Texture("resources/container2.png", true); // note that we're loading the texture as an SRGB texture
    newScene->textures["brick_wall"] = load_Texture("resources/brickwall.jpg", true);
    newScene->textures["pbr_albedo"] = load_Texture("resources/rustediron1/basecolor.png", false);
    newScene->textures["pbr_metallic"] = load_Texture("resources/rustediron1/metallic.png", false);
    newScene->textures["pbr_roughness"] = load_Texture("resources/rustediron1/roughness.png", false);
    newScene->textures["pbr_normal"] = load_Texture("resources/rustediron1/normal.png", false);
    
    // load models
    newScene->addModel("moon", "resources/44-moon-photorealistic-2k/Moon 2K.obj");
    newScene->addModel("rock", "resources/rock/rock.obj");
    newScene->addModel("nanosuit", "resources/nanosuit/nanosuit.obj");
    newScene->addModel("sphere", "resources/sphere.obj");
    
    // assign cubemap for models
    newScene->models["nanosuit"]->BindTexture(GL_TEXTURE_CUBE_MAP, newScene->skyboxTexture);
    
    //add primitives
    Geometry* cube = newScene->addGeometry("cube0", CUBE);
    Geometry* quad = newScene->addGeometry("quad0", QUAD);
    newScene->addGeometry("skybox", SKYBOX);
    
    // add lights
    newScene->addLight(POINT_LIGHT, glm::vec3(0.0f, 0.5f,  1.5f), glm::vec3(2.0f, 2.0f, 2.0f));
    newScene->addLight(POINT_LIGHT, glm::vec3(-4.0f, 0.5f, -3.0f), glm::vec3(1.5f, 0.0f, 0.0f));
    newScene->addLight(POINT_LIGHT, glm::vec3(3.0f, 0.5f,  1.0f), glm::vec3(0.0f, 0.0f, 1.5f));
    newScene->addLight(POINT_LIGHT, glm::vec3(-.8f,  2.4f, -1.0f), glm::vec3(0.0f, 1.5f, 0.0f));
    
    return newScene;
}

void renderScene(Scene* scene) {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    Camera* cam = scene->camera;
    glm::mat4 projection = glm::perspective(glm::radians(cam->Zoom), (float)scene->screenWidth/ (float)scene->screenHeight, 0.1f, 100.0f);
    glm::mat4 view = cam->GetViewMatrix();
    glm::mat4 model;
    
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-3.0f, 0.0f, 0.0));
    scene->shaders["nano"]->use();
    scene->shaders["nano"]->setMat4("model", model);
    scene->shaders["nano"]->setMat4("view", view);
    scene->shaders["nano"]->setMat4("projection", projection);
    scene->models["nanosuit"]->draw(scene->shaders["nano"]);
    
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(3.0f, 2.0f, 0.0));
    scene->shaders["simple_texture"]->use();
    scene->shaders["simple_texture"]->setMat4("model", model);
    scene->shaders["simple_texture"]->setMat4("view", view);
    scene->shaders["simple_texture"]->setMat4("projection", projection);
    scene->models["moon"]->draw(scene->shaders["simple_texture"]);
    
    scene->shaders["envmap"]->use();
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(2.0f, 9.0f, -3.0));
    model = glm::scale(model, glm::vec3(2.0f, 2.0f, 2.0));
    model = glm::rotate(model, glm::radians(45.0f), glm::vec3(0.0, 0.0, 1.0));
    scene->shaders["envmap"]->setMat4("model", model);
    scene->shaders["envmap"]->setMat4("view", view);
    scene->shaders["envmap"]->setMat4("projection", projection);
    scene->shaders["envmap"]->setVec3("cameraPos", cam->Position);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, scene->skyboxTexture);
    scene->primitives["cube0"]->draw();
    
    scene->shaders["simple_texture"]->use();
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0, 0.0, 0.0));
    model = glm::scale(model, glm::vec3(20.0f, 20.0f, 20.0));
    model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0, 0.0, 0.0));
    scene->shaders["simple_texture"]->setMat4("model", model);
    scene->shaders["simple_texture"]->setMat4("view", view);
    scene->shaders["simple_texture"]->setMat4("projection", projection);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, scene->textures["brick_wall"]);
    scene->primitives["quad0"]->draw();
    
    scene->shaders["pbr"]->use();
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(3.0f, 10.0f, 0.0f));
    model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    scene->shaders["pbr"]->setMat4("model", model);
    scene->shaders["pbr"]->setMat4("view", view);
    scene->shaders["pbr"]->setMat4("projection", projection);
    scene->shaders["pbr"]->setVec3("cameraPos", cam->Position);
    scene->shaders["pbr"]->setInt("texture_diffuse", 0);
    scene->shaders["pbr"]->setInt("texture_normal", 1);
    scene->shaders["pbr"]->setInt("texture_metallic", 2);
    scene->shaders["pbr"]->setInt("texture_roughness", 3);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, scene->textures["pbr_albedo"]);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, scene->textures["pbr_normal"]);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, scene->textures["pbr_metallic"]);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, scene->textures["pbr_roughness"]);
    scene->models["sphere"]->draw(scene->shaders["pbr"]);
    
    //skybox
    glDepthFunc(GL_LEQUAL);
    scene->shaders["skybox"]->use();
    view = glm::mat4(glm::mat3(view));
    scene->shaders["skybox"]->setMat4("view", view);
    scene->shaders["skybox"]->setMat4("projection", projection);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, scene->skyboxTexture);
    scene->primitives["skybox"]->draw();
    glDepthFunc(GL_LESS);
}

#endif /* world_h */
