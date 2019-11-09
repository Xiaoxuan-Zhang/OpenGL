//
//  geometry.hpp
//  openGL_app
//
//  Created by Sirius Ding on 11/6/19.
//  Copyright © 2019 Zooey Zhang. All rights reserved.
//

#ifndef geometry_hpp
#define geometry_hpp

#include <stdio.h>
#include <vector>
#include <GL/glew.h> //loader, must be loaded first
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#endif /* geometry_hpp */
using namespace std;

enum PrimitiveType {
    CUBE = 0,
    QUAD = 1,
    SPHERE = 2,
    SKYBOX
};

class Geometry {
public:
    Geometry() {};
    virtual ~Geometry() {};
    
    virtual void setTransformMatrix(glm::mat4 t);
    virtual glm::mat4 getTransform();
    virtual void draw();
    
protected:
    glm::mat4 transformMatrix;
    int numOfVertices;
    vector<float> vertexData;
    unsigned int VAO;
    unsigned int VBO;
    unsigned int VEO;
    unsigned int shader;
};

class Cube : public Geometry {
public:
    Cube();
    ~Cube() {};
};

class Quad : public Geometry {
public:
    Quad();
    ~Quad() {};
};

class Skybox : public Geometry {
public:
    Skybox();
    ~Skybox() {};
};

class Sphere : public Geometry {
public:
    Sphere();
    ~Sphere() {};
    
    void draw();
    
    vector<glm::vec3> positions;
    vector<glm::vec3> normals;
    vector<glm::vec2> uvs;
    vector<int> indices;
};
