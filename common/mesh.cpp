//
//  mesh.cpp
//  misc05_picking_BulletPhysics
//
//  Created by Zooey Zhang on 7/25/18.
//

#include <mesh.hpp>

Mesh::Mesh(vector<Vertex> vertices, vector<unsigned int> indices, vector<Texture> textures) {
    this->vertices = vertices;
    this->indices = indices;
    this->textures = textures;
    setupMesh();
};

void Mesh::BindExternalTextures(vector<Texture> textures) {
    externalTextures = textures;
};

void Mesh::Draw(Shader* shader) {
    // draw mesh
    shader->use();
    unsigned int diffMapN = 1;
    unsigned int specMapN = 1;
    unsigned int reflectMapN = 1;
    unsigned int normapMapN = 1;
    unsigned int texIndex = 0;
    for (unsigned int i = 0; i < textures.size(); i ++ ){
        // bind diffuse map
        glActiveTexture(GL_TEXTURE0 + i);
        string number;
        string name = textures[i].type;
        if (name == "texture_diffuse") {
            number = std::to_string(diffMapN++);
        } else if (name == "texture_specular"){
            number = std::to_string(specMapN++);
        } else if (name == "texture_reflection") {
            number = std::to_string(reflectMapN++);
        } else if (name == "texture_normal") {
            number = std::to_string(normapMapN++);
        }
        shader->setInt(("material." + name + number).c_str(), i);
        glBindTexture(GL_TEXTURE_2D, textures[i].id);
    }
    
    texIndex = (unsigned int)textures.size();
    for (unsigned int i = 0; i < externalTextures.size(); i ++ ){
        glActiveTexture(GL_TEXTURE0 + texIndex + i);
        shader->setInt(externalTextures[i].type.c_str(), texIndex + i);
        if (externalTextures[i].type == "cubemap") {
            glBindTexture(GL_TEXTURE_CUBE_MAP, externalTextures[i].id);
        } else {
            glBindTexture(GL_TEXTURE_2D, externalTextures[i].id);
        }
    }
    
    shader->setFloat("material.shininess", 32.0f);
    
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    // always good practice to set everything back to defaults once configured.
    glBindVertexArray(0);
    glActiveTexture(GL_TEXTURE0);
};

void Mesh::setupMesh() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));
    
    
    glBindVertexArray(0);
};
