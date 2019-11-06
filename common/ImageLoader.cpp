//
//  ImageLoader.cpp
//  playground
//
//  Created by Zooey Zhang on 7/9/18.
//


#include "ImageLoader.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "std_image.h"

GLuint load_BMP(const char * imagePath) {
    // Data read from the header of the BMP file
    unsigned char header[54]; // Each BMP file begins by a 54-bytes header
    unsigned int dataPos;     // Position in the file where the actual data begins
    unsigned int width, height;
    unsigned int imageSize;   // = width*height*3
    // Actual RGB data
    unsigned char * data;
    
    FILE * pFile = fopen(imagePath , "r");
    if (pFile == NULL) {
        printf("Image could not be loaded!");
        return false;
    }
    if ( fread(header, 1, 54, pFile)!=54 ){ // If not 54 bytes read : problem
        printf("Not a correct BMP file\n");
        return false;
    }
    if (header[0] != 'B' || header[1] != 'M') {
        //The first two bytes should be 'B' and 'M'
        printf("Incorrect header!");
        return false;
    }
    // Read ints from the byte array
    dataPos    = *(int*)&(header[0x0A]);
    width      = *(int*)&(header[0x12]);
    height     = *(int*)&(header[0x16]);
    imageSize  = *(int*)&(header[0x22]);
    
    // Some BMP files are misformatted, guess missing information
    if (imageSize==0)    imageSize=width*height*3; // 3 : one byte for each Red, Green and Blue component
    if (dataPos==0)      dataPos=54; // The BMP header is done that way
    
    // Create a buffer
    data = new unsigned char [imageSize];
    
    // Read the actual data from the file into the buffer
    fread(data,1,imageSize,pFile);
    
    //Everything is in memory now, the file can be closed
    fclose(pFile);
    
    // Create one OpenGL texture
    GLuint textureID;
    glGenTextures(1, &textureID);
    // "Bind" the newly created texture : all future texture functions will modify this texture
    glBindTexture(GL_TEXTURE_2D, textureID);
    // Give the image to OpenGL
    //BMP does not store Red->Green->Blue but Blue->Green->Red, so we have to tell it to OpenGL using GL_BGR.
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);

    // OpenGL has now copied the data. Free our own version
    delete [] data;

    // ... nice trilinear filtering ...
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    // ... which requires mipmaps. Generate them automatically.
    glGenerateMipmap(GL_TEXTURE_2D);
    
    // Return the ID of the texture we just created
    return textureID;
}

GLuint load_Texture(const char * imagePath, bool gammaCorrection) {
    // Create one OpenGL texture
    GLuint textureID;
    glGenTextures(1, &textureID);
    
    int width, height, nrChannels;
    unsigned char *data = stbi_load(imagePath, &width, &height, &nrChannels, 0);
    
    GLint format;
    GLint internalFormat;
    if (nrChannels == 1)
    {
        internalFormat = format = GL_RED;
    }
    else if (nrChannels == 3)
    {
        format = GL_RGB;
        internalFormat = gammaCorrection? GL_SRGB : GL_RGB;
    }
    else if (nrChannels == 4)
    {
        format = GL_RGBA;
        internalFormat = gammaCorrection? GL_SRGB_ALPHA : GL_RGBA;
    }
    if (data) {
        // "Bind" the newly created texture : all future texture functions will modify this texture
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); //GL_CLAMP_TO_EDGE if using transparency
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); //GL_CLAMP_TO_EDGE
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
        stbi_image_free(data);
    } else {
        printf("Image could not be loaded!");
        stbi_image_free(data);
    }
    return textureID;
}

GLuint load_Cubemap(vector<string> imagePaths) {
    // Create one OpenGL texture
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
    
    int width, height, nrChannels;
    GLint format = GL_RED;
    for (unsigned int i =0; i < imagePaths.size(); i++) {
        unsigned char *data = stbi_load(imagePaths[i].c_str(), &width, &height, &nrChannels, 0);
        
        if (nrChannels == 1)
            format = GL_RED;
        else if (nrChannels == 3)
            format = GL_RGB;
        else if (nrChannels == 4)
            format = GL_RGBA;
        if (data) {
            // "Bind" the newly created texture : all future texture functions will modify this texture
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        } else {
            printf("Image could not be loaded!");
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    return textureID;
}

