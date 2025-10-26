#ifndef SCENARIO_H
#define SCENARIO_H

#include <string>
#include <vector>
#include <optional>
#include <memory> // Needed for unique_ptr
#include <glm/glm.hpp>
#include "planet.h" // Full definition needed for unique_ptr member

// Forward declaration (if needed elsewhere, though including planet.h is better here)
// class Planet;

// Structure to hold properties of a celestial body
struct CelestialBody {
    std::string name;
    float radius = 1.0f;
    std::string texturePath;
    unsigned int textureID = 0; // OpenGL texture ID, loaded later
    bool isEmissive = false;
    std::optional<std::string> parentName = std::nullopt; // Name of the body it orbits, if any
    float orbitRadius = 0.0f;
    float orbitSpeed = 0.0f;
    float rotationSpeed = 0.0f;
    glm::vec3 rotationAxis = glm::vec3(0.0f, 1.0f, 0.0f);
    std::unique_ptr<Planet> mesh = nullptr; // The sphere mesh (using unique_ptr for ownership)
    glm::mat4 currentModelMatrix = glm::mat4(1.0f); // Current matrix after transformations

    // Constructor (example - adjust as needed)
    CelestialBody(std::string n, float r, std::string tex, bool emissive,
                  float orbRad, float orbSpd, float rotSpd, glm::vec3 rotAxis,
                  std::optional<std::string> parent = std::nullopt)
        : name(std::move(n)), radius(r), texturePath(std::move(tex)), isEmissive(emissive),
          parentName(std::move(parent)), orbitRadius(orbRad), orbitSpeed(orbSpd),
          rotationSpeed(rotSpd), rotationAxis(rotAxis) {}


    // Explicitly default the default constructor
    CelestialBody() = default;

    // Destructor MUST be defined out-of-line in the .cpp file
    // where Planet is fully defined if using unique_ptr<Planet>.
    ~CelestialBody();

    // Rule of 5/3: Since we have unique_ptr, manage copy/move operations
    CelestialBody(const CelestialBody&) = delete; // No copying
    CelestialBody& operator=(const CelestialBody&) = delete; // No copying
    CelestialBody(CelestialBody&&) noexcept = default; // Default move is fine
    CelestialBody& operator=(CelestialBody&&) noexcept = default; // Default move is fine

};

// Structure to hold the entire scene definition
struct Scenario {
    std::vector<CelestialBody> bodies;
    glm::vec3 initialCameraPos = glm::vec3(0.0f, 0.0f, 10.0f);
    glm::vec3 lightPos = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
};

// Function to load a specific scenario
Scenario loadScenario_SolarSystemBasic();

#endif // SCENARIO_H
