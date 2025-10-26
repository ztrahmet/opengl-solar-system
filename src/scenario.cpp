/**
 * @file scenario.cpp
 * @brief Implements the function to load the basic solar system scenario definition.
 */
#include "scenario.h"
#include "planet.h" // Include the full definition for unique_ptr destructor
#include <vector>
#include <string>
#include <optional>
#include <memory> // For std::make_unique
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp> // For glm::pi
#include <cmath>                 // For basic math

// Destructor implementation (required because unique_ptr<Planet> needs full Planet definition here)
CelestialBody::~CelestialBody() = default;

/**
 * @brief Creates and returns a Scenario object containing the Sun and planets up to Neptune.
 * Defines relative sizes, compressed orbital distances, and relative speeds.
 */
Scenario loadScenario_SolarSystemBasic()
{
    Scenario scenario;
    scenario.initialCameraPos = glm::vec3(0.0f, 5.0f, 20.0f); // Start a bit further out and higher
    scenario.lightPos = glm::vec3(0.0f, 0.0f, 0.0f);          // Sun is the light source at the origin
    scenario.lightColor = glm::vec3(1.0f, 1.0f, 0.9f);        // Slightly yellowish light (not scientific)

    // Define baseline parameters relative to Earth for easier scaling
    float earthRadius = 0.5f;        // Base radius used for scaling other planets
    float earthOrbitRadius = 10.0f;  // Base orbital distance
    float earthOrbitSpeed = 0.5f;    // Base orbital speed
    float earthRotationSpeed = 1.0f; // Base rotation speed

    // --- Define Celestial Bodies ---
    // Sizes are relative to earthRadius.
    // Orbit radii are heavily compressed artistically.
    // Orbit/Rotation speeds are relative to Earth's speeds.

    // Sun
    CelestialBody sun(
        "Sun", 2.0f, "textures/sun.jpg", true,         // Emissive
        0.0f, 0.0f, 0.1f, glm::vec3(0.0f, 1.0f, 0.0f), // Orbit params (none), slow rotation
        std::nullopt                                   // No parent
    );
    sun.mesh = std::make_unique<Planet>(1.0f, 64, 64); // High detail mesh (radius 1.0, scaled later)
    scenario.bodies.push_back(std::move(sun));         // Add to scenario (use std::move for unique_ptr)

    // Mercury
    CelestialBody mercury(
        "Mercury", earthRadius * 0.38f, "textures/mercury.jpg", false,
        4.0f, earthOrbitSpeed * 1.61f, earthRotationSpeed * 0.01f, glm::vec3(0.0f, 1.0f, 0.0f),
        "Sun" // Parent
    );
    mercury.mesh = std::make_unique<Planet>(1.0f, 32, 32); // Lower detail mesh
    scenario.bodies.push_back(std::move(mercury));

    // Venus
    CelestialBody venus(
        "Venus", earthRadius * 0.95f, "textures/venus.jpg", false,
        7.0f, earthOrbitSpeed * 1.18f, earthRotationSpeed * -0.004f, glm::vec3(0.0f, 1.0f, 0.0f), // Retrograde rotation
        "Sun");
    venus.mesh = std::make_unique<Planet>(1.0f, 48, 48);
    scenario.bodies.push_back(std::move(venus));

    // Earth
    CelestialBody earth(
        "Earth", earthRadius, "textures/earth.jpg", false,
        earthOrbitRadius, earthOrbitSpeed, earthRotationSpeed, glm::vec3(0.0f, 1.0f, 0.0f),
        "Sun");
    earth.mesh = std::make_unique<Planet>(1.0f, 64, 64); // High detail mesh
    scenario.bodies.push_back(std::move(earth));

    // Moon
    CelestialBody moon(
        "Moon", earthRadius * 0.27f, "textures/moon.jpg", false,
        earthRadius * 2.0f + 0.5f, earthOrbitSpeed * 2.0f, earthRotationSpeed * 0.1f, glm::vec3(0.0f, 1.0f, 0.0f),
        "Earth" // Orbits Earth
    );
    moon.mesh = std::make_unique<Planet>(1.0f, 32, 32);
    scenario.bodies.push_back(std::move(moon));

    // Mars
    CelestialBody mars(
        "Mars", earthRadius * 0.53f, "textures/mars.jpg", false,
        15.0f, earthOrbitSpeed * 0.81f, earthRotationSpeed * 0.97f, glm::vec3(0.0f, 1.0f, 0.0f),
        "Sun");
    mars.mesh = std::make_unique<Planet>(1.0f, 48, 48);
    scenario.bodies.push_back(std::move(mars));

    // Jupiter
    CelestialBody jupiter(
        "Jupiter", earthRadius * 3.0f, "textures/jupiter.jpg", false,                         // Scaled down significantly for visibility
        25.0f, earthOrbitSpeed * 0.44f, earthRotationSpeed * 2.41f, glm::vec3(0.0f, 1.0f, 0.0f), // Fast rotation
        "Sun");
    jupiter.mesh = std::make_unique<Planet>(1.0f, 64, 64);
    scenario.bodies.push_back(std::move(jupiter));

    // Saturn
    CelestialBody saturn(
        "Saturn", earthRadius * 2.5f, "textures/saturn.jpg", false,                           // Scaled down
        35.0f, earthOrbitSpeed * 0.32f, earthRotationSpeed * 2.25f, glm::vec3(0.0f, 1.0f, 0.0f), // Fast rotation
        "Sun");
    saturn.mesh = std::make_unique<Planet>(1.0f, 64, 64);
    // Note: Rings are not implemented in this simple version
    scenario.bodies.push_back(std::move(saturn));

    // Uranus
    CelestialBody uranus(
        "Uranus", earthRadius * 1.5f, "textures/uranus.jpg", false,                            // Scaled down
        45.0f, earthOrbitSpeed * 0.23f, earthRotationSpeed * -1.40f, glm::vec3(1.0f, 0.0f, 0.0f), // Retrograde, Tilted axis
        "Sun");
    uranus.mesh = std::make_unique<Planet>(1.0f, 48, 48);
    scenario.bodies.push_back(std::move(uranus));

    // Neptune
    CelestialBody neptune(
        "Neptune", earthRadius * 1.4f, "textures/neptune.jpg", false, // Scaled down
        55.0f, earthOrbitSpeed * 0.18f, earthRotationSpeed * 1.49f, glm::vec3(0.0f, 1.0f, 0.0f),
        "Sun");
    neptune.mesh = std::make_unique<Planet>(1.0f, 48, 48);
    scenario.bodies.push_back(std::move(neptune));

    return scenario;
}
