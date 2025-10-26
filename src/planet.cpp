/**
 * @file planet.cpp
 * @brief Implements the Planet class methods for generating and rendering spheres.
 */

#include "planet.h"
#include <vector>
#include <cmath> // For sin, cos

/**
 * @brief Constructor: Generates vertex positions, normals, texture coordinates,
 * and indices for a UV sphere, then uploads this data to OpenGL buffers (VBO, EBO)
 * and configures the vertex attributes within a VAO.
 */
Planet::Planet(float radius, unsigned int rings, unsigned int sectors)
{
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec2> texCoords;
    std::vector<glm::vec3> normals;
    std::vector<unsigned int> indices;

    // Constants for calculating vertex positions based on spherical coordinates
    float const R = 1.0f / (float)(rings - 1);   // Inverse of the number of ring segments
    float const S = 1.0f / (float)(sectors - 1); // Inverse of the number of sector segments

    // Generate vertices, normals, and texture coordinates
    vertices.reserve(rings * sectors);
    texCoords.reserve(rings * sectors);
    normals.reserve(rings * sectors);
    for (unsigned int r = 0; r < rings; ++r)
    {
        for (unsigned int s = 0; s < sectors; ++s)
        {
            // Calculate spherical coordinates (phi, theta)
            float phi = glm::pi<float>() * r * R;
            float theta = 2 * glm::pi<float>() * s * S;

            // Convert spherical to Cartesian coordinates (y-up)
            float y = sin(-glm::pi<float>() / 2 + phi); // Latitude ranges from -pi/2 to +pi/2
            float x = cos(theta) * sin(phi);            // Longitude ranges from 0 to 2*pi
            float z = sin(theta) * sin(phi);

            // Texture coordinates (u, v) - flipped 'u' to correct mirroring
            texCoords.push_back(glm::vec2(1.0f - (s * S), r * R));

            // Vertex position (scaled by radius)
            vertices.push_back(glm::vec3(x, y, z) * radius);

            // Normal vector (for a sphere, it's just the normalized position vector before scaling)
            normals.push_back(glm::vec3(x, y, z));
        }
    }

    // Generate indices for triangle strips (forming quadrilaterals)
    indices.reserve((rings - 1) * (sectors - 1) * 6);
    for (unsigned int r = 0; r < rings - 1; ++r)
    {
        for (unsigned int s = 0; s < sectors - 1; ++s)
        {
            // Indices for the two triangles forming a quad
            // Triangle 1: (r,s), (r, s+1), (r+1, s+1)
            indices.push_back(r * sectors + s);
            indices.push_back(r * sectors + (s + 1));
            indices.push_back((r + 1) * sectors + (s + 1));

            // Triangle 2: (r,s), (r+1, s+1), (r+1, s)
            indices.push_back(r * sectors + s);
            indices.push_back((r + 1) * sectors + (s + 1));
            indices.push_back((r + 1) * sectors + s);
        }
    }
    indexCount = static_cast<unsigned int>(indices.size());

    // Interleave vertex data (Position, Normal, TexCoord) into a single array
    std::vector<float> data;
    data.reserve(vertices.size() * 8); // 3 pos + 3 normal + 2 texCoord = 8 floats per vertex
    for (size_t i = 0; i < vertices.size(); ++i)
    {
        data.push_back(vertices[i].x);
        data.push_back(vertices[i].y);
        data.push_back(vertices[i].z);

        if (normals.size() > i)
        {
            data.push_back(normals[i].x);
            data.push_back(normals[i].y);
            data.push_back(normals[i].z);
        }

        if (texCoords.size() > i)
        {
            data.push_back(texCoords[i].x);
            data.push_back(texCoords[i].y);
        }
    }

    // --- OpenGL Buffer Setup ---
    glGenVertexArrays(1, &VAO); // Generate VAO to store attribute configurations
    glGenBuffers(1, &VBO);      // Generate VBO for vertex data
    glGenBuffers(1, &EBO);      // Generate EBO for index data

    glBindVertexArray(VAO); // Bind the VAO to start configuring it

    // Upload interleaved vertex data to VBO
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), &data[0], GL_STATIC_DRAW);

    // Upload index data to EBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    // --- Configure Vertex Attributes ---
    // Calculate the stride between consecutive vertices in the interleaved array
    GLsizei stride = (3 + 3 + 2) * sizeof(float); // Pos(3) + Normal(3) + TexCoord(2)

    // Attribute 0: Vertex Position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void *)0); // 3 floats, starting at offset 0

    // Attribute 1: Vertex Normal
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void *)(3 * sizeof(float))); // 3 floats, starting after position data

    // Attribute 2: Vertex Texture Coordinates
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void *)(6 * sizeof(float))); // 2 floats, starting after normal data

    glBindVertexArray(0); // Unbind VAO to prevent accidental changes
    // Buffers (VBO, EBO) remain associated with the VAO even after unbinding the VAO itself
}

/**
 * @brief Destructor: Cleans up the OpenGL buffer objects (VAO, VBO, EBO).
 */
Planet::~Planet()
{
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}

/**
 * @brief Renders the sphere by binding its VAO and issuing a draw call.
 */
void Planet::draw()
{
    glBindVertexArray(VAO); // Bind the VAO containing the mesh data and attribute configuration
    // Draw the triangles using the indices stored in the EBO
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0); // Unbind the VAO
}
