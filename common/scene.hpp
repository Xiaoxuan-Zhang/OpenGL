//
//  scene.hpp
//  openGL_app
//
//  Created by Zooey Zhang on 10/8/19.
//  Copyright © 2019 Zooey Zhang. All rights reserved.
//

#ifndef scene_hpp
#define scene_hpp

#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <glm/glm.hpp> //3D Math library
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <model.hpp>
#include <shaderProgram.hpp>
#include <ImageLoader.hpp>
#include <light.hpp>

using namespace std;

class Scene {
public:
    Scene() {};
    ~Scene() {};
    
    unordered_map<string, Shader*> shaders;
    unordered_map<string, unsigned int> textures;
    unordered_map<string, Model*> models;
    vector<Light*> lights;
    
    
    unsigned int screenWidth = 1280;
    unsigned int screenHeight = 720;
    
    unsigned int skyboxTexture;
    unsigned int hdrFBO;
    unsigned int colorBuffers[2];
    unsigned int rboDepth;
    unsigned int pingpongFBO[2];
    unsigned int pingpongColorbuffers[2];
    unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    
    void screenSize(unsigned int width, unsigned height);
    void loadData();
    void addLight(LightType type, const glm::vec3& pos, const glm::vec3& color);
    void render();
    void configFrameBuffers();
    void skybox();
    
};
#endif /* scene_hpp */
