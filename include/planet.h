#ifndef PLANET_H
#define PLANET_H

#include <glad/glad.h>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

class Planet {
public:
    // Constructor
    Planet(float radius, unsigned int rings, unsigned int sectors);
    
    // Destructor
    ~Planet();

    // Renders the sphere
    void draw();

private:
    unsigned int VAO, VBO, EBO;
    unsigned int indexCount;
};

#endif
