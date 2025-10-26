/**
 * @file camera.h
 * @brief Defines the Camera class for managing view transformations and user input.
 */

#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>                  // OpenGL types
#include <glm/glm.hpp>                  // Vector/matrix math
#include <glm/gtc/matrix_transform.hpp> // lookAt function

#include <vector>

// Defines several possible options for camera movement.
// Used as abstraction to stay away from window-system specific input methods
enum Camera_Movement
{
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
    // UP and DOWN are handled directly in main.cpp for absolute Y movement
};

// Default camera values used if not specified in constructor
const float YAW = -90.0f;       // Initial horizontal angle (looking down negative Z)
const float PITCH = 0.0f;       // Initial vertical angle
const float SPEED = 5.0f;       // Default movement speed (adjusted by zoom/sprint in main.cpp)
const float SENSITIVITY = 0.1f; // Mouse look sensitivity (adjusted by zoom in main.cpp)
const float ZOOM = 45.0f;       // Default Field of View (FOV) in degrees

/**
 * @class Camera
 * @brief Handles camera position, orientation, projection, and input processing.
 *
 * An abstract camera class that processes input (keyboard, mouse movement, scroll)
 * and calculates the corresponding Euler Angles, direction vectors (Front, Up, Right),
 * and the View Matrix for use in OpenGL rendering. Also manages Field of View (Zoom).
 */
class Camera
{
public:
    // --- Camera Attributes ---
    glm::vec3 Position; // Camera's world space position
    glm::vec3 Front;    // Direction camera is facing (normalized)
    glm::vec3 Up;       // Camera's local up direction (normalized)
    glm::vec3 Right;    // Camera's local right direction (normalized)
    glm::vec3 WorldUp;  // Global up direction (usually 0,1,0)

    // --- Euler Angles ---
    float Yaw;   // Horizontal rotation angle
    float Pitch; // Vertical rotation angle

    // --- Camera Options ---
    float MovementSpeed;    // Current movement speed
    float MouseSensitivity; // Current mouse look sensitivity
    float Zoom;             // Current Field of View (FOV) in degrees

    // --- Constructors ---

    /**
     * @brief Constructor using vectors.
     * @param position Initial camera position. Defaults to (0,0,0).
     * @param up Initial world up direction. Defaults to (0,1,0).
     * @param yaw Initial yaw angle. Defaults to YAW.
     * @param pitch Initial pitch angle. Defaults to PITCH.
     */
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH);

    /**
     * @brief Constructor using scalar values.
     */
    Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch);

    // --- Core Functions ---

    /**
     * @brief Calculates and returns the view matrix using the current camera position and orientation.
     * Uses Euler Angles and the LookAt Matrix.
     * @return The 4x4 view matrix.
     */
    glm::mat4 GetViewMatrix();

    // --- Input Processing ---

    /**
     * @brief Processes keyboard input for FORWARD, BACKWARD, LEFT, RIGHT movement.
     * Updates the camera's Position based on direction and deltaTime.
     * @param direction The movement direction enum (FORWARD, BACKWARD, LEFT, RIGHT).
     * @param deltaTime Time elapsed since the last frame.
     */
    void ProcessKeyboard(Camera_Movement direction, float deltaTime);

    /**
     * @brief Processes mouse movement input to update camera orientation (Yaw and Pitch).
     * @param xoffset Change in horizontal mouse position since last frame.
     * @param yoffset Change in vertical mouse position since last frame.
     * @param constrainPitch If true, limits the pitch angle to prevent looking straight up/down. Defaults to true.
     */
    void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true);

    /**
     * @brief Processes mouse scroll wheel input to adjust the Zoom level (FOV).
     * @param yoffset Change in vertical scroll position.
     */
    void ProcessMouseScroll(float yoffset);

    /**
     * @brief Recalculates the Front, Right, and Up vectors based on the current Yaw and Pitch angles.
     * Needs to be public for camera locking logic in main.cpp to manually update vectors.
     */
    void updateCameraVectors();

    // private: // Moved updateCameraVectors to public
    //  Originally private, moved to public for external updates during camera lock
    //  void updateCameraVectors();
};
#endif
