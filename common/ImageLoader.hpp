//
//  ImageLoader.hpp
//  playground
//
//  Created by Zooey Zhang on 7/9/18.
//

#ifndef ImageLoader_hpp
#define ImageLoader_hpp

#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <fstream>
#include <vector>

using namespace std;

GLuint load_BMP(const char * imagePath);
GLuint load_Texture(const char * imagePath, bool gammaCorrection = false);
GLuint load_Cubemap(vector<string> imagePaths);
#endif /* ImageLoader_hpp */

