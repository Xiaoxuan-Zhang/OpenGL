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
    void Draw(Shader& shader);
    void BindTexture(GLenum target,GLuint texture);
    vector<Mesh> meshes;
    vector<Texture> textures_loaded;
private:
    
    vector<Texture> textures_external;
    string directory;
    void processNode(aiNode* node, const aiScene* scene);
    Mesh processMesh(aiMesh* mesh, const aiScene* scene);
    vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type,
                                         string typeName);
};

#endif /* model_hpp */
