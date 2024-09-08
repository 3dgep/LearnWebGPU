#pragma once

#include "Camera.hpp"
#include "GamePad.hpp"
#include "Mouse.hpp"

#include <glm/vec3.hpp>

class CameraController
{
public:
    CameraController( Camera& camera, const glm::vec3& position, const glm::vec3& rotation );
    ~CameraController();

    CameraController( const CameraController& )                = delete;
    CameraController( CameraController&& ) noexcept            = delete;
    CameraController& operator=( const CameraController& )     = delete;
    CameraController& operator=( CameraController&& ) noexcept = delete;

    void update( float deltaTime );

    void reset();

private:
    Camera&      camera;
    GamePad      gamePad { 0 };
    GamePadState previousGamepadState {};
    // Used for calculating deltas.
    MouseState previousMouseState {};

    glm::vec3 initialPosition;
    glm::vec3 initialRotation;

    glm::vec3 position;
    glm::vec3 rotation;

    glm::vec3 deltaPosition{0};
    glm::vec3 deltaRotation{0};

    // Tweakables
    static constexpr inline float rotationSpeed = 5.0f;   // Degrees per second.
    static constexpr inline float movementSpeed = 20.0f;  // Units per second.
};