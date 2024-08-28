#pragma once

#include "Camera.hpp"
#include "GamePad.hpp"
#include "Timer.hpp"

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

    glm::vec3    initialPosition;
    glm::vec3    initialRotation;

    glm::vec3 position;
    glm::vec3 rotation;
};