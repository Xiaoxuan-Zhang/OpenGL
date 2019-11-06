#include <stdio.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <sstream>

#include <stdlib.h>
#include <string.h>

#include <GL/glew.h>

#include "shaderProgram.hpp"

Shader::Shader(const char * geometry_file_path, const char * vertex_file_path,const char * fragment_file_path){
    ID = glCreateProgram();
    
    GLuint vertexShaderId = loadShader(vertex_file_path, GL_VERTEX_SHADER);
    GLuint fragmentShaderId = loadShader(fragment_file_path, GL_FRAGMENT_SHADER);
    GLuint geometryShaderId = loadShader(geometry_file_path, GL_GEOMETRY_SHADER);
    
    // Link the program
    printf("Linking program\n");
    glAttachShader(ID, geometryShaderId);
    glAttachShader(ID, vertexShaderId);
    glAttachShader(ID, fragmentShaderId);
    glLinkProgram(ID);
    
    GLint Result = GL_FALSE;
    int InfoLogLength;
    // Check the program
    glGetProgramiv(ID, GL_LINK_STATUS, &Result);
    glGetProgramiv(ID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if ( InfoLogLength > 0 ){
        std::vector<char> ProgramErrorMessage(InfoLogLength+1);
        glGetProgramInfoLog(ID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
        printf("%s\n", &ProgramErrorMessage[0]);
    }
    //shaders have been linked to program, clean them up now.
    glDetachShader(ID, geometryShaderId);
    glDeleteShader(geometryShaderId);
    glDetachShader(ID, vertexShaderId);
    glDeleteShader(vertexShaderId);
    glDetachShader(ID, fragmentShaderId);
    glDeleteShader(fragmentShaderId);
}

Shader::Shader(const char * vertex_file_path,const char * fragment_file_path){
    ID = glCreateProgram();
    
    GLuint vertexShaderId = loadShader(vertex_file_path, GL_VERTEX_SHADER);
    GLuint fragmentShaderId = loadShader(fragment_file_path, GL_FRAGMENT_SHADER);
    
    // Link the program
    printf("Linking program\n");
    glAttachShader(ID, vertexShaderId);
    glAttachShader(ID, fragmentShaderId);
    glLinkProgram(ID);
    
    GLint Result = GL_FALSE;
    int InfoLogLength;
    // Check the program
    glGetProgramiv(ID, GL_LINK_STATUS, &Result);
    glGetProgramiv(ID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if ( InfoLogLength > 0 ){
        std::vector<char> ProgramErrorMessage(InfoLogLength+1);
        glGetProgramInfoLog(ID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
        printf("%s\n", &ProgramErrorMessage[0]);
    }
    //shaders have been linked to program, clean them up now.
    glDetachShader(ID, vertexShaderId);
    glDeleteShader(vertexShaderId);
    glDetachShader(ID, fragmentShaderId);
    glDeleteShader(fragmentShaderId);
}
GLuint Shader::loadShader(const char * shader_file_path, GLenum shaderType) {
    GLuint shaderID = glCreateShader(shaderType);
    // Read the Vertex Shader code from the file
    std::string shaderCode;
    std::ifstream shaderStream(shader_file_path, std::ios::in);
    if(shaderStream.is_open()){
        std::stringstream sstr;
        sstr << shaderStream.rdbuf();
        shaderCode = sstr.str();
        shaderStream.close();
    }else{
        printf("Impossible to open %s. Are you in the right directory ? Don't forget to read the FAQ !\n", shader_file_path);
        getchar();
    }
    GLint Result = GL_FALSE;
    int InfoLogLength;
    
    // Compile Geometry Shader
    printf("Compiling shader : %s\n", shader_file_path);
    char const * sourcePointer = shaderCode.c_str();
    glShaderSource(shaderID, 1, &sourcePointer , NULL);
    glCompileShader(shaderID);
    
    // Check Geometry Shader
    glGetShaderiv(shaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if ( InfoLogLength > 0 ){
        std::vector<char> shaderErrorMessage(InfoLogLength+1);
        glGetShaderInfoLog(shaderID, InfoLogLength, NULL, &shaderErrorMessage[0]);
        printf("%s\n", &shaderErrorMessage[0]);
    }
    return shaderID;
}

void Shader::use() {
    glUseProgram(ID);
}

void Shader::setBool(const char * name, bool value) const
{
    glUniform1i(glGetUniformLocation(ID, name), (int)value);
}
// ------------------------------------------------------------------------
void Shader::setInt(const char * name, int value) const
{
    glUniform1i(glGetUniformLocation(ID, name), value);
}
// ------------------------------------------------------------------------
void Shader::setFloat(const char * name, float value) const
{
    glUniform1f(glGetUniformLocation(ID, name), value);
}
void Shader::setVec3(const char * name, glm::vec3 value) const
{
    glUniform3fv(glGetUniformLocation(ID, name), 1, glm::value_ptr(value));
}
void Shader::setVec2(const char * name, glm::vec2 value) const
{
    glUniform2fv(glGetUniformLocation(ID, name), 1, glm::value_ptr(value));
}
void Shader::setMat4(const char * name, glm::mat4 value) const
{
    glUniformMatrix4fv(glGetUniformLocation(ID, name), 1, GL_FALSE, glm::value_ptr(value));
}
Shader::~Shader() {
    glDeleteProgram(ID);
}

