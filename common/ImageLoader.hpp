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
#include <iostream>
#include <GL/glew.h>
#include <fstream>
#include <vector>

using namespace std;

GLuint loadBMP(const char * imagePath);
GLuint loadTexture(const char * imagePath, bool gammaCorrection = false);
GLuint loadCubemap(string folderPath, string fileFormat = "jpg");
GLuint loadHDRTexture(const char * imagePath);
#endif /* ImageLoader_hpp */

