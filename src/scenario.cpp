#include "scenario.h"
#include "planet.h"
#include <vector>
#include <string>
#include <memory>
#include <optional> // Include optional header

// Definition for CelestialBody destructor.
CelestialBody::~CelestialBody() = default;

// Defines the initial state of our basic solar system
Scenario loadScenario_SolarSystemBasic() {
    Scenario scene;

    // --- Define Celestial Bodies ---
    // Using slightly more realistic relative sizes and compressed distances.
    // Speeds are arbitrary for visual effect.

    // Sun (Emissive Light Source, Center of Orbit)
    float sunRadius = 2.0f;
    scene.bodies.emplace_back(
        "Sun",                      // Name
        sunRadius,                  // Radius
        "textures/sun.jpg",         // Texture Path
        true,                       // Is Emissive
        0.0f,                       // Orbit Radius (orbits nothing)
        0.0f,                       // Orbit Speed
        0.05f,                      // Rotation Speed (slow)
        glm::vec3(0.0f, 1.0f, 0.0f) // Rotation Axis (Y-axis)
        // No parent
    );
    scene.lightPos = glm::vec3(0.0f, 0.0f, 0.0f); // Sun is the light source

    // Earth
    float earthRadius = 0.5f;
    float earthOrbitRadius = 10.0f;
    float earthOrbitSpeed = 0.2f;
    float earthRotationSpeed = 1.0f;
    scene.bodies.emplace_back(
        "Earth",                    // Name
        earthRadius,                // Radius
        "textures/earth.jpg",       // Texture Path
        false,                      // Is Not Emissive
        earthOrbitRadius,           // Orbit Radius
        earthOrbitSpeed,            // Orbit Speed
        earthRotationSpeed,         // Rotation Speed
        glm::vec3(0.0f, 1.0f, 0.0f), // Rotation Axis
        "Sun"                       // Parent
    );

    // Mars
    float marsRadius = 0.3f; // Roughly half of Earth's radius
    float marsOrbitRadius = 15.0f; // Further out than Earth
    float marsOrbitSpeed = 0.15f; // Slower orbit than Earth
    float marsRotationSpeed = 0.9f; // Similar rotation speed to Earth
    scene.bodies.emplace_back(
        "Mars",                     // Name
        marsRadius,                 // Radius
        "textures/mars.jpg",        // Texture Path
        false,                      // Is Not Emissive
        marsOrbitRadius,            // Orbit Radius
        marsOrbitSpeed,             // Orbit Speed
        marsRotationSpeed,          // Rotation Speed
        glm::vec3(0.0f, 1.0f, 0.0f), // Rotation Axis
        "Sun"                       // Parent
    );

     // Moon (Orbits Earth)
    float moonRadius = 0.15f; // Smaller than Earth
    float moonOrbitRadius = 1.5f; // Orbit radius around Earth
    float moonOrbitSpeed = 1.5f; // Orbits Earth relatively quickly
    float moonRotationSpeed = 0.0f; // Tidally locked (or very slow rotation)
    scene.bodies.emplace_back(
        "Moon",                     // Name
        moonRadius,                 // Radius
        "textures/moon.jpg",        // Texture Path
        false,                      // Is Not Emissive
        moonOrbitRadius,            // Orbit Radius (around Earth)
        moonOrbitSpeed,             // Orbit Speed (around Earth)
        moonRotationSpeed,          // Rotation Speed
        glm::vec3(0.0f, 1.0f, 0.0f), // Rotation Axis
        "Earth"                     // Parent is Earth
    );


    // --- Set Initial Camera Position ---
    scene.initialCameraPos = glm::vec3(0.0f, 5.0f, 20.0f);

    // --- Load Meshes ---
    for (auto& body : scene.bodies) {
        body.mesh = std::make_unique<Planet>(body.radius, 64, 64);
    }

    return scene;
}
