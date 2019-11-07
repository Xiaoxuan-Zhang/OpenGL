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
    
    unsigned int VAO;
    unsigned int VBO;
    vector<float> vertices;
    int numOfVertices;
    
    void draw();
    
};


class Cube : public Geometry {
public:
    Cube();
};

class Quad : public Geometry {
public:
    Quad();
};
