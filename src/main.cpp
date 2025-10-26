#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "stb_image.h"

#include "shader.h"
#include "camera.h"
#include "config.h"
#include "scenario.h"

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <thread>
#include <chrono>
#include <optional> // Include optional header

// Function prototypes (same as before)
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
unsigned int loadTexture(const char *path);

// Globals (same as before)
Config config;
unsigned int SCR_WIDTH;
unsigned int SCR_HEIGHT;
Camera camera;
float lastX;
float lastY;
bool firstMouse = true;
float deltaTime = 0.0f;
float lastFrame = 0.0f;
double lastTimeForFPS = 0.0;
int nbFrames = 0;
bool fullscreen = false;
bool f11_pressed = false;
int last_window_x = 100, last_window_y = 100, last_window_width = 1280, last_window_height = 720;


int main() {
    // 1. Load configuration (Same)
    config = loadConfig("config.ini");
    SCR_WIDTH = config.width; SCR_HEIGHT = config.height; fullscreen = config.startFullscreen;
    last_window_width = config.width; last_window_height = config.height;
    lastX = SCR_WIDTH / 2.0f; lastY = SCR_HEIGHT / 2.0f;

    // 2. GLFW Init (Same)
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // 3. Window Creation (Same)
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    GLFWwindow* window;
    if (fullscreen) { window = glfwCreateWindow(mode->width, mode->height, "Solar System", monitor, NULL); SCR_WIDTH=mode->width; SCR_HEIGHT=mode->height; }
    else { window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Solar System", NULL, NULL); }
    if (!window) { std::cerr << "Failed to create GLFW window" << std::endl; glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwGetWindowPos(window, &last_window_x, &last_window_y);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // 4. GLAD Load (Same)
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) { std::cerr << "Failed to initialize GLAD" << std::endl; return -1; }

    // 5. VSync (Same)
    glfwSwapInterval(1);

    // 6. OpenGL State (Same)
    glEnable(GL_DEPTH_TEST);

    // 7. Load Scenario (Same)
    Scenario currentScenario = loadScenario_SolarSystemBasic();

    // 8. Set initial camera position (Same)
    camera.Position = currentScenario.initialCameraPos;

    // 9. Compile Shaders (Same)
    Shader lightingShader("shaders/lighting.vert", "shaders/lighting.frag");
    Shader emissiveShader("shaders/emissive.vert", "shaders/emissive.frag");

    // 10. Load Meshes and Textures (Same - meshes loaded in scenario, textures here)
    stbi_set_flip_vertically_on_load(true);
    for (auto& body : currentScenario.bodies) {
        if (!body.mesh) { std::cerr << "Error: Mesh not created for " << body.name << std::endl; return -1; }
        body.textureID = loadTexture(body.texturePath.c_str());
        if (body.textureID == 0) { std::cerr << "Error: Failed texture load for " << body.name << std::endl; return -1; }
    }

    // 11. Set Texture Units (Same)
    lightingShader.use(); lightingShader.setInt("ourTexture", 0);
    emissiveShader.use(); emissiveShader.setInt("ourTexture", 0);

    lastTimeForFPS = glfwGetTime();
    lastFrame = (float)lastTimeForFPS;

    glm::vec3 lightPos = currentScenario.lightPos; // Usually Sun's position (0,0,0)
    glm::vec3 lightColor = currentScenario.lightColor;

    // --- Render loop ---
    while (!glfwWindowShouldClose(window)) {
        double currentFrameTime = glfwGetTime();
        deltaTime = (float)currentFrameTime - lastFrame;
        lastFrame = (float)currentFrameTime;
        float time = (float)currentFrameTime; // Get current time for animation

        // FPS Counter (Same)
        nbFrames++;
        if (currentFrameTime - lastTimeForFPS >= 1.0) { /* ... update title ... */ std::string title = "Solar System - FPS: " + std::to_string(nbFrames); glfwSetWindowTitle(window, title.c_str()); nbFrames=0; lastTimeForFPS=currentFrameTime; }

        // Input (Same)
        processInput(window);

        // Render Setup (Same)
        glClearColor(0.01f, 0.01f, 0.01f, 1.0f); // Darker background
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f); // Increased far plane
        glm::mat4 view = camera.GetViewMatrix();

        // --- Update and Draw Celestial Bodies ---
        for (auto& body : currentScenario.bodies) {
            // --- Calculate Animation ---
            glm::mat4 model = glm::mat4(1.0f);
            glm::mat4 orbitTranslation = glm::mat4(1.0f);
            glm::mat4 rotation = glm::mat4(1.0f);

            // Calculate orbit position (around Y-axis for simplicity)
            if (body.orbitRadius > 0.0f) {
                float orbitAngle = time * body.orbitSpeed;
                orbitTranslation = glm::translate(orbitTranslation,
                    glm::vec3(cos(orbitAngle) * body.orbitRadius,
                              0.0f,
                              sin(orbitAngle) * body.orbitRadius));
            }

            // Calculate rotation on axis
            rotation = glm::rotate(rotation, time * body.rotationSpeed, glm::normalize(body.rotationAxis));

            // --- Handle Hierarchy ---
            glm::mat4 parentModelMatrix = glm::mat4(1.0f);
            if (body.parentName) {
                CelestialBody* parent = currentScenario.findBody(*body.parentName);
                if (parent) {
                    parentModelMatrix = parent->currentModelMatrix; // Use parent's calculated matrix from this frame
                } else {
                    std::cerr << "Warning: Parent body '" << *body.parentName << "' not found for '" << body.name << "'" << std::endl;
                }
            }

            // Combine transformations: Parent's transform -> Orbit -> Rotation
            model = parentModelMatrix * orbitTranslation * rotation;
            body.currentModelMatrix = model; // Store for children

            // --- Rendering ---
            Shader& currentShader = body.isEmissive ? emissiveShader : lightingShader;
            currentShader.use();
            currentShader.setMat4("projection", projection);
            currentShader.setMat4("view", view);
            currentShader.setMat4("model", model); // Pass the final animated matrix

            if (!body.isEmissive) {
                lightingShader.setVec3("lightPos", lightPos); // Assuming light is at origin
                lightingShader.setVec3("viewPos", camera.Position);
                lightingShader.setVec3("lightColor", lightColor);
                // Calculate normal matrix and pass it (important for non-uniform scaling, good practice)
                 glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(model)));
                 lightingShader.setMat3("normalMatrix", normalMatrix); // Add setMat3 to Shader class if missing
            }

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, body.textureID);
            if (body.mesh) {
                body.mesh->draw();
            }
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup (Same)
    for (auto& body : currentScenario.bodies) {
        if (body.textureID != 0) glDeleteTextures(1, &body.textureID);
    }
    glfwTerminate();
    return 0;
}

// --- Function Implementations ---
// (loadTexture, processInput, framebuffer_size_callback, mouse_callback, scroll_callback - same as before)
// ... [Keep the full implementations of these functions] ...

// Anisotropic filtering definitions
#ifndef GL_TEXTURE_MAX_ANISOTROPY_EXT
#define GL_TEXTURE_MAX_ANISOTROPY_EXT 0x84FE
#endif
#ifndef GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT
#define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT 0x84FF
#endif

// Utility function for loading a 2D texture from file
unsigned int loadTexture(char const * path) {
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data) {
        GLenum format;
        if (nrComponents == 1) format = GL_RED;
        else if (nrComponents == 3) format = GL_RGB;
        else if (nrComponents == 4) format = GL_RGBA;
        else { std::cerr << "Texture format error: " << path << std::endl; stbi_image_free(data); glDeleteTextures(1,&textureID); return 0;}

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        if (glfwExtensionSupported("GL_EXT_texture_filter_anisotropic")) {
            float maxAnisotropy;
            glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropy);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAnisotropy);
        }
        stbi_image_free(data);
    } else {
        std::cerr << "Texture failed to load at path: " << path << std::endl;
        glDeleteTextures(1,&textureID); // Clean up if stbi_load fails
        return 0;
    }
    return textureID;
}

void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    const float normalSpeed = 5.0f;
    const float sprintSpeed = 15.0f;
    camera.MovementSpeed = (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) ? sprintSpeed : normalSpeed;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) camera.ProcessKeyboard(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) camera.Position.y += camera.MovementSpeed * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) camera.Position.y -= camera.MovementSpeed * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_F11) == GLFW_PRESS && !f11_pressed) {
        fullscreen = !fullscreen; f11_pressed = true;
        GLFWmonitor* monitor = glfwGetPrimaryMonitor(); const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        if (fullscreen) {
            glfwGetWindowPos(window, &last_window_x, &last_window_y); glfwGetWindowSize(window, &last_window_width, &last_window_height);
            glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
        } else {
            glfwSetWindowMonitor(window, NULL, last_window_x, last_window_y, last_window_width, last_window_height, 0);
        }
        glfwSwapInterval(1); // Re-apply VSync
    }
    if (glfwGetKey(window, GLFW_KEY_F11) == GLFW_RELEASE) f11_pressed = false;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    if(height == 0) height = 1; // Prevent divide by zero
    glViewport(0, 0, width, height); SCR_WIDTH = width; SCR_HEIGHT = height;
    if (!fullscreen) { last_window_width = width; last_window_height = height; }
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    float xpos = static_cast<float>(xposIn); float ypos = static_cast<float>(yposIn);
    if (firstMouse) { lastX = xpos; lastY = ypos; firstMouse = false; }
    float xoffset = xpos - lastX; float yoffset = lastY - ypos;
    lastX = xpos; lastY = ypos;
    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}
