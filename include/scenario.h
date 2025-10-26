/**
 * @file scenario.h
 * @brief Defines structures for representing celestial bodies and the overall scene scenario.
 */

#ifndef SCENARIO_H
#define SCENARIO_H

#include <string>
#include <vector>
#include <optional>    // For optional parent name
#include <memory>      // For std::unique_ptr
#include <glm/glm.hpp> // Vector types

// Forward declaration of Planet class to avoid circular dependency
// Include the full "planet.h" in scenario.cpp where unique_ptr needs the definition.
class Planet;
class Shader; // Forward declaration

/**
 * @struct CelestialBody
 * @brief Represents a single object in the solar system (planet, moon, sun).
 * Holds properties like size, texture, shader, animation parameters, and hierarchy.
 */
struct CelestialBody
{
    std::string name;        // Unique identifier (e.g., "Earth")
    float radius;            // Scaled radius for rendering
    std::string texturePath; // Path to the texture file
    bool isEmissive;         // If true, uses emissive shader (like the Sun)

    // Animation parameters
    float orbitRadius;      // Distance from the parent's center
    float orbitSpeed;       // Speed of orbit around the parent (relative units)
    float rotationSpeed;    // Speed of rotation on its own axis (relative units)
    glm::vec3 rotationAxis; // Axis of rotation

    // Hierarchy
    std::optional<std::string> parentName; // Name of the parent body, if any

    // Rendering data (initialized later)
    unsigned int textureID = 0;                     // OpenGL texture ID
    glm::mat4 currentModelMatrix = glm::mat4(1.0f); // Current world transform matrix, updated each frame
    std::unique_ptr<Planet> mesh = nullptr;         // The sphere mesh (using unique_ptr for ownership and RAII)

    /**
     * @brief Parameterized constructor.
     */
    CelestialBody(std::string n, float r, std::string tex, bool emissive,
                  float orbRad, float orbSpd, float rotSpd, glm::vec3 rotAx,
                  std::optional<std::string> parent)
        : name(std::move(n)), radius(r), texturePath(std::move(tex)), isEmissive(emissive),
          orbitRadius(orbRad), orbitSpeed(orbSpd), rotationSpeed(rotSpd),
          rotationAxis(rotAx), parentName(std::move(parent)) {}

    // Explicitly default the default constructor (needed due to other constructors)
    CelestialBody() = default;

    // Explicitly define/default special member functions because we have unique_ptr
    ~CelestialBody();                                         // Defined in .cpp
    CelestialBody(const CelestialBody &) = delete;            // No copying
    CelestialBody &operator=(const CelestialBody &) = delete; // No copying
    CelestialBody(CelestialBody &&) = default;                // Default move constructor
    CelestialBody &operator=(CelestialBody &&) = default;     // Default move assignment
};

/**
 * @struct Scenario
 * @brief Contains all the elements defining a specific scene setup.
 */
struct Scenario
{
    std::vector<CelestialBody> bodies; // List of all celestial bodies in the scene
    glm::vec3 initialCameraPos;        // Starting position for the camera
    glm::vec3 lightPos;                // Position of the primary light source (usually the Sun)
    glm::vec3 lightColor;              // Color of the primary light source
};

/**
 * @brief Loads the definition for the basic solar system scenario.
 * @return A Scenario struct populated with the Sun, planets, and Moon.
 */
Scenario loadScenario_SolarSystemBasic();

#endif // SCENARIO_H
