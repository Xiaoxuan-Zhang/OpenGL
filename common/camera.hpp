//
//  Header.hpp
//  playground
//
//  Created by Zooey Zhang on 7/25/18.
//

#ifndef Header_h
#define Header_h

#include <stdio.h>
#include <stdlib.h>

#include <glm/glm.hpp> //3D Math library
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

// Default camera values
const float YAW         = -90.0f;
const float PITCH       =  0.0f;
const float SPEED       =  2.5f;
const float SENSITIVITY =  0.1f;
const float ZOOM        =  45.0f;

class Camera {
public:
    // Camera Attributes
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;
    // Euler Angles
    float Yaw;
    float Pitch;
    // Camera options
    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;
    bool rotating;
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH, glm::vec3 front = glm::vec3(0.0f, 0.0f, -1.0f), float movementSpeed = SPEED, float mouseSensitivitiy = SENSITIVITY, float zoom = ZOOM) {
        Position = position;
        WorldUp = up;
        Yaw = yaw;
        Pitch = pitch;
        Front = front;
        MovementSpeed = movementSpeed;
        MouseSensitivity = mouseSensitivitiy;
        Zoom = zoom;
        rotating = false;
        updateCameraVectors();
    }
    
    // Constructor with scalar values
    Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch,
           glm::vec3 front = glm::vec3(0.0f, 0.0f, -1.0f), float movementSpeed = SPEED, float mouseSensitivitiy = SENSITIVITY, float zoom = ZOOM)
    {
        Position = glm::vec3(posX, posY, posZ);
        WorldUp = glm::vec3(upX, upY, upZ);
        Yaw = yaw;
        Pitch = pitch;
        Front = front;
        MovementSpeed = movementSpeed;
        MouseSensitivity = mouseSensitivitiy;
        Zoom = zoom;
        rotating = false;
        updateCameraVectors();
    }
    glm::mat4 GetViewMatrix() {
        if (rotating) {
            return glm::lookAt(Position, glm::vec3(0.0, 0.0, 0.0), Up);
        } else {
            return glm::lookAt(Position, Front + Position, Up);
        }
    }
    // Processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
    void ProcessKeyboard(Camera_Movement direction, float deltaTime)
    {
        rotating = false;
        float velocity = MovementSpeed * deltaTime;
        if (direction == FORWARD)
            Position += Front * velocity;
        if (direction == BACKWARD)
            Position -= Front * velocity;
        if (direction == LEFT)
            Position -= Right * velocity;
        if (direction == RIGHT)
            Position += Right * velocity;
    }
    void ProcessMouseMove(float xoffset, float yoffset, GLboolean constrainPitch = true) {
        rotating = false;
        xoffset *= MouseSensitivity;
        yoffset *= MouseSensitivity;
        
        Yaw += xoffset;
        Pitch += yoffset;
        if (constrainPitch) {
            if (Pitch > 89.0f){
                Pitch = 89.0f;
            }
            if (Pitch < -89.0f){
                Pitch = -89.0f;
            }
        }
        updateCameraVectors();
    }
    void ProcessMouseScroll(float yoffset) {
        rotating = false;
        if(Zoom >= 1.0f && Zoom <= 45.0f)
            Zoom -= yoffset;
        if(Zoom <= 1.0f)
            Zoom = 1.0f;
        if(Zoom >= 45.0f)
            Zoom = 45.0f;
    }
    void Rotation(float currentTime) {
        float Radius = 10.0f;
        Position.x = sin(0.1 * currentTime) * Radius;
        Position.z = cos(0.1 * currentTime) * Radius;
        rotating = true;
    }
private:
    // Calculates the front vector from the Camera's (updated) Euler Angles
    void updateCameraVectors()
    {
        // Calculate the new Front vector
        glm::vec3 front;
        front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        front.y = sin(glm::radians(Pitch));
        front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        Front = glm::normalize(front);
        // Also re-calculate the Right and Up vector
        Right = glm::normalize(glm::cross(Front, WorldUp));  // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
        Up    = glm::normalize(glm::cross(Right, Front));
    }
};

#endif /* Header_h */
