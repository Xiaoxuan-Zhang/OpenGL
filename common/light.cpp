//
//  light.cpp
//  openGL_app
//
//  Created by Zooey Zhang on 10/8/19.
//  Copyright © 2019 Zooey Zhang. All rights reserved.
//

#include "light.hpp"

Light::Light(int id, LightType type, glm::vec3 position, glm::vec3 color) {
    this->addLight(id, type, position, color);
}

void Light::addLight(int id, LightType type, glm::vec3 position, glm::vec3 color) {
    this->lightId = id;
    this->lightType = type;
    this->lightPos = position;
    this->lightColor = color;
}
