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
#include <geometry.hpp>
#include <shaderProgram.hpp>
#include <ImageLoader.hpp>
#include <light.hpp>
#include <camera.hpp>

using namespace std;

class Scene {
public:
    Scene();
    ~Scene();
    
    unordered_map<string, Shader*> shaders;
    unordered_map<string, unsigned int> textures;
    unordered_map<string, Model*> models;
    unordered_map<string, Geometry*> primitives;
    unordered_map<string, unsigned int> FBOs;
    unordered_map<string, unsigned int> RBOs;
    vector<Light*> lights;
    Camera* camera;
    
    
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
    void addLight(LightType type, const glm::vec3& pos, const glm::vec3& color);
    void render();
    void configFrameBuffers();
    void loadSkyboxTexture();
    void addCamera(Camera* cam);
    void addNullHdrCubemapTexture(string name, GLuint width, GLuint height);
    void addNullHdrCubemapMipmap(string name, GLuint width, GLuint height);
    void addNullTexture(string name, GLuint width, GLuint height);
    
    Geometry* addGeometry(string name, PrimitiveType type);
    Model* addModel(string name, string filePath);
};
#endif /* scene_hpp */
