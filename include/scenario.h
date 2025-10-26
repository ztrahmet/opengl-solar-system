#ifndef SCENARIO_H
#define SCENARIO_H

#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> // Include for matrix functions
#include "shader.h"
#include <memory>
#include "planet.h"
#include <optional> // For optional parent name

// Represents a single celestial body (Sun, Planet, Moon)
struct CelestialBody {
    std::string name;
    float radius;
    std::string texturePath;
    // glm::vec3 initialPosition; // Replaced by orbital parameters
    bool isEmissive;

    // --- Animation Properties ---
    float orbitRadius;        // Distance from the body it orbits (0 for Sun)
    float orbitSpeed;         // Speed of orbit (radians per second, can be negative for direction)
    float rotationSpeed;      // Speed of rotation on axis (radians per second)
    glm::vec3 rotationAxis;     // Axis of rotation (e.g., glm::vec3(0.0f, 1.0f, 0.0f))
    std::optional<std::string> parentName; // Name of the body this one orbits, if any

    // --- Data loaded/used at runtime ---
    unsigned int textureID = 0;
    std::unique_ptr<Planet> mesh = nullptr;
    glm::mat4 currentModelMatrix = glm::mat4(1.0f); // Store the calculated matrix each frame

    // Constructor updated for animation
    CelestialBody(std::string n, float r, std::string tex, bool emissive,
                  float orbRad, float orbSpd, float rotSpd, glm::vec3 rotAx,
                  std::optional<std::string> parent = std::nullopt) // Use std::nullopt for no parent
        : name(std::move(n)), radius(r), texturePath(std::move(tex)), isEmissive(emissive),
          orbitRadius(orbRad), orbitSpeed(orbSpd), rotationSpeed(rotSpd), rotationAxis(rotAx),
          parentName(std::move(parent)) {}

    ~CelestialBody(); // Still defined in scenario.cpp

    CelestialBody(CelestialBody&&) = default;
    CelestialBody& operator=(CelestialBody&&) = default;
    CelestialBody(const CelestialBody&) = delete;
    CelestialBody& operator=(const CelestialBody&) = delete;
};

// Represents the entire scene/scenario
struct Scenario {
    std::vector<CelestialBody> bodies;
    glm::vec3 lightPos = glm::vec3(0.0f);
    glm::vec3 lightColor = glm::vec3(1.0f);
    glm::vec3 initialCameraPos = glm::vec3(0.0f, 5.0f, 20.0f);

     // Helper function to find a body by name (needed for hierarchy)
     // Returns nullptr if not found
    CelestialBody* findBody(const std::string& name) {
        for (auto& body : bodies) {
            if (body.name == name) {
                return &body;
            }
        }
        return nullptr;
    }
};

// Function declaration
Scenario loadScenario_SolarSystemBasic();

#endif // SCENARIO_H
