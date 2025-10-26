#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "stb_image.h"

#include "shader.h"
#include "camera.h"
// #include "planet.h" // Planet is now included via scenario.h indirectly
#include "config.h"
#include "scenario.h" // Include the new scenario header

#include <iostream>
#include <string>
#include <vector>   // Needed for scenario.bodies
#include <memory>   // Needed for unique_ptr in scenario.h
#include <thread>
#include <chrono>

// Function prototypes remain the same
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
unsigned int loadTexture(const char *path); // Keep loadTexture here

// --- Settings ---
Config config;
unsigned int SCR_WIDTH;
unsigned int SCR_HEIGHT;

// Camera - Initial position will be set from the scenario
Camera camera; // Default constructor, position set later
float lastX;
float lastY;
bool firstMouse = true;

// Timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// FPS Counter
double lastTimeForFPS = 0.0;
int nbFrames = 0;

// Fullscreen state
bool fullscreen = false;
bool f11_pressed = false;
int last_window_x = 100;
int last_window_y = 100;
int last_window_width = 1280;
int last_window_height = 720;


int main() {
    // 1. Load configuration
    config = loadConfig("config.ini");
    SCR_WIDTH = config.width;
    SCR_HEIGHT = config.height;
    fullscreen = config.startFullscreen;
    last_window_width = config.width;
    last_window_height = config.height;

    lastX = SCR_WIDTH / 2.0f;
    lastY = SCR_HEIGHT / 2.0f;

    // 2. GLFW: initialize and configure (Same as before)
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // 3. GLFW window creation (Same as before)
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    GLFWwindow* window;
    if (fullscreen) { /* ... */ window = glfwCreateWindow(mode->width, mode->height, "Solar System", monitor, NULL); SCR_WIDTH=mode->width; SCR_HEIGHT=mode->height; } else { /* ... */ window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Solar System", NULL, NULL); }
    if (window == NULL) { /* ... error handling ... */ std::cout << "Failed create window" << std::endl; return -1;}
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwGetWindowPos(window, &last_window_x, &last_window_y);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // 4. GLAD: load all OpenGL function pointers (Same as before)
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) { /* ... error handling ... */ std::cout << "Failed init GLAD" << std::endl; return -1;}

    // 5. Enable VSync (Same as before)
    glfwSwapInterval(1);

    // 6. Configure global opengl state (Same as before)
    glEnable(GL_DEPTH_TEST);

    // --- NEW: Load Scenario ---
    Scenario currentScenario = loadScenario_SolarSystemBasic();

    // --- NEW: Set initial camera position from scenario ---
    camera.Position = currentScenario.initialCameraPos;

    // 7. Build and compile our shader programs (Same as before)
    Shader lightingShader("shaders/lighting.vert", "shaders/lighting.frag");
    Shader emissiveShader("shaders/emissive.vert", "shaders/emissive.frag");

    // 8. Create planet meshes and load textures (Using scenario data)
    stbi_set_flip_vertically_on_load(true);
    for (auto& body : currentScenario.bodies) {
        // Mesh creation is now done in loadScenario_SolarSystemBasic()
        if (!body.mesh) {
             std::cerr << "Error: Mesh not created for body: " << body.name << std::endl;
             glfwTerminate();
             return -1;
        }
        // Load texture here, as we need the OpenGL context
        body.textureID = loadTexture(body.texturePath.c_str());
        if (body.textureID == 0) {
            std::cerr << "Error: Failed to load texture for body: " << body.name << " at path: " << body.texturePath << std::endl;
            // Depending on desired behavior, you might want to return -1 or continue with a default texture
            glfwTerminate();
            return -1;
        }
    }


    // 9. Set texture units (only needs to be done once)
    lightingShader.use();
    lightingShader.setInt("ourTexture", 0); // Both shaders use texture unit 0
    emissiveShader.use();
    emissiveShader.setInt("ourTexture", 0);


    lastTimeForFPS = glfwGetTime();
    lastFrame = (float)lastTimeForFPS;

    // Get light properties from scenario
    glm::vec3 lightPos = currentScenario.lightPos;
    glm::vec3 lightColor = currentScenario.lightColor;

    // 10. Render loop
    while (!glfwWindowShouldClose(window)) {
        double frameStartTime = glfwGetTime();
        deltaTime = (float)frameStartTime - lastFrame;
        lastFrame = (float)frameStartTime;

        // FPS Counter Logic (Same as before)
        nbFrames++;
        if (frameStartTime - lastTimeForFPS >= 1.0) { /* ... update window title ... */ std::string title = "Solar System - FPS: " + std::to_string(nbFrames); glfwSetWindowTitle(window, title.c_str()); nbFrames=0; lastTimeForFPS=frameStartTime;}

        // Input (Same as before)
        processInput(window);

        // Render
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Common Matrices (Same as before)
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();

        // --- NEW: Iterate through bodies in the scenario ---
        for (const auto& body : currentScenario.bodies) {
            // Choose the correct shader
            Shader& currentShader = body.isEmissive ? emissiveShader : lightingShader;
            currentShader.use();

            // Set common uniforms
            currentShader.setMat4("projection", projection);
            currentShader.setMat4("view", view);

            // Set lighting uniforms only for the lighting shader
            if (!body.isEmissive) {
                lightingShader.setVec3("lightPos", lightPos);
                lightingShader.setVec3("viewPos", camera.Position);
                lightingShader.setVec3("lightColor", lightColor);
            }

            // Calculate model matrix (using initial position for now, animation comes later)
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, body.initialPosition);
            // Rotation/Orbit animation will go here in the next milestone
            currentShader.setMat4("model", model);

            // Bind texture and draw
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, body.textureID);
            if (body.mesh) {
                body.mesh->draw();
            }
        }

        // GLFW: swap buffers and poll IO events (Same as before)
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // --- Cleanup ---
    // Textures are deleted manually (or could be managed by the CelestialBody destructor if using RAII)
    for (auto& body : currentScenario.bodies) {
        if (body.textureID != 0) {
            glDeleteTextures(1, &body.textureID);
        }
        // unique_ptr for body.mesh handles mesh VAO/VBO deletion automatically
    }

    glfwTerminate();
    return 0;
}

// --- Function Implementations ---
// (loadTexture, processInput, framebuffer_size_callback, mouse_callback, scroll_callback remain the same)

// Anisotropic filtering definitions (Keep these)
#ifndef GL_TEXTURE_MAX_ANISOTROPY_EXT
#define GL_TEXTURE_MAX_ANISOTROPY_EXT 0x84FE
#endif
#ifndef GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT
#define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT 0x84FF
#endif

// Utility function for loading a 2D texture from file (Keep this function here)
unsigned int loadTexture(char const * path) {
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    // std::cout << "Attempting to load texture: " << path << std::endl; // Debug output
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data) {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;
        else {
             std::cerr << "Texture loaded with unknown format (Components: " << nrComponents << ") Path: " << path << std::endl;
             stbi_image_free(data);
             glDeleteTextures(1, &textureID); // Clean up allocated texture ID
             return 0; // Return 0 on failure
        }

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        // Set texture wrapping and filtering options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // Enable Anisotropic Filtering
        if (glfwExtensionSupported("GL_EXT_texture_filter_anisotropic")) {
            float maxAnisotropy;
            glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropy);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAnisotropy);
        } else {
             // std::cout << "Warning: Anisotropic filtering not supported." << std::endl; // Less verbose
        }

        stbi_image_free(data);
        // std::cout << "Texture loaded successfully: " << path << std::endl; // Less verbose
    } else {
        std::cerr << "Texture failed to load at path: " << path << std::endl;
        glDeleteTextures(1, &textureID); // Clean up allocated texture ID
        return 0; // Return 0 on failure
    }

    return textureID;
}

// processInput (Keep the existing implementation)
void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    const float normalSpeed = 5.0f; // Slightly increased default speed
    const float sprintSpeed = 15.0f; // Increased sprint speed
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
        camera.MovementSpeed = sprintSpeed;
    else
        camera.MovementSpeed = normalSpeed;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        camera.Position.y += camera.MovementSpeed * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        camera.Position.y -= camera.MovementSpeed * deltaTime;

    // Fullscreen Toggle (Keep the existing implementation)
    if (glfwGetKey(window, GLFW_KEY_F11) == GLFW_PRESS && !f11_pressed) {
        fullscreen = !fullscreen;
        f11_pressed = true;
        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        if (fullscreen) {
            glfwGetWindowPos(window, &last_window_x, &last_window_y);
            glfwGetWindowSize(window, &last_window_width, &last_window_height);
            glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
        } else {
            glfwSetWindowMonitor(window, NULL, last_window_x, last_window_y, last_window_width, last_window_height, 0);
        }
        glfwSwapInterval(1); // Re-apply VSync
    }
    if (glfwGetKey(window, GLFW_KEY_F11) == GLFW_RELEASE) {
        f11_pressed = false;
    }
}

// framebuffer_size_callback (Keep the existing implementation)
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    if (height == 0) height = 1;
    glViewport(0, 0, width, height);
    SCR_WIDTH = width;
    SCR_HEIGHT = height;
    if (!fullscreen) {
        last_window_width = width;
        last_window_height = height;
    }
}

// mouse_callback (Keep the existing implementation)
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;
    camera.ProcessMouseMovement(xoffset, yoffset);
}

// scroll_callback (Keep the existing implementation)
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}
