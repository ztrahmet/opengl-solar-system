#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

// Function prototype for the framebuffer size callback
void framebuffer_size_callback(GLFWwindow* window, int width, int height);

// Settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

int main() {
    // 1. Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // 2. Create a Window
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Solar System", NULL, NULL);
    if (window == NULL) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // 3. Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // 4. Set the viewport
    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

    // 5. The Render Loop
    while (!glfwWindowShouldClose(window)) {
        // --- Input (we'll add this later) ---
        // processInput(window);

        // --- Rendering ---
        // Set the clear color (a dark gray)
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        // Clear the color buffer
        glClear(GL_COLOR_BUFFER_BIT);

        // --- Check and call events and swap buffers ---
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // 6. Terminate and cleanup
    glfwTerminate();
    return 0;
}

// GLFW: whenever the window size changed, this callback function executes
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    // Make sure the viewport matches the new window dimensions
    glViewport(0, 0, width, height);
}
