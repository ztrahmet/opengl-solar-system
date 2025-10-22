#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// STB_IMAGE_IMPLEMENTATION is no longer defined here
#include "stb_image.h" 

#include "shader.h"
#include "camera.h"
#include "planet.h"
#include "config.h"

#include <iostream>
#include <string>
#include <thread>   // For std::this_thread
#include <chrono>   // For std::chrono

// Function prototypes
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
unsigned int loadTexture(const char *path);

// --- Settings ---
// These are now loaded from config.ini
Config config;
unsigned int SCR_WIDTH;
unsigned int SCR_HEIGHT;

// Camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX; // Will be set after window creation
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
    
    // Set initial mouse position
    lastX = SCR_WIDTH / 2.0f;
    lastY = SCR_HEIGHT / 2.0f;

    // 2. GLFW: initialize and configure
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // 3. GLFW window creation
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);

    GLFWwindow* window;
    if (fullscreen) {
        // Start in fullscreen
        window = glfwCreateWindow(mode->width, mode->height, "Solar System", monitor, NULL);
        SCR_WIDTH = mode->width;
        SCR_HEIGHT = mode->height;
    } else {
        // Start in windowed mode
        window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Solar System", NULL, NULL);
    }
    
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // Store initial window state
    glfwGetWindowPos(window, &last_window_x, &last_window_y);

    // Tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // 4. GLAD: load all OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // 5. Enable VSync
    glfwSwapInterval(1); 

    // 6. Configure global opengl state
    glEnable(GL_DEPTH_TEST);

    // 7. Build and compile our shader program
    Shader ourShader("shaders/basic.vert", "shaders/basic.frag");

    // 8. Create our planet
    Planet earth(1.0f, 64, 64);

    // 9. Load the texture
    stbi_set_flip_vertically_on_load(true); 
    unsigned int earthTexture = loadTexture("textures/earth.jpg");
    
    ourShader.use();
    ourShader.setInt("ourTexture", 0);
    
    lastTimeForFPS = glfwGetTime();
    lastFrame = (float)lastTimeForFPS;

    // 10. Render loop
    while (!glfwWindowShouldClose(window)) {
        double frameStartTime = glfwGetTime();

        // Per-frame time logic
        deltaTime = (float)frameStartTime - lastFrame;
        lastFrame = (float)frameStartTime;

        // --- FPS Counter Logic ---
        nbFrames++;
        if (frameStartTime - lastTimeForFPS >= 1.0) {
            std::string title = "Solar System - FPS: " + std::to_string(nbFrames);
            glfwSetWindowTitle(window, title.c_str());
            nbFrames = 0;
            lastTimeForFPS = frameStartTime;
        }
        
        // Input
        processInput(window);

        // Render
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 

        ourShader.use();

        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        ourShader.setMat4("projection", projection);

        glm::mat4 view = camera.GetViewMatrix();
        ourShader.setMat4("view", view);
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, earthTexture);

        glm::mat4 model = glm::mat4(1.0f); 
        ourShader.setMat4("model", model);
        
        earth.draw();
        
        // GLFW: swap buffers and poll IO events
        glfwSwapBuffers(window);
        glfwPollEvents();

        // --- FPS Capping Logic Removed ---
    }
    
    glfwTerminate();
    return 0;
}

// --- Function Implementations ---

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
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;
        else {
            std::cerr << "Unknown texture format with " << nrComponents << " components" << std::endl;
            stbi_image_free(data);
            return 0;
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
            std::cout << "Anisotropic filtering not supported" << std::endl;
        }

        stbi_image_free(data);
    } else {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // --- Camera Movement Speed ---
    const float normalSpeed = 2.5f;
    const float sprintSpeed = 7.5f;
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
        camera.MovementSpeed = sprintSpeed;
    else
        camera.MovementSpeed = normalSpeed;

    // --- Camera WASD Movement ---
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);

    // --- Camera Absolute Vertical Movement ---
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        camera.Position.y += camera.MovementSpeed * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        camera.Position.y -= camera.MovementSpeed * deltaTime;

    // --- Fullscreen Toggle ---
    if (glfwGetKey(window, GLFW_KEY_F11) == GLFW_PRESS && !f11_pressed) {
        fullscreen = !fullscreen;
        f11_pressed = true; // Mark as pressed

        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        
        if (fullscreen) {
            // Store windowed size/pos
            glfwGetWindowPos(window, &last_window_x, &last_window_y);
            glfwGetWindowSize(window, &last_window_width, &last_window_height);
            // Switch to fullscreen
            glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
        } else {
            // Switch to windowed
            glfwSetWindowMonitor(window, NULL, last_window_x, last_window_y, last_window_width, last_window_height, 0);
        }

        // --- BUG FIX: Re-apply VSync setting ---
        // Calling glfwSetWindowMonitor resets the swap interval
        glfwSwapInterval(1); // Always Enable VSync
        // --- End of Bug Fix ---
    }
    // Reset pressed flag when key is released
    if (glfwGetKey(window, GLFW_KEY_F11) == GLFW_RELEASE) {
        f11_pressed = false;
    }
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    SCR_WIDTH = width;
    SCR_HEIGHT = height;

    // Update last windowed size if not in fullscreen
    if (!fullscreen) {
        last_window_width = width;
        last_window_height = height;
    }
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    float xpos = static_cast<float>(xposIn); // <-- Corrected typo here
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

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}
