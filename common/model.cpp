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
    unordered_map<unsigned int, vector<glm::vec3>> tangents;
    unordered_map<unsigned int, vector<glm::vec3>> bitangents;
    for (unsigned int i = 0; i < mesh->mNumVertices; i++){
        Vertex vertex;
        glm::vec3 pos = glm::vec3(0.0f), nor = glm::vec3(0.0f);
        glm::vec2 uv = glm::vec2(0.0f);
        pos.x = mesh->mVertices[i].x;
        pos.y = mesh->mVertices[i].y;
        pos.z = mesh->mVertices[i].z;
        vertex.Position = pos;
        nor.x = mesh->mNormals[i].x;
        nor.y = mesh->mNormals[i].y;
        nor.z = mesh->mNormals[i].z;
        vertex.Normal = nor;
        
        if (mesh->mTextureCoords[0]) {
            uv.x = mesh->mTextureCoords[0][i].x;
            uv.y = mesh->mTextureCoords[0][i].y;
        }
        vertex.TexCoords = uv;
        vertex.Tangent = glm::vec3(0.0f);
        vertex.Bitangent = glm::vec3(0.0f);
        if (mesh->mTangents != NULL) {
            vertex.Tangent.x = mesh->mTangents[i].x;
            vertex.Tangent.y = mesh->mTangents[i].y;
            vertex.Tangent.z = mesh->mTangents[i].z;
        } else {
            vertex.Tangent = glm::vec3(0.0f, 0.0f, 0.0f);
        }

        if (mesh->mBitangents != NULL) {
            vertex.Bitangent.x = mesh->mBitangents[i].x;
            vertex.Bitangent.y = mesh->mBitangents[i].y;
            vertex.Bitangent.z = mesh->mBitangents[i].z;
        } else {
            vertex.Bitangent = glm::vec3(0.0f, 0.0f, 0.0f);
        }
        
        vertices.push_back(vertex);
    }
    
    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        glm::vec3 tangent = glm::vec3(0.0f);
        glm::vec3 bitangent = glm::vec3(0.0f);
        if (face.mNumIndices >= 3) {
            calcTangent(vertices[face.mIndices[0]], vertices[face.mIndices[1]], vertices[face.mIndices[2]], tangent, bitangent);
        }
        for (unsigned int j = 0; j < face.mNumIndices; j ++) {
            int idx = face.mIndices[j];
            indices.push_back(idx);
            tangents[idx].push_back(tangent);
            bitangents[idx].push_back(bitangent);
        }
    }
//    for (unsigned int i = 0; i < vertices.size(); ++i) {
//        int id = i;
//        glm::vec3 avgT = glm::vec3(0.0f);
//        glm::vec3 avgB = glm::vec3(0.0f);
//        unsigned int size = tangents[i].size();
//        for (unsigned int j = 0; j < size; ++j) {
//            avgT += tangents[id][j];
//            avgB += bitangents[id][j];
//        }
//        avgT /= size;
//        avgB /= size;
//        vertices[id].Tangent = avgT;
//        vertices[id].Bitangent = avgB;
//    }
    
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
            if (type == aiTextureType_DIFFUSE) {
                texture.id = loadTexture(imgPath.c_str(), true);
            } else {
                texture.id = loadTexture(imgPath.c_str(), false);
            }
            texture.type = typeName;
            texture.path = str.C_Str();
            textures.push_back(texture);
            loadedTextures.push_back(texture); // add to loaded textures
        }
    }
    return textures;
}

void Model::calcTangent(Vertex v1, Vertex v2, Vertex v3, glm::vec3& tangent, glm::vec3& bitangent) {
    glm::vec3 edge1 = v2.Position - v1.Position;
    glm::vec3 edge2 = v3.Position - v1.Position;
    glm::vec2 deltaUV1 = v2.TexCoords - v1.TexCoords;
    glm::vec2 deltaUV2 = v3.TexCoords - v1.TexCoords;
    float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
    tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
    tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
    tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
    tangent = glm::normalize(tangent);
    
    bitangent.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
    bitangent.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
    bitangent.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
    bitangent = glm::normalize(bitangent);
}
