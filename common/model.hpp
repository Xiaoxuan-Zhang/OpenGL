//
//  model.hpp
//  misc05_picking_BulletPhysics
//
//  Created by Zooey Zhang on 7/26/18.
//

#ifndef model_hpp
#define model_hpp

#include <stdio.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <mesh.hpp>
#include <shaderProgram.hpp>
#include <ImageLoader.hpp>

class Model {
public:
    Model(const char* path);
    void loadModel(const char* path);
    void BindTexture(GLenum target,GLuint texture);
    void draw(Shader* shader);
    
    vector<Mesh> meshes;
    vector<Texture> loadedTextures;
        
private:
    
    vector<Texture> externalTextures;
    string directory;
    void processNode(aiNode* node, const aiScene* scene);
    Mesh processMesh(aiMesh* mesh, const aiScene* scene);
    vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type,
                                         string typeName);
    void calcTangent(Vertex v1, Vertex v2, Vertex v3, glm::vec3& tangent, glm::vec3& bitangent);
};

#endif /* model_hpp */
