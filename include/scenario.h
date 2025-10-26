#ifndef SCENARIO_H
#define SCENARIO_H

#include <string>
#include <vector>
#include <glm/glm.hpp>
#include "shader.h" // Needed for Shader reference
#include <memory>   // std::unique_ptr
#include "planet.h" // Forward declaration might not be enough if unique_ptr needs the full type

// Represents a single celestial body (Sun, Planet, Moon)
struct CelestialBody {
    std::string name;
    float radius;
    std::string texturePath;
    glm::vec3 initialPosition; // Position at time t=0
    bool isEmissive;           // Does it use the emissive shader (like the Sun)?

    // --- Data loaded/used at runtime ---
    unsigned int textureID = 0; // OpenGL texture ID
    std::unique_ptr<Planet> mesh = nullptr; // The sphere mesh (using unique_ptr for ownership)

    // Constructor to simplify creation
    CelestialBody(std::string n, float r, std::string tex, glm::vec3 pos, bool emissive)
        : name(std::move(n)), radius(r), texturePath(std::move(tex)), initialPosition(pos), isEmissive(emissive) {}

    // Destructor needed for unique_ptr to incomplete type if Planet declaration is moved out
     ~CelestialBody(); // Need to define this in scenario.cpp

    // Add move constructor/assignment for proper unique_ptr handling if needed
    CelestialBody(CelestialBody&&) = default;
    CelestialBody& operator=(CelestialBody&&) = default;

    // Delete copy constructor/assignment because unique_ptr cannot be copied
    CelestialBody(const CelestialBody&) = delete;
    CelestialBody& operator=(const CelestialBody&) = delete;
};

// Represents the entire scene/scenario
struct Scenario {
    std::vector<CelestialBody> bodies;
    glm::vec3 lightPos = glm::vec3(0.0f);   // Position of the primary light source
    glm::vec3 lightColor = glm::vec3(1.0f); // Color of the primary light source
    glm::vec3 initialCameraPos = glm::vec3(0.0f, 5.0f, 20.0f); // Sensible starting camera position
    // Add other scene-wide parameters here later (e.g., skybox path)
};

// Function declaration to load our specific solar system scenario
Scenario loadScenario_SolarSystemBasic();

#endif // SCENARIO_H
