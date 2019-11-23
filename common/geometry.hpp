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

class Geometry {
public:
    Geometry() {};
    virtual ~Geometry() {};
    virtual void draw();
    
protected:
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

class RenderObject : public Geometry {
public:
    RenderObject(Geometry* geo) : geometry(geo) {};
    void setTransform(glm::mat4 trans);
    void render();
    
protected:
    Geometry* geometry;
    glm::mat4 transformMatrix;
};
