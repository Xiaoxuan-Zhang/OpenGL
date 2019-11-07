//
//  light.hpp
//  openGL_app
//
//  Created by Zooey Zhang on 10/8/19.
//  Copyright © 2019 Zooey Zhang. All rights reserved.
//

#ifndef light_hpp
#define light_hpp

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <glm/glm.hpp> //3D Math library
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace std;

enum LightType {
    DIR_LIGHT = 0,
    POINT_LIGHT,
    SPOT_LIGHT
};

class Light {
public:
    Light(int id, LightType type, const glm::vec3& position, const glm::vec3& color);

    int lightId;
    LightType lightType;
    glm::vec3 lightPos;
    glm::vec3 lightColor;
    
    void addLight(int id, LightType type, const glm::vec3& position, const glm::vec3& color);
};
#endif /* light_hpp */
