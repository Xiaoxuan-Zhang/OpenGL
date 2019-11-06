#ifndef SHADER_HPP
#define SHADER_HPP

#include <glm/glm.hpp> //3D Math library
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Shader {
public:
    // the program ID
    unsigned int ID;
    // constructor reads and builds the shader
    Shader(const char * geometry_file_path, const char * vertex_file_path,const char * fragment_file_path);
    Shader(const char * vertex_file_path,const char * fragment_file_path);
    ~Shader();
    // use/activate the shader
    void use();
    // utility uniform functions
    void setBool(const char * name, bool value) const;
    void setInt(const char * name, int value) const;
    void setFloat(const char * name, float value) const;
    void setMat4(const char * name, glm::mat4 value) const;
    void setVec3(const char * name, glm::vec3 value) const;
    void setVec2(const char * name, glm::vec2 value) const;
private:
    GLuint loadShader(const char * shader_file_path, GLenum shaderType);
};

#endif
