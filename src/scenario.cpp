#include "scenario.h"
#include "planet.h" // Needed for unique_ptr<Planet>
#include <memory>  // Needed for unique_ptr
#include <vector>

// Explicitly define CelestialBody destructor here where Planet is fully defined
CelestialBody::~CelestialBody() = default;


Scenario loadScenario_SolarSystemBasic() {
    Scenario scene;

    scene.initialCameraPos = glm::vec3(0.0f, 2.0f, 15.0f);
    scene.lightPos = glm::vec3(0.0f, 0.0f, 0.0f);
    scene.lightColor = glm::vec3(1.0f, 1.0f, 0.9f);

    // --- Sun ---
    CelestialBody sun;
    sun.name = "Sun";
    sun.radius = 2.0f;
    sun.texturePath = "textures/sun.jpg";
    sun.isEmissive = true;
    sun.orbitRadius = 0.0f;
    sun.orbitSpeed = 0.0f;
    sun.rotationSpeed = 0.05f;
    sun.rotationAxis = glm::vec3(0.0f, 1.0f, 0.0f);
    sun.mesh = std::make_unique<Planet>(1.0f, 64, 64);
    scene.bodies.push_back(std::move(sun));

    // --- Earth ---
    CelestialBody earth;
    earth.name = "Earth";
    earth.radius = 0.5f;
    earth.texturePath = "textures/earth.jpg";
    earth.isEmissive = false;
    earth.orbitRadius = 10.0f;
    earth.orbitSpeed = 0.2f;
    earth.rotationSpeed = 0.5f;
    earth.rotationAxis = glm::vec3(0.0f, 1.0f, 0.0f);
    earth.mesh = std::make_unique<Planet>(1.0f, 48, 48);
    scene.bodies.push_back(std::move(earth));

    // --- Moon ---
    CelestialBody moon;
    moon.name = "Moon";
    moon.radius = 0.15f;
    moon.texturePath = "textures/moon.jpg";
    moon.isEmissive = false;
    moon.parentName = "Earth";
    // Orbit radius = Earth radius + Moon radius + clearance
    moon.orbitRadius = scene.bodies.back().radius + moon.radius + 1.0f; // Approx 0.5 + 0.15 + 1.0 = 1.65
    moon.orbitSpeed = 2.0f;
    moon.rotationSpeed = 0.3f;
    moon.rotationAxis = glm::vec3(0.0f, 1.0f, 0.0f);
    moon.mesh = std::make_unique<Planet>(1.0f, 32, 32);
    scene.bodies.push_back(std::move(moon));

    // --- Mars ---
    CelestialBody mars;
    mars.name = "Mars";
    mars.radius = 0.3f;
    mars.texturePath = "textures/mars.jpg";
    mars.isEmissive = false;
    mars.orbitRadius = 15.0f;
    mars.orbitSpeed = 0.15f;
    mars.rotationSpeed = 0.45f;
    mars.rotationAxis = glm::vec3(0.0f, 1.0f, 0.0f);
    mars.mesh = std::make_unique<Planet>(1.0f, 48, 48);
    scene.bodies.push_back(std::move(mars));

    return scene;
}
