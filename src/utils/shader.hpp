#pragma once
#include <glad/glad.h>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

class Shader {
public:
    Shader(const std::string& vertexSource, const std::string& fragmentSource);
    ~Shader();

    void use();
    unsigned int getProgram() const { return program; }
    
    void setMat4(const std::string& name, const glm::mat4& mat) const {
        glUniformMatrix4fv(glGetUniformLocation(program, name.c_str()), 
                          1, GL_FALSE, glm::value_ptr(mat));
    }
    
    void setVec2(const std::string& name, const glm::vec2& value) const {
        glUniform2fv(glGetUniformLocation(program, name.c_str()), 1, &value[0]);
    }
    
    void setFloat(const std::string& name, float value) const {
        glUniform1f(glGetUniformLocation(program, name.c_str()), value);
    }
    
    void setVec4(const std::string& name, const glm::vec4& value) const {
        glUniform4fv(glGetUniformLocation(program, name.c_str()), 1, &value[0]);
    }

private:
    unsigned int program;
    unsigned int compileShader(unsigned int type, const std::string& source);
};