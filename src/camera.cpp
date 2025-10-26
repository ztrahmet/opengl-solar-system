/**
 * @file camera.cpp
 * @brief Implements the Camera class methods.
 */

#include "camera.h"
#include <algorithm> // For std::clamp

/**
 * @brief Constructor using vectors. Initializes camera attributes and calculates initial direction vectors.
 */
Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch)
    : Front(glm::vec3(0.0f, 0.0f, -1.0f)), // Initial front vector
      MovementSpeed(SPEED),                // Default speed
      MouseSensitivity(SENSITIVITY),       // Default sensitivity
      Zoom(ZOOM)                           // Default FOV
{
    Position = position;
    WorldUp = up;
    Yaw = yaw;
    Pitch = pitch;
    updateCameraVectors(); // Calculate initial Front, Right, Up vectors from Yaw/Pitch
}

/**
 * @brief Constructor using scalar values. Initializes camera attributes and calculates initial direction vectors.
 */
Camera::Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch)
    : Front(glm::vec3(0.0f, 0.0f, -1.0f)),
      MovementSpeed(SPEED),
      MouseSensitivity(SENSITIVITY),
      Zoom(ZOOM)
{
    Position = glm::vec3(posX, posY, posZ);
    WorldUp = glm::vec3(upX, upY, upZ);
    Yaw = yaw;
    Pitch = pitch;
    updateCameraVectors();
}

/**
 * @brief Calculates and returns the view matrix using glm::lookAt.
 * This matrix transforms world coordinates to view (camera) coordinates.
 */
glm::mat4 Camera::GetViewMatrix()
{
    // glm::lookAt requires: camera position, target position, world up vector
    return glm::lookAt(Position, Position + Front, Up);
}

/**
 * @brief Updates camera position based on keyboard input (WASD).
 */
void Camera::ProcessKeyboard(Camera_Movement direction, float deltaTime)
{
    float velocity = MovementSpeed * deltaTime; // Movement distance for this frame
    if (direction == FORWARD)
        Position += Front * velocity;
    if (direction == BACKWARD)
        Position -= Front * velocity;
    if (direction == LEFT)
        Position -= Right * velocity; // Use the calculated Right vector
    if (direction == RIGHT)
        Position += Right * velocity;
    // Note: UP/DOWN movement is handled directly in main.cpp using absolute Y axis
}

/**
 * @brief Updates camera orientation (Yaw and Pitch) based on mouse movement offsets.
 */
void Camera::ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch)
{
    // Scale offsets by sensitivity
    xoffset *= MouseSensitivity;
    yoffset *= MouseSensitivity;

    // Add offsets to angles
    Yaw += xoffset;
    Pitch += yoffset;

    // Constrain pitch to avoid flipping upside down
    if (constrainPitch)
    {
        Pitch = std::clamp(Pitch, -89.0f, 89.0f);
    }

    // Update Front, Right, and Up vectors using the modified Euler angles
    updateCameraVectors();
}

/**
 * @brief Updates the camera's Zoom level (FOV) based on mouse scroll wheel input.
 */
void Camera::ProcessMouseScroll(float yoffset)
{
    Zoom -= yoffset; // Adjust FOV
    // Clamp FOV to reasonable limits
    Zoom = std::clamp(Zoom, 1.0f, 45.0f);
}

/**
 * @brief Recalculates the Front, Right, and Up direction vectors from the camera's updated Euler Angles (Yaw and Pitch).
 */
void Camera::updateCameraVectors()
{
    // Calculate the new Front vector using trigonometry
    glm::vec3 front;
    front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    front.y = sin(glm::radians(Pitch));
    front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    Front = glm::normalize(front);

    // Also re-calculate the Right and Up vectors using cross products
    // Normalize vectors to prevent slowing down movement when looking up/down
    Right = glm::normalize(glm::cross(Front, WorldUp));
    Up = glm::normalize(glm::cross(Right, Front));
}
