/**
 * @file planet.h
 * @brief Defines the Planet class for generating and rendering sphere meshes.
 */

#ifndef PLANET_H
#define PLANET_H

#include <glad/glad.h>           // OpenGL types
#include <vector>                // For std::vector
#include <glm/glm.hpp>           // Vector/math types
#include <glm/gtc/constants.hpp> // For glm::pi

/**
 * @class Planet
 * @brief Generates vertex data for a UV sphere and manages the corresponding
 * OpenGL Vertex Array Object (VAO), Vertex Buffer Object (VBO), and
 * Element Buffer Object (EBO) for rendering.
 */
class Planet
{
public:
    /**
     * @brief Constructor that generates sphere vertex data and uploads it to the GPU.
     * @param radius The radius of the sphere.
     * @param rings The number of latitudinal rings (stacks). Affects vertical smoothness.
     * @param sectors The number of longitudinal sectors (slices). Affects horizontal smoothness.
     */
    Planet(float radius, unsigned int rings, unsigned int sectors);

    /**
     * @brief Destructor that cleans up the OpenGL buffer objects.
     */
    ~Planet();

    /**
     * @brief Renders the sphere mesh by binding its VAO and calling glDrawElements.
     */
    void draw();

private:
    unsigned int VAO;        // Vertex Array Object ID
    unsigned int VBO;        // Vertex Buffer Object ID
    unsigned int EBO;        // Element Buffer Object ID (for indices)
    unsigned int indexCount; // Number of indices to draw
};

#endif // PLANET_H
