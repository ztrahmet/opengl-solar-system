#include "scenario.h"
#include "planet.h" // Include Planet class definition for mesh creation

#include <vector>
#include <string>
#include <memory> // For std::make_unique

// Definition for CelestialBody destructor, required for std::unique_ptr<Planet>.
CelestialBody::~CelestialBody() = default;

// Defines the initial state of our basic solar system
Scenario loadScenario_SolarSystemBasic() {
    Scenario scene;

    // --- Define Celestial Bodies ---
    // Note: Distances and sizes are NOT to astronomical scale,
    // they are adjusted for visibility in the simulation.

    // Sun (Emissive Light Source)
    float sunRadius = 2.0f;
    scene.bodies.emplace_back(
        "Sun",                      // Name
        sunRadius,                  // Radius
        "textures/sun.jpg",         // Texture Path
        glm::vec3(0.0f, 0.0f, 0.0f),// Initial Position (Origin)
        true                        // Is Emissive
    );
    scene.lightPos = glm::vec3(0.0f, 0.0f, 0.0f); // Sun is the light source

    // Earth
    float earthRadius = 0.5f; // Much smaller than the sun
    float earthDistance = 10.0f; // Further away than before
     scene.bodies.emplace_back(
        "Earth",                    // Name
        earthRadius,                // Radius
        "textures/earth.jpg",       // Texture Path
        glm::vec3(earthDistance, 0.0f, 0.0f), // Initial Position (along X-axis)
        false                       // Is Not Emissive (uses lighting shader)
    );

    // --- Set Initial Camera Position ---
    scene.initialCameraPos = glm::vec3(0.0f, 5.0f, 20.0f); // Start further back

    // --- Load Meshes (Textures are loaded in main.cpp) ---
    for (auto& body : scene.bodies) {
        // Create the sphere mesh for each body based on its radius
        body.mesh = std::make_unique<Planet>(body.radius, 64, 64);
    }


    return scene;
}
