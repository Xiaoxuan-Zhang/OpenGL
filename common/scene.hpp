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
    
    
    GLuint screenWidth = 1280;
    GLuint screenHeight = 720;
    
    GLuint skyboxTexture;
    GLuint hdrFBO;
    GLuint colorBuffers[2];
    GLuint rboDepth;
    GLuint pingpongFBO[2];
    GLuint pingpongColorbuffers[2];
    GLuint attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    
    void screenSize(GLuint width, GLuint height);
    void addLight(LightType type, const glm::vec3& pos, const glm::vec3& color);
    void render();
    void configFrameBuffers();
    void addCamera(Camera* cam);
    void addTexture(string name, GLuint id);
    
    Geometry* addGeometry(string name, PrimitiveType type);
    Model* addModel(string name, string filePath);
};
#endif /* scene_hpp */
