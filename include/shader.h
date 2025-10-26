/**
 * @file shader.h
 * @brief Defines the Shader class for loading, compiling, and managing GLSL shaders.
 */

#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h> // OpenGL types
#include <glm/glm.hpp> // Matrix/vector types

#include <string>   // For file paths and uniform names
#include <fstream>  // For file reading
#include <sstream>  // For reading file into string
#include <iostream> // For error reporting

/**
 * @class Shader
 * @brief Encapsulates loading GLSL shaders from files, compiling them,
 * linking them into a shader program, and providing utility functions
 * for activating the program and setting uniform variables.
 */
class Shader
{
public:
    // The OpenGL shader program ID
    unsigned int ID;

    /**
     * @brief Constructor that reads shader source from files, compiles, and links them.
     * @param vertexPath Path to the vertex shader source file (.vert).
     * @param fragmentPath Path to the fragment shader source file (.frag).
     */
    Shader(const char *vertexPath, const char *fragmentPath);

    /**
     * @brief Activates this shader program for subsequent rendering calls.
     */
    void use();

    // --- Utility functions for setting uniform variables ---
    // Note: The shader program must be active (use() called) before setting uniforms.

    /** @brief Sets a boolean uniform. */
    void setBool(const std::string &name, bool value) const;
    /** @brief Sets an integer uniform. */
    void setInt(const std::string &name, int value) const;
    /** @brief Sets a float uniform. */
    void setFloat(const std::string &name, float value) const;
    /** @brief Sets a vec3 uniform (using glm::vec3). */
    void setVec3(const std::string &name, const glm::vec3 &value) const;
    /** @brief Sets a vec3 uniform (using 3 float values). */
    void setVec3(const std::string &name, float x, float y, float z) const;
    /** @brief Sets a mat3 uniform (using glm::mat3). */
    void setMat3(const std::string &name, const glm::mat3 &mat) const;
    /** @brief Sets a mat4 uniform (using glm::mat4). */
    void setMat4(const std::string &name, const glm::mat4 &mat) const;

private:
    /**
     * @brief Utility function for checking shader compilation or program linking errors.
     * Prints errors to the console if any occur.
     * @param shader The ID of the shader or program to check.
     * @param type A string indicating the type ("VERTEX", "FRAGMENT", or "PROGRAM").
     */
    void checkCompileErrors(unsigned int shader, std::string type);
};

#endif
