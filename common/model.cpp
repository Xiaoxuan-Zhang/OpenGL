//
//  model.cpp
//  misc05_picking_BulletPhysics
//
//  Created by Zooey Zhang on 7/26/18.
//

#include "model.hpp"


Model::Model(const char* path) {
    loadModel(path);
};

void Model::draw(Shader* shader) {
    for (unsigned int i = 0; i < meshes.size(); i ++) {
        meshes[i].BindExternalTextures(externalTextures);
        meshes[i].Draw(shader);
    };
};

void Model::BindTexture(GLenum target,GLuint texture) {
    Texture extTexture;
    if (target == GL_TEXTURE_CUBE_MAP) {
        extTexture.type = "cubemap";
    } else {
        extTexture.type = "texture";
    }
    extTexture.id = texture;
    externalTextures.push_back(extTexture);
};

void Model::loadModel(const char* path) {
    Assimp::Importer import;
    string pathStr = path;
    const aiScene* scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace);
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        printf("ERROR: %s", import.GetErrorString());
        return;
    }
    directory = pathStr.substr(0, pathStr.find_last_of("/"));
    processNode(scene->mRootNode, scene);
}

void Model::processNode(aiNode* node, const aiScene* scene) {
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(processMesh(mesh,scene));
    }
    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        processNode(node->mChildren[i], scene);
    }
};

Mesh Model::processMesh(aiMesh* mesh, const aiScene* scene) {
    vector<Vertex> vertices;
    vector<unsigned int> indices;
    vector<Texture> textures;
    for (unsigned int i = 0; i < mesh->mNumVertices; i++){
        Vertex vertex;
        glm::vec3 myVector3;
        myVector3.x = mesh->mVertices[i].x;
        myVector3.y = mesh->mVertices[i].y;
        myVector3.z = mesh->mVertices[i].z;
        vertex.Position = myVector3;
        myVector3.x = mesh->mNormals[i].x;
        myVector3.y = mesh->mNormals[i].y;
        myVector3.z = mesh->mNormals[i].z;
        vertex.Normal = myVector3;
        myVector3.x = mesh->mTangents[i].x;
        myVector3.y = mesh->mTangents[i].y;
        myVector3.z = mesh->mTangents[i].z;
        vertex.Tangent = myVector3;
        myVector3.x = mesh->mBitangents[i].x;
        myVector3.y = mesh->mBitangents[i].y;
        myVector3.z = mesh->mBitangents[i].z;
        vertex.Bitangent = myVector3;
        if (mesh->mTextureCoords[0]) {
            glm::vec2 myVector2;
            myVector2.x = mesh->mTextureCoords[0][i].x;
            myVector2.y = mesh->mTextureCoords[0][i].y;
            vertex.TexCoords = myVector2;
        } else {
            vertex.TexCoords = glm::vec2(0.0f, 0.0f);
        }
        vertices.push_back(vertex);
    }
    for (unsigned int i = 0; i < mesh->mNumFaces; i++){
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j ++) {
            indices.push_back(face.mIndices[j]);
        }
    }
    if (mesh->mMaterialIndex >= 0) {
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        vector<Texture> diffuseMaps = loadMaterialTextures(material,
                                                           aiTextureType_DIFFUSE, "texture_diffuse");
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
        vector<Texture> specularMaps = loadMaterialTextures(material,
                                                            aiTextureType_SPECULAR, "texture_specular");
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
        
        vector<Texture> reflectMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_reflection");
        textures.insert(textures.end(), reflectMaps.begin(), reflectMaps.end());
        vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
        textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
    }
    return Mesh(vertices, indices, textures);
};
vector<Texture> Model::loadMaterialTextures(aiMaterial *mat, aiTextureType type, string typeName)
{
    vector<Texture> textures;
    for(unsigned int i = 0; i < mat->GetTextureCount(type); i++)
    {
        aiString str;
        mat->GetTexture(type, i, &str);
        bool skip = false;
        for(unsigned int j = 0; j < loadedTextures.size(); j++)
        {
            if(std::strcmp(loadedTextures[j].path.data(), str.C_Str()) == 0)
            {
                textures.push_back(loadedTextures[j]);
                skip = true;
                break;
            }
        }
        if(!skip)
        {   // if texture hasn't been loaded already, load it
            Texture texture;
            string imgPath = directory + "/" + str.C_Str();
            texture.id = loadTexture(imgPath.c_str());
            texture.type = typeName;
            texture.path = str.C_Str();
            textures.push_back(texture);
            loadedTextures.push_back(texture); // add to loaded textures
        }
    }
    return textures;
}
