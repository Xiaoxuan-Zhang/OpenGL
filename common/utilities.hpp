//
//  utilities.hpp
//  openGL_app
//
//  Created by Zooey Zhang on 11/20/19.
//  Copyright © 2019 Zooey Zhang. All rights reserved.
//

#ifndef utilities_hpp
#define utilities_hpp

#include <stdio.h>
#include <GL/glew.h>
#include <glm/glm.hpp> //3D Math library
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <random>

void checkGLErrors();
void checkFramebufferStatus();
unsigned int addNullHdrCubemapTexture(unsigned int width, unsigned int height);
unsigned int addNullHdrCubemapMipmap(unsigned int width, unsigned int height);
unsigned int addNullTexture(unsigned int width, unsigned int height, GLuint internalFormat = GL_RGB, GLenum format = GL_RGB, GLenum dataType = GL_FLOAT, GLenum filter = GL_LINEAR, GLenum wrap = GL_CLAMP_TO_EDGE);
unsigned int addNullDepthTexture(unsigned int width, unsigned int height);
float randomFloat();
#endif /* utilities_hpp */
