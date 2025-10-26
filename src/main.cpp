/**
 * @file main.cpp
 * @brief Main application entry point for the OpenGL Solar System simulation.
 * Initializes GLFW, GLAD, ImGui, loads the scenario, textures, shaders,
 * and runs the main render loop. Handles input processing and updates.
 */

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "stb_image.h" // For loading textures (implementation in stb_image.cpp)

#include "shader.h"   // For loading and managing GLSL shaders
#include "camera.h"   // For managing the camera view and movement
#include "config.h"   // For loading window/simulation settings
#include "planet.h"   // Include full Planet definition BEFORE scenario.h
#include "scenario.h" // For defining the celestial bodies and scene parameters

#include "imgui.h"              // Immediate mode GUI library
#include "imgui_impl_glfw.h"    // ImGui backend for GLFW
#include "imgui_impl_opengl3.h" // ImGui backend for OpenGL 3

#include <iostream>  // For standard I/O (like cerr)
#include <string>    // For using std::string
#include <vector>    // For std::vector (used for cubemap faces)
#include <memory>    // For std::unique_ptr (used in Scenario)
#include <thread>    // For std::this_thread::sleep_for
#include <chrono>    // For std::chrono::milliseconds
#include <optional>  // For std::optional (used for parentName in CelestialBody)
#include <map>       // For std::map (used to look up bodies by name)
#include <algorithm> // For std::clamp, std::max, std::find, std::distance

// --- Function Prototypes ---
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
unsigned int loadTexture(const char *path);
unsigned int loadCubemap(std::vector<std::string> faces);
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
void lockCameraToBody(const std::string &name);

// --- Global Variables ---

// Configuration loaded from config.ini
Config config;
unsigned int SCR_WIDTH;
unsigned int SCR_HEIGHT;

// Camera object
Camera camera;

// Mouse state for camera control
float lastX;
float lastY;
bool firstMouse = true;

// Frame timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// FPS calculation
double lastTimeForFPS = 0.0;
int nbFrames = 0;

// Fullscreen state management
bool fullscreen = false;
bool f11_pressed = false;                                                                         // Prevents toggling repeatedly if F11 is held
int last_window_x = 100, last_window_y = 100, last_window_width = 1280, last_window_height = 720; // Windowed mode fallback

// Simulation control
float simulationSpeed = 1.0f;    // Multiplier for animation speed
float accumulatedSimTime = 0.0f; // Tracks total simulation time elapsed, adjusted by speed

// Camera locking state
CelestialBody *cameraLockedTo = nullptr;        // Pointer to the body the camera is locked on, or nullptr
std::string lockedBodyName = "None";            // Name of the locked body for display
std::map<std::string, CelestialBody *> bodyMap; // Map for easy lookup of bodies by name
std::vector<std::string> lockablePlanetNames;   // Order for cycling through planets with 'P' key
int currentLockIndex = -1;                      // Index into lockablePlanetNames for cycling

// Locked camera parameters (orbit mode)
float lockedCameraDistance = 10.0f;  // Distance from the locked body
float lockedCameraOrbitYaw = -90.0f; // Horizontal angle around the body
float lockedCameraOrbitPitch = 0.0f; // Vertical angle around the body

// Skybox vertex data (simple cube)
float skyboxVertices[] = {
    // positions
    -1.0f, 1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f,
    1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f,

    -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f,
    -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f,

    1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, 1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f, 1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f,

    -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f,
    1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f};

/**
 * @brief Locks the camera to orbit the celestial body with the given name.
 * Resets orbit distance, FOV, and initial orbit angles.
 * @param name The name of the CelestialBody to lock onto.
 */
void lockCameraToBody(const std::string &name)
{
    auto it = bodyMap.find(name);
    if (it != bodyMap.end())
    {
        cameraLockedTo = it->second;
        lockedBodyName = name;
        lockedCameraDistance = cameraLockedTo->radius * 5.0f; // Set initial distance relative to body size
        camera.Zoom = ZOOM;                                   // Reset zoom (FOV) to default

        // Initialize orbit angles based on current camera view when locking
        // This makes the transition smoother
        glm::vec3 direction = glm::normalize(camera.Position - glm::vec3(cameraLockedTo->currentModelMatrix[3]));
        lockedCameraOrbitYaw = glm::degrees(atan2(direction.z, direction.x));
        lockedCameraOrbitPitch = glm::degrees(asin(direction.y));
        lockedCameraOrbitPitch = std::clamp(lockedCameraOrbitPitch, -89.0f, 89.0f); // Prevent looking straight up/down initially

        // Update the index used for cycling through planets ('P' key)
        auto lockIt = std::find(lockablePlanetNames.begin(), lockablePlanetNames.end(), name);
        if (lockIt != lockablePlanetNames.end())
        {
            currentLockIndex = std::distance(lockablePlanetNames.begin(), lockIt);
        }
        else
        {
            currentLockIndex = -1; // Indicate it's not part of the 'P' cycle (e.g., Earth/Mars)
        }
    }
}

/**
 * @brief Main application function.
 */
int main()
{
    // --- Initialization ---
    config = loadConfig("config.ini");
    SCR_WIDTH = config.width;
    SCR_HEIGHT = config.height;
    fullscreen = config.startFullscreen;
    last_window_width = config.width;
    last_window_height = config.height; // Store initial windowed size
    lastX = SCR_WIDTH / 2.0f;
    lastY = SCR_HEIGHT / 2.0f; // Center mouse initially

    // Initialize GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    const char *glsl_version = "#version 330 core";      // GLSL version for ImGui
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // Required on MacOS

    // Create GLFW window (fullscreen or windowed based on config)
    GLFWmonitor *monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode *mode = glfwGetVideoMode(monitor);
    GLFWwindow *window;
    if (fullscreen)
    {
        window = glfwCreateWindow(mode->width, mode->height, "Solar System", monitor, NULL);
        SCR_WIDTH = mode->width;
        SCR_HEIGHT = mode->height;
    }
    else
    {
        window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Solar System", NULL, NULL);
    }
    if (!window)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwGetWindowPos(window, &last_window_x, &last_window_y); // Store initial windowed position

    // Initialize GLAD (loads OpenGL function pointers)
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Enable VSync (limits framerate to monitor refresh rate)
    glfwSwapInterval(1);
    // Enable depth testing for correct 3D rendering order
    glEnable(GL_DEPTH_TEST);

    // Initialize Dear ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Allow keyboard navigation in ImGui
    io.MouseDrawCursor = false;                           // Don't let ImGui draw its own cursor
    io.ConfigFlags |= ImGuiConfigFlags_NoMouse;           // Disable mouse interaction for ImGui
    ImGui::StyleColorsDark();                             // Set ImGui theme
    ImGui_ImplGlfw_InitForOpenGL(window, false);          // Init ImGui for GLFW (false = don't install callbacks automatically)
    ImGui_ImplOpenGL3_Init(glsl_version);                 // Init ImGui for OpenGL 3

    // Set GLFW callbacks (must be done AFTER ImGui init if install_callbacks=false)
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback);

    // Load the scene description
    Scenario currentScenario = loadScenario_SolarSystemBasic();

    // Populate lookup map and list for camera locking
    // Define the order for the 'P' key cycle
    lockablePlanetNames.push_back("Mercury");
    lockablePlanetNames.push_back("Venus");
    // Earth and Mars are handled by E/M keys, so not added here for 'P' cycle
    lockablePlanetNames.push_back("Jupiter");
    lockablePlanetNames.push_back("Saturn");
    lockablePlanetNames.push_back("Uranus");
    lockablePlanetNames.push_back("Neptune");
    // Build the name->pointer map for fast lookups
    for (auto &body : currentScenario.bodies)
    {
        bodyMap[body.name] = &body;
    }

    // Set initial camera position from scenario
    camera.Position = currentScenario.initialCameraPos;
    camera.updateCameraVectors(); // Ensure camera vectors are consistent

    // Load shaders
    Shader lightingShader("shaders/lighting.vert", "shaders/lighting.frag"); // For planets
    Shader emissiveShader("shaders/emissive.vert", "shaders/emissive.frag"); // For the Sun
    Shader skyboxShader("shaders/skybox.vert", "shaders/skybox.frag");       // For the background

    // Load textures for celestial bodies
    stbi_set_flip_vertically_on_load(true); // Tell stb_image to flip textures vertically (OpenGL expects 0,0 at bottom-left)
    for (auto &body : currentScenario.bodies)
    {
        if (!body.mesh)
        {
            std::cerr << "Error: Mesh not created for " << body.name << std::endl;
            return -1;
        }
        body.textureID = loadTexture(body.texturePath.c_str());
        if (body.textureID == 0)
        {
            std::cerr << "Error: Failed texture load for " << body.name << std::endl;
            return -1;
        }
    }

    // Set up skybox VAO and VBO
    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0); // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glBindVertexArray(0); // Unbind

    // Load skybox cubemap texture
    std::vector<std::string> faces = {// Order: +X, -X, +Y, -Y, +Z, -Z
                                      "textures/skybox/right.jpg", "textures/skybox/left.jpg",
                                      "textures/skybox/top.jpg", "textures/skybox/bottom.jpg",
                                      "textures/skybox/front.jpg", "textures/skybox/back.jpg"};
    stbi_set_flip_vertically_on_load(false); // Cubemaps often don't need flipping
    unsigned int cubemapTexture = loadCubemap(faces);
    stbi_set_flip_vertically_on_load(true); // Flip back for regular textures

    // Set initial texture units for shaders
    lightingShader.use();
    lightingShader.setInt("ourTexture", 0); // Use texture unit 0
    emissiveShader.use();
    emissiveShader.setInt("ourTexture", 0); // Use texture unit 0
    skyboxShader.use();
    skyboxShader.setInt("skybox", 0); // Use texture unit 0

    // Initialize timing and lighting variables
    lastTimeForFPS = glfwGetTime();
    lastFrame = (float)lastTimeForFPS;
    accumulatedSimTime = (float)lastTimeForFPS; // Start simulation time from current time
    glm::vec3 lightPos = currentScenario.lightPos;
    glm::vec3 lightColor = currentScenario.lightColor;

    // Hide and capture the mouse cursor
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // --- Main Render Loop ---
    while (!glfwWindowShouldClose(window))
    {
        // --- Timing ---
        double currentFrameTime = glfwGetTime();
        deltaTime = (float)currentFrameTime - lastFrame;
        lastFrame = (float)currentFrameTime;
        float simDeltaTime = deltaTime * simulationSpeed; // Time step adjusted by simulation speed
        accumulatedSimTime += simDeltaTime;               // Accumulate simulation time

        // Calculate and display FPS in window title once per second
        nbFrames++;
        if (currentFrameTime - lastTimeForFPS >= 1.0)
        {
            std::string title = "Solar System - FPS: " + std::to_string(nbFrames);
            glfwSetWindowTitle(window, title.c_str());
            nbFrames = 0;
            lastTimeForFPS = currentFrameTime;
        }

        // --- Input ---
        glfwPollEvents();     // Check for window events (close, resize, etc.)
        processInput(window); // Handle keyboard input for camera/simulation

        // --- ImGui Frame Setup ---
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // --- Clear Buffers ---
        glClearColor(0.01f, 0.01f, 0.01f, 1.0f); // Dark background
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // --- Camera Update ---
        glm::vec3 currentCameraTargetPos = glm::vec3(0.0f); // World position of the locked body
        glm::mat4 view;
        if (cameraLockedTo)
        {
            // Camera is locked - calculate orbit position and view matrix
            currentCameraTargetPos = glm::vec3(cameraLockedTo->currentModelMatrix[3]); // Get target's world position

            // Adjust distance based on scroll wheel input (clamped)
            lockedCameraDistance = std::clamp(lockedCameraDistance, cameraLockedTo->radius * 1.5f, 50.0f * cameraLockedTo->radius);

            // Calculate camera position in spherical coordinates around the target
            float camX = currentCameraTargetPos.x + lockedCameraDistance * cos(glm::radians(lockedCameraOrbitPitch)) * cos(glm::radians(lockedCameraOrbitYaw));
            float camY = currentCameraTargetPos.y + lockedCameraDistance * sin(glm::radians(lockedCameraOrbitPitch));
            float camZ = currentCameraTargetPos.z + lockedCameraDistance * cos(glm::radians(lockedCameraOrbitPitch)) * sin(glm::radians(lockedCameraOrbitYaw));
            camera.Position = glm::vec3(camX, camY, camZ); // Set the camera's position

            // Create the view matrix looking at the target
            view = glm::lookAt(camera.Position, currentCameraTargetPos, camera.WorldUp);

            // Update camera's internal orientation vectors to match the locked view
            camera.Front = glm::normalize(currentCameraTargetPos - camera.Position);
            camera.Right = glm::normalize(glm::cross(camera.Front, camera.WorldUp));
            camera.Up = glm::normalize(glm::cross(camera.Right, camera.Front));
            // Recalculate Yaw/Pitch from the Front vector for consistency if needed later
            camera.Yaw = glm::degrees(atan2(camera.Front.z, camera.Front.x));
            camera.Pitch = glm::degrees(asin(camera.Front.y));
        }
        else
        {
            // Camera is in free-fly mode - get view matrix from camera object directly
            view = camera.GetViewMatrix();
        }

        // Calculate projection matrix (perspective)
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f); // Increased far plane for larger scene

        // --- Render Celestial Bodies ---
        for (auto &body : currentScenario.bodies)
        {
            // Calculate Model Matrix for the current body
            glm::mat4 model = glm::mat4(1.0f);
            glm::mat4 orbitTranslation = glm::mat4(1.0f);
            glm::mat4 rotation = glm::mat4(1.0f);

            // Apply orbit if applicable
            if (body.orbitRadius > 0.0f)
            {
                float orbitAngle = accumulatedSimTime * body.orbitSpeed;
                orbitTranslation = glm::translate(glm::mat4(1.0f), glm::vec3(cos(orbitAngle) * body.orbitRadius, 0.0f, sin(orbitAngle) * body.orbitRadius));
            }
            // Apply rotation
            rotation = glm::rotate(glm::mat4(1.0f), accumulatedSimTime * body.rotationSpeed, glm::normalize(body.rotationAxis));

            // --- Hierarchical Transformation ---
            // Calculate the body's final world position and build its model matrix
            glm::vec3 parentPosition = glm::vec3(0.0f);
            // If the body has a parent...
            if (body.parentName)
            {
                // ...find the parent's current world matrix
                auto it = bodyMap.find(*body.parentName);
                if (it != bodyMap.end())
                {
                    // Extract only the translation (position) part of the parent's matrix
                    // This prevents the parent's scale/rotation from affecting the child's orbital distance
                    parentPosition = glm::vec3(it->second->currentModelMatrix[3]);
                }
            }

            // Calculate the final world position: Parent's Position + Orbital Offset
            glm::vec3 finalPosition = parentPosition + glm::vec3(orbitTranslation * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));

            // Construct the final model matrix for this body:
            // 1. Translate to the final world position
            // 2. Apply its own rotation around its center
            // 3. Apply its own scale
            model = glm::translate(glm::mat4(1.0f), finalPosition);
            model = model * rotation;                          // Apply self-rotation after translation
            model = glm::scale(model, glm::vec3(body.radius)); // Apply self-scaling last

            // Store the calculated world matrix for use by children or camera locking
            body.currentModelMatrix = model;
            // --- End Hierarchical Transformation ---

            // Select the appropriate shader (emissive or lighting)
            Shader &currentShader = body.isEmissive ? emissiveShader : lightingShader;
            currentShader.use();

            // Set common uniforms
            currentShader.setMat4("projection", projection);
            currentShader.setMat4("view", view);
            currentShader.setMat4("model", model);

            // Set lighting-specific uniforms only for non-emissive bodies
            if (!body.isEmissive)
            {
                lightingShader.setVec3("lightPos", lightPos);       // Position of the light source (Sun)
                lightingShader.setVec3("viewPos", camera.Position); // Camera's position for specular highlights
                lightingShader.setVec3("lightColor", lightColor);   // Color of the light

                // Calculate and set the normal matrix (for correct lighting on scaled/rotated objects)
                glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(model)));
                lightingShader.setMat3("normalMatrix", normalMatrix);
            }

            // Bind the texture
            glActiveTexture(GL_TEXTURE0); // Activate texture unit 0
            glBindTexture(GL_TEXTURE_2D, body.textureID);

            // Draw the mesh
            if (body.mesh)
            {
                body.mesh->draw(); // Call draw method on the Planet object managed by unique_ptr
            }
        }

        // --- Render Skybox ---
        glDepthFunc(GL_LEQUAL); // Change depth function so depth test passes when values are equal to depth buffer's content
        skyboxShader.use();
        glm::mat4 skyboxView = glm::mat4(glm::mat3(view)); // Remove translation from the view matrix
        skyboxShader.setMat4("view", skyboxView);
        skyboxShader.setMat4("projection", projection);
        // Draw skybox cube
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS); // Set depth function back to default

        // --- Render ImGui UI ---
        ImGui::Begin("Controls");
        ImGui::Text("Sim Speed: %.1fx (Keys 0-4)", simulationSpeed);
        ImGui::Text("Cam Lock: %s (Keys E,M,P,N)", lockedBodyName.c_str());
        ImGui::Separator();
        ImGui::Text("WASD: Move | Spc/Shft: Up/Dn | Ctrl: Sprint");
        ImGui::Text("Mouse: Look/Orbit | Scroll: Zoom");
        ImGui::Text("F11: Fullscr | Esc: Exit");
        ImGui::Separator();
        ImGui::Text("Performance: %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // --- Swap Buffers and Poll Events ---
        glfwSwapBuffers(window);

    } // End of main render loop

    // --- Cleanup ---
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // Delete OpenGL objects
    glDeleteVertexArrays(1, &skyboxVAO);
    glDeleteBuffers(1, &skyboxVBO);
    glDeleteTextures(1, &cubemapTexture);
    for (auto &body : currentScenario.bodies)
    {
        if (body.textureID != 0)
            glDeleteTextures(1, &body.textureID);
        // Note: body.mesh unique_ptr will automatically delete the Planet object and its GL buffers
    }

    // Terminate GLFW
    glfwTerminate();
    return 0;
}

// Define anisotropic filtering constants if not already defined (might be needed on some platforms/headers)
#ifndef GL_TEXTURE_MAX_ANISOTROPY_EXT
#define GL_TEXTURE_MAX_ANISOTROPY_EXT 0x84FE
#endif
#ifndef GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT
#define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT 0x84FF
#endif

/**
 * @brief Loads a 2D texture from file using stb_image.
 * @param path Path to the texture file.
 * @return OpenGL texture ID, or 0 on failure.
 */
unsigned int loadTexture(char const *path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format = GL_RGB; // Default format
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;
        else
        {
            std::cerr << "Texture format error: Unsupported number of components (" << nrComponents << ") in " << path << std::endl;
            stbi_image_free(data);
            glDeleteTextures(1, &textureID); // Clean up allocated texture ID
            return 0;
        }

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D); // Generate mipmaps for better quality at distance

        // Set texture wrapping and filtering options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // Trilinear filtering
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);               // Bilinear filtering

        // Enable Anisotropic Filtering if available (improves clarity at angles)
        if (glfwExtensionSupported("GL_EXT_texture_filter_anisotropic"))
        {
            float maxAniso;
            glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAniso);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAniso);
        }

        stbi_image_free(data); // Free the loaded image data from CPU memory
    }
    else
    {
        std::cerr << "Texture load failure: Failed to load texture at path: " << path << std::endl;
        // stbi_failure_reason() might provide more details if needed
        glDeleteTextures(1, &textureID); // Clean up allocated texture ID
        return 0;
    }

    return textureID;
}

/**
 * @brief Loads 6 textures into a single OpenGL cubemap texture.
 * @param faces Vector of 6 strings, paths to the texture files in order: +X, -X, +Y, -Y, +Z, -Z.
 * @return OpenGL cubemap texture ID, or 0 on failure.
 */
unsigned int loadCubemap(std::vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            GLenum format = GL_RGB;
            if (nrChannels == 1)
                format = GL_RED;
            else if (nrChannels == 3)
                format = GL_RGB;
            else if (nrChannels == 4)
                format = GL_RGBA;
            else
            {
                std::cerr << "Cubemap error: Unsupported number of channels (" << nrChannels << ") in " << faces[i] << std::endl;
                stbi_image_free(data);
                glDeleteTextures(1, &textureID);
                return 0;
            }
            // Note: GL_TEXTURE_CUBE_MAP_POSITIVE_X + i relies on the enum values being sequential
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            std::cerr << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            // stbi_failure_reason() might provide more details
            glDeleteTextures(1, &textureID);
            return 0;
        }
    }
    // Set cubemap texture parameters
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}

/**
 * @brief GLFW callback for key presses. Handles simulation speed, camera locking,
 * fullscreen toggle, and exiting.
 */
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS)
    {
        // --- Simulation Speed Control (Keys 0-4) ---
        if (key >= GLFW_KEY_0 && key <= GLFW_KEY_4)
        {
            float speeds[] = {0.0f, 0.5f, 1.0f, 2.0f, 5.0f};
            simulationSpeed = speeds[key - GLFW_KEY_0];
        }
        // --- Direct Camera Lock (E for Earth, M for Mars) ---
        else if (key == GLFW_KEY_E)
        {
            lockCameraToBody("Earth");
        }
        else if (key == GLFW_KEY_M)
        {
            lockCameraToBody("Mars");
        }
        // --- Cycle Camera Lock (P key) ---
        else if (key == GLFW_KEY_P)
        {
            if (!lockablePlanetNames.empty())
            {
                currentLockIndex++;
                if (currentLockIndex >= static_cast<int>(lockablePlanetNames.size()))
                {
                    currentLockIndex = 0; // Wrap around
                }
                lockCameraToBody(lockablePlanetNames[currentLockIndex]);
            }
        }
        // --- Unlock Camera (N key) ---
        else if (key == GLFW_KEY_N)
        {
            cameraLockedTo = nullptr;
            lockedBodyName = "None";
            currentLockIndex = -1;
            camera.updateCameraVectors();
        }
        // --- Fullscreen Toggle (F11 key) ---
        else if (key == GLFW_KEY_F11 && !f11_pressed)
        {
            fullscreen = !fullscreen;
            f11_pressed = true;
            GLFWmonitor *monitor = glfwGetPrimaryMonitor();
            const GLFWvidmode *mode = glfwGetVideoMode(monitor);
            if (fullscreen)
            {
                glfwGetWindowPos(window, &last_window_x, &last_window_y);
                glfwGetWindowSize(window, &last_window_width, &last_window_height);
                glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
            }
            else
            {
                glfwSetWindowMonitor(window, NULL, last_window_x, last_window_y, last_window_width, last_window_height, 0);
            }
            glfwSwapInterval(1);
        }
        // --- Exit Application (Escape key) ---
        else if (key == GLFW_KEY_ESCAPE)
        {
            glfwSetWindowShouldClose(window, true);
        }
    }
    // Handle key releases (currently only needed for F11 toggle flag)
    else if (action == GLFW_RELEASE)
    {
        if (key == GLFW_KEY_F11)
        {
            f11_pressed = false; // Allow toggling again
        }
    }
}

/**
 * @brief GLFW callback for mouse scroll wheel events. Controls zoom (FOV) in free mode
 * or distance from target in locked mode.
 */
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
    if (cameraLockedTo)
    {
        // Adjust distance from the locked target
        float zoomSensitivity = 0.5f;
        // Scale sensitivity by current distance to make zooming smoother when far away
        lockedCameraDistance -= static_cast<float>(yoffset) * zoomSensitivity * (lockedCameraDistance * 0.1f);
        // Clamp distance to reasonable bounds relative to the planet's radius
        lockedCameraDistance = std::clamp(lockedCameraDistance, cameraLockedTo->radius * 1.5f, 50.0f * cameraLockedTo->radius);
    }
    else
    {
        // Adjust camera Field of View (FOV) for zooming in free look mode
        camera.ProcessMouseScroll(static_cast<float>(yoffset));
    }
}

/**
 * @brief GLFW callback for mouse movement. Controls camera look direction in free mode
 * or orbits the camera around the target in locked mode.
 */
void mouse_callback(GLFWwindow *window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    // Update last mouse position on first call
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    // Calculate mouse offset since last frame
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // Reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    // Scale mouse sensitivity based on FOV (makes looking slower when zoomed in)
    float sensitivityMultiplier = camera.Zoom / 45.0f;                     // 45.0f is default FOV
    sensitivityMultiplier = std::clamp(sensitivityMultiplier, 0.1f, 1.0f); // Limit scaling

    xoffset *= sensitivityMultiplier;
    yoffset *= sensitivityMultiplier;

    if (cameraLockedTo)
    {
        // Update orbit angles based on mouse movement
        float sensitivity = 0.1f; // Base sensitivity for orbiting
        lockedCameraOrbitYaw += xoffset * sensitivity;
        lockedCameraOrbitPitch -= yoffset * sensitivity; // Inverted Y for pitch

        // Clamp pitch to prevent looking directly up or down, which causes instability
        lockedCameraOrbitPitch = std::clamp(lockedCameraOrbitPitch, -89.0f, 89.0f);
    }
    else
    {
        // Update camera orientation in free look mode
        camera.ProcessMouseMovement(xoffset, yoffset);
    }
}

/**
 * @brief Processes keyboard input for camera movement (WASD, Space, Shift, Ctrl)
 * each frame. Only active when the camera is not locked.
 */
void processInput(GLFWwindow *window)
{
    // Movement keys are disabled if camera is locked
    if (!cameraLockedTo)
    {
        // Calculate movement speed based on zoom level and sprint key
        const float baseNormalSpeed = 5.0f;
        const float baseSprintSpeed = 15.0f;
        float speedMultiplier = camera.Zoom / 45.0f;       // Scale speed with FOV
        speedMultiplier = std::max(speedMultiplier, 0.1f); // Ensure minimum speed
        float currentNormalSpeed = baseNormalSpeed * speedMultiplier;
        float currentSprintSpeed = baseSprintSpeed * speedMultiplier;

        // Check for sprint key (Left Control)
        camera.MovementSpeed = (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) ? currentSprintSpeed : currentNormalSpeed;

        // Process movement keys
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            camera.ProcessKeyboard(FORWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            camera.ProcessKeyboard(BACKWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            camera.ProcessKeyboard(LEFT, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            camera.ProcessKeyboard(RIGHT, deltaTime);
        // Absolute vertical movement
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
            camera.Position.y += camera.MovementSpeed * deltaTime;
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
            camera.Position.y -= camera.MovementSpeed * deltaTime;
    }
}

/**
 * @brief GLFW callback for window resize events. Updates the OpenGL viewport
 * and stores the windowed size if not currently fullscreen.
 */
void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    if (height == 0)
        height = 1;                  // Prevent divide by zero
    glViewport(0, 0, width, height); // Set OpenGL drawing region to match window
    SCR_WIDTH = width;
    SCR_HEIGHT = height; // Update global width/height

    // If we're resizing while in windowed mode, update the stored dimensions
    // so we can correctly restore to this size after leaving fullscreen.
    if (!fullscreen)
    {
        last_window_width = width;
        last_window_height = height;
    }
}
