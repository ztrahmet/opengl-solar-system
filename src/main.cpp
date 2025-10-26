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

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <thread>
#include <chrono>
#include <optional>
#include <map>
#include <algorithm>

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
unsigned int loadTexture(const char *path);
unsigned int loadCubemap(std::vector<std::string> faces);
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);

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
float simulationSpeed = 1.0f;
CelestialBody *cameraLockedTo = nullptr;
std::string lockedBodyName = "None";
std::map<std::string, CelestialBody *> bodyMap;
float lockedCameraDistance = 10.0f;
float lockedCameraOrbitYaw = -90.0f;
float lockedCameraOrbitPitch = 0.0f;
float accumulatedSimTime = 0.0f; // Track simulation time incrementally

float skyboxVertices[] = {
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

int main()
{
    config = loadConfig("config.ini");
    SCR_WIDTH = config.width;
    SCR_HEIGHT = config.height;
    fullscreen = config.startFullscreen;
    last_window_width = config.width;
    last_window_height = config.height;
    lastX = SCR_WIDTH / 2.0f;
    lastY = SCR_HEIGHT / 2.0f;

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    const char *glsl_version = "#version 330 core";
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

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
        std::cerr << "Failed GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwGetWindowPos(window, &last_window_x, &last_window_y);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed GLAD" << std::endl;
        return -1;
    }

    glfwSwapInterval(1);
    glEnable(GL_DEPTH_TEST);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.MouseDrawCursor = false;
    io.ConfigFlags |= ImGuiConfigFlags_NoMouse;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, false);
    ImGui_ImplOpenGL3_Init(glsl_version);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback);

    Scenario currentScenario = loadScenario_SolarSystemBasic();
    for (auto &body : currentScenario.bodies)
    {
        bodyMap[body.name] = &body;
    }

    camera.Position = currentScenario.initialCameraPos;
    camera.updateCameraVectors();

    Shader lightingShader("shaders/lighting.vert", "shaders/lighting.frag");
    Shader emissiveShader("shaders/emissive.vert", "shaders/emissive.frag");
    Shader skyboxShader("shaders/skybox.vert", "shaders/skybox.frag");

    stbi_set_flip_vertically_on_load(true);
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

    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glBindVertexArray(0);

    std::vector<std::string> faces = {
        "textures/skybox/right.jpg", "textures/skybox/left.jpg",
        "textures/skybox/top.jpg", "textures/skybox/bottom.jpg",
        "textures/skybox/front.jpg", "textures/skybox/back.jpg"};
    stbi_set_flip_vertically_on_load(false);
    unsigned int cubemapTexture = loadCubemap(faces);
    stbi_set_flip_vertically_on_load(true);

    lightingShader.use();
    lightingShader.setInt("ourTexture", 0);
    emissiveShader.use();
    emissiveShader.setInt("ourTexture", 0);
    skyboxShader.use();
    skyboxShader.setInt("skybox", 0);

    lastTimeForFPS = glfwGetTime();
    lastFrame = (float)lastTimeForFPS;
    glm::vec3 lightPos = currentScenario.lightPos;
    glm::vec3 lightColor = currentScenario.lightColor;

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    while (!glfwWindowShouldClose(window))
    {
        double currentFrameTime = glfwGetTime();
        deltaTime = (float)currentFrameTime - lastFrame;
        lastFrame = (float)currentFrameTime;
        // Calculate simulation time delta and accumulate
        float simDeltaTime = deltaTime * simulationSpeed;
        accumulatedSimTime += simDeltaTime;

        nbFrames++;
        if (currentFrameTime - lastTimeForFPS >= 1.0)
        {
            std::string title = "FPS:" + std::to_string(nbFrames);
            glfwSetWindowTitle(window, title.c_str());
            nbFrames = 0;
            lastTimeForFPS = currentFrameTime;
        }

        glfwPollEvents();
        processInput(window);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        glClearColor(0.01f, 0.01f, 0.01f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::vec3 currentCameraTargetPos = glm::vec3(0.0f);
        glm::mat4 view;
        if (cameraLockedTo)
        {
            currentCameraTargetPos = glm::vec3(cameraLockedTo->currentModelMatrix[3]);
            lockedCameraDistance = std::clamp(lockedCameraDistance, cameraLockedTo->radius * 1.5f, 50.0f * cameraLockedTo->radius);
            float camX = currentCameraTargetPos.x + lockedCameraDistance * cos(glm::radians(lockedCameraOrbitPitch)) * cos(glm::radians(lockedCameraOrbitYaw));
            float camY = currentCameraTargetPos.y + lockedCameraDistance * sin(glm::radians(lockedCameraOrbitPitch));
            float camZ = currentCameraTargetPos.z + lockedCameraDistance * cos(glm::radians(lockedCameraOrbitPitch)) * sin(glm::radians(lockedCameraOrbitYaw));
            camera.Position = glm::vec3(camX, camY, camZ);
            view = glm::lookAt(camera.Position, currentCameraTargetPos, camera.WorldUp);
            camera.Front = glm::normalize(currentCameraTargetPos - camera.Position);
            camera.Right = glm::normalize(glm::cross(camera.Front, camera.WorldUp));
            camera.Up = glm::normalize(glm::cross(camera.Right, camera.Front));
            camera.Yaw = glm::degrees(atan2(camera.Front.z, camera.Front.x));
            camera.Pitch = glm::degrees(asin(camera.Front.y));
        }
        else
        {
            view = camera.GetViewMatrix();
        }

        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);

        // Use accumulatedSimTime for animations
        for (auto &body : currentScenario.bodies)
        {
            glm::mat4 model = glm::mat4(1.0f);
            glm::mat4 orbitTranslation = glm::mat4(1.0f);
            glm::mat4 rotation = glm::mat4(1.0f);
            if (body.orbitRadius > 0.0f)
            {
                float orbitAngle = accumulatedSimTime * body.orbitSpeed;
                orbitTranslation = glm::translate(glm::mat4(1.0f), glm::vec3(cos(orbitAngle) * body.orbitRadius, 0.0f, sin(orbitAngle) * body.orbitRadius));
            }
            rotation = glm::rotate(glm::mat4(1.0f), accumulatedSimTime * body.rotationSpeed, glm::normalize(body.rotationAxis));
            glm::mat4 parentModelMatrix = glm::mat4(1.0f);
            if (body.parentName)
            {
                auto it = bodyMap.find(*body.parentName);
                if (it != bodyMap.end())
                {
                    parentModelMatrix = it->second->currentModelMatrix;
                }
            }
            model = parentModelMatrix * orbitTranslation * rotation;
            model = glm::scale(model, glm::vec3(body.radius));
            body.currentModelMatrix = model;

            Shader &currentShader = body.isEmissive ? emissiveShader : lightingShader;
            currentShader.use();
            currentShader.setMat4("projection", projection);
            currentShader.setMat4("view", view);
            currentShader.setMat4("model", model);
            if (!body.isEmissive)
            {
                lightingShader.setVec3("lightPos", lightPos);
                lightingShader.setVec3("viewPos", camera.Position);
                lightingShader.setVec3("lightColor", lightColor);
                glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(model)));
                lightingShader.setMat3("normalMatrix", normalMatrix);
            }
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, body.textureID);
            if (body.mesh)
            {
                body.mesh->draw();
            }
        }

        glDepthFunc(GL_LEQUAL);
        skyboxShader.use();
        glm::mat4 skyboxView = glm::mat4(glm::mat3(view));
        skyboxShader.setMat4("view", skyboxView);
        skyboxShader.setMat4("projection", projection);
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS);

        ImGui::Begin("Controls");
        ImGui::Text("Sim Speed: %.1fx (1-5)", simulationSpeed);
        ImGui::Text("Cam Lock: %s (E,M,N)", lockedBodyName.c_str());
        ImGui::Separator();
        ImGui::Text("WASD: Move | Spc/Shft: Up/Dn | Ctrl: Sprint");
        ImGui::Text("Mouse: Look/Orbit | Scroll: Zoom");
        ImGui::Text("F11: Fullscr | Esc: Exit");
        ImGui::Separator();
        ImGui::Text("%.3f ms/fr (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glDeleteVertexArrays(1, &skyboxVAO);
    glDeleteBuffers(1, &skyboxVBO);
    glDeleteTextures(1, &cubemapTexture);
    for (auto &body : currentScenario.bodies)
    {
        if (body.textureID != 0)
            glDeleteTextures(1, &body.textureID);
    }
    glfwTerminate();
    return 0;
}

#ifndef GL_TEXTURE_MAX_ANISOTROPY_EXT
#define GL_TEXTURE_MAX_ANISOTROPY_EXT 0x84FE
#endif
#ifndef GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT
#define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT 0x84FF
#endif

unsigned int loadTexture(char const *path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format = GL_RGB;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;
        else
        {
            std::cerr << "Tex fmt err:" << path << std::endl;
            stbi_image_free(data);
            glDeleteTextures(1, &textureID);
            return 0;
        }

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        if (glfwExtensionSupported("GL_EXT_texture_filter_anisotropic"))
        {
            float maxAniso;
            glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAniso);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAniso);
        }
        stbi_image_free(data);
    }
    else
    {
        std::cerr << "Tex load fail:" << path << std::endl;
        glDeleteTextures(1, &textureID);
        return 0;
    }
    return textureID;
}

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
                std::cerr << "Cubemap unsupported channels: " << faces[i] << " (" << nrChannels << ")" << std::endl;
                stbi_image_free(data);
                glDeleteTextures(1, &textureID);
                return 0;
            }
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            std::cerr << "Cubemap fail: " << faces[i] << std::endl;
            stbi_image_free(data);
            glDeleteTextures(1, &textureID);
            return 0;
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    return textureID;
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS)
    {
        if (key >= GLFW_KEY_1 && key <= GLFW_KEY_5)
        {
            float speeds[] = {0.0f, 0.5f, 1.0f, 2.0f, 5.0f};
            simulationSpeed = speeds[key - GLFW_KEY_1];
        }
        else if (key == GLFW_KEY_E)
        {
            auto it = bodyMap.find("Earth");
            if (it != bodyMap.end())
            {
                cameraLockedTo = it->second;
                lockedBodyName = "Earth";
                lockedCameraDistance = cameraLockedTo->radius * 5.0f;
                camera.Zoom = ZOOM; // Reset zoom on lock
                glm::vec3 direction = glm::normalize(camera.Position - glm::vec3(cameraLockedTo->currentModelMatrix[3]));
                lockedCameraOrbitYaw = glm::degrees(atan2(direction.z, direction.x));
                lockedCameraOrbitPitch = glm::degrees(asin(direction.y));
            }
        }
        else if (key == GLFW_KEY_M)
        {
            auto it = bodyMap.find("Mars");
            if (it != bodyMap.end())
            {
                cameraLockedTo = it->second;
                lockedBodyName = "Mars";
                lockedCameraDistance = cameraLockedTo->radius * 5.0f;
                camera.Zoom = ZOOM; // Reset zoom on lock
                glm::vec3 direction = glm::normalize(camera.Position - glm::vec3(cameraLockedTo->currentModelMatrix[3]));
                lockedCameraOrbitYaw = glm::degrees(atan2(direction.z, direction.x));
                lockedCameraOrbitPitch = glm::degrees(asin(direction.y));
            }
        }
        else if (key == GLFW_KEY_N)
        {
            cameraLockedTo = nullptr;
            lockedBodyName = "None";
            camera.updateCameraVectors();
        }
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
        else if (key == GLFW_KEY_ESCAPE)
        {
            glfwSetWindowShouldClose(window, true);
        }
    }
    else if (action == GLFW_RELEASE)
    {
        if (key == GLFW_KEY_F11)
        {
            f11_pressed = false;
        }
    }
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
    if (cameraLockedTo)
    {
        float zoomSensitivity = 0.5f;
        lockedCameraDistance -= static_cast<float>(yoffset) * zoomSensitivity * (lockedCameraDistance * 0.1f);
        lockedCameraDistance = std::clamp(lockedCameraDistance, cameraLockedTo->radius * 1.5f, 50.0f * cameraLockedTo->radius);
    }
    else
    {
        camera.ProcessMouseScroll(static_cast<float>(yoffset));
    }
}

void mouse_callback(GLFWwindow *window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    float sensitivityMultiplier = camera.Zoom / 45.0f;
    sensitivityMultiplier = std::clamp(sensitivityMultiplier, 0.1f, 1.0f);

    xoffset *= sensitivityMultiplier;
    yoffset *= sensitivityMultiplier;

    if (cameraLockedTo)
    {
        float sensitivity = 0.1f;
        lockedCameraOrbitYaw += xoffset * sensitivity;
        lockedCameraOrbitPitch -= yoffset * sensitivity;
        lockedCameraOrbitPitch = std::clamp(lockedCameraOrbitPitch, -89.0f, 89.0f);
    }
    else
    {
        camera.ProcessMouseMovement(xoffset, yoffset);
    }
}

void processInput(GLFWwindow *window)
{
    if (!cameraLockedTo)
    {
        const float baseNormalSpeed = 5.0f;
        const float baseSprintSpeed = 15.0f;
        float speedMultiplier = camera.Zoom / 45.0f;
        speedMultiplier = std::max(speedMultiplier, 0.1f);
        float currentNormalSpeed = baseNormalSpeed * speedMultiplier;
        float currentSprintSpeed = baseSprintSpeed * speedMultiplier;
        camera.MovementSpeed = (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) ? currentSprintSpeed : currentNormalSpeed;
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
    }
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    if (height == 0)
        height = 1;
    glViewport(0, 0, width, height);
    SCR_WIDTH = width;
    SCR_HEIGHT = height;
    if (!fullscreen)
    {
        last_window_width = width;
        last_window_height = height;
    }
}
