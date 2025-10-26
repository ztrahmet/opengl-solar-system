#include "shader.h"

Shader::Shader(const char* vertexPath, const char* fragmentPath) {
    // Constructor code remains the same...
    // 1. Retrieve the vertex/fragment source code from filePath
    std::string vertexCode;
    std::string fragmentCode;
    std::ifstream vShaderFile;
    std::ifstream fShaderFile;
    vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try {
        vShaderFile.open(vertexPath); fShaderFile.open(fragmentPath);
        std::stringstream vShaderStream, fShaderStream;
        vShaderStream << vShaderFile.rdbuf(); fShaderStream << fShaderFile.rdbuf();
        vShaderFile.close(); fShaderFile.close();
        vertexCode = vShaderStream.str(); fragmentCode = fShaderStream.str();
    } catch (std::ifstream::failure& e) {
        std::cerr << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " << e.what() << " (" << vertexPath << ", " << fragmentPath << ")" << std::endl;
        // Optionally throw or exit here
        ID = 0; // Indicate failure
        return;
    }
    const char* vShaderCode = vertexCode.c_str();
    const char * fShaderCode = fragmentCode.c_str();
    // 2. Compile shaders
    unsigned int vertex, fragment;
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, NULL);
    glCompileShader(vertex);
    checkCompileErrors(vertex, "VERTEX");
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, NULL);
    glCompileShader(fragment);
    checkCompileErrors(fragment, "FRAGMENT");
    // Shader Program
    ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
    glLinkProgram(ID);
    checkCompileErrors(ID, "PROGRAM");
    // Delete shaders
    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

void Shader::use() {
     if(ID != 0) glUseProgram(ID);
}
void Shader::setBool(const std::string &name, bool value) const { if(ID != 0) glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value); }
void Shader::setInt(const std::string &name, int value) const { if(ID != 0) glUniform1i(glGetUniformLocation(ID, name.c_str()), value); }
void Shader::setFloat(const std::string &name, float value) const { if(ID != 0) glUniform1f(glGetUniformLocation(ID, name.c_str()), value); }
void Shader::setVec3(const std::string &name, const glm::vec3 &value) const { if(ID != 0) glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]); }
void Shader::setVec3(const std::string &name, float x, float y, float z) const { if(ID != 0) glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z); }

// --- ADDED setMat3 ---
void Shader::setMat3(const std::string &name, const glm::mat3 &mat) const {
    if(ID != 0) glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}
// --- END ADDED ---

void Shader::setMat4(const std::string &name, const glm::mat4 &mat) const { if(ID != 0) glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]); }

// checkCompileErrors remains the same...
void Shader::checkCompileErrors(unsigned int shader, std::string type) {
    GLint success;
    GLchar infoLog[1024];
    if(type != "PROGRAM") {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if(!success) { glGetShaderInfoLog(shader, 1024, NULL, infoLog); std::cerr << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl; }
    } else {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if(!success) { glGetProgramInfoLog(shader, 1024, NULL, infoLog); std::cerr << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl; }
    }
}
