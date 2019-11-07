//
//  Mesh.hpp
//  playground
//
//  Created by Zooey Zhang on 7/25/18.
//

#ifndef Mesh_h
#define Mesh_h

#include <string>
#include <vector>
#include <stdlib.h>
#include <stdio.h>
#include <GL/glew.h> //loader, must be loaded first
#include <GLFW/glfw3.h> //window and keyboard
#include <glm/glm.hpp> //3D Math library
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <shaderProgram.hpp>

using namespace std;

struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
    glm::vec3 Tangent;
    glm::vec3 Bitangent;
};

struct Texture {
    unsigned int id;
    string type;
    string path;
};

class Mesh {
public:
    Mesh(vector<Vertex> vertices, vector<unsigned int> indices, vector<Texture> textures);
    
    vector<Vertex> vertices;
    vector<unsigned int> indices;
    vector<Texture> textures;
    vector<Texture> textures_external;
    
    void Draw(Shader& shader);
    void BindExternalTextures(vector<Texture> textures);
    unsigned int VAO, VBO, EBO;
private:
    
    void setupMesh();
};


#endif /* Mesh_h */
