
#include <CameraController.hpp>

#include <Keyboard.hpp>

#include <algorithm>
#include <iostream>
#include <cmath>

// Perform a linear interpolation
inline float lerp( float x0, float x1, float a )
{
    return x0 + a * ( x1 - x0 );
}

// Apply smoothing
inline void smooth( float& x0, float& x1, float deltaTime )
{
    float x;
    if ( std::fabsf( x0 ) < std::fabsf( x1 ) )  // Speeding up
    {
        x = ::lerp( x1, x0, std::powf( 0.6f, deltaTime * 60.0f ) );
    }
    else  // Slowing down
    {
        x = ::lerp( x1, x0, std::powf( 0.8f, deltaTime * 60.0f ) );
    }

    x0 = x;
    x1 = x;
}

CameraController::CameraController( Camera& camera, const glm::vec3& initialPosition, const glm::vec3& initialRotation )
: camera { camera }
, initialPosition { initialPosition }
, initialRotation { initialRotation }
{
    reset();
}

CameraController::~CameraController() = default;

void CameraController::update( float deltaTime )
{
    const GamePadState  gamePadState  = gamePad.getState( DeadZone::Circular );
    const MouseState    mouseState    = Mouse::getState();
    const KeyboardState keyboardState = Keyboard::getState();

    glm::vec3 translate { 0 };
    glm::vec3 rotate { 0 };

    if ( gamePadState.connected )
    {
        rotate.x -= gamePadState.thumbSticks.rightY * 15.0f * rotationSpeed * deltaTime;  // Pitch
        rotate.y -= gamePadState.thumbSticks.rightX * 15.0f * rotationSpeed * deltaTime;  // Yaw

        float y =
            ( gamePadState.buttons.rightShoulder ? 1.0f : 0.0f ) - ( gamePadState.buttons.leftShoulder ? 1.0f : 0.0f );

        translate +=
            glm::vec3 { gamePadState.thumbSticks.leftX, y, gamePadState.thumbSticks.leftY } * movementSpeed * deltaTime;
    }

    // Update mouse
    {
        float dx = static_cast<float>( previousMouseState.x - mouseState.x );
        float dy = static_cast<float>( previousMouseState.y - mouseState.y );

        if ( mouseState.leftButton )
        {
            rotate.x -= dy / 60.0f;// *rotationSpeed;
            rotate.y -= dx / 60.0f;  // *rotationSpeed;
        }
    }

    // Update keyboard
    {
        float x = static_cast<float>( keyboardState[SDL_SCANCODE_D] - keyboardState[SDL_SCANCODE_A] );
        float y = static_cast<float>( keyboardState[SDL_SCANCODE_E] - keyboardState[SDL_SCANCODE_Q] );
        float z = static_cast<float>( keyboardState[SDL_SCANCODE_S] - keyboardState[SDL_SCANCODE_W] );

        translate += glm::vec3 { x, y, z } * movementSpeed * deltaTime;
    }

    // Apply smoothing
    smooth( deltaPosition.x, translate.x, deltaTime );
    smooth( deltaPosition.y, translate.y, deltaTime );
    smooth( deltaPosition.z, translate.z, deltaTime );
    smooth( deltaRotation.x, rotate.x, deltaTime );
    smooth( deltaRotation.y, rotate.y, deltaTime );

    rotation += deltaRotation;

    // Clamp pitch
    rotation.x = std::clamp( rotation.x, -90.0f, 90.0f );

    // Convert to rotation quaternion.
    glm::quat q = glm::quat { glm::radians( rotation ) };
    // Apply translation in camera's local space.
    position += q * deltaPosition;

    camera.setRotation( glm::quat { glm::radians( rotation ) } );
    camera.setPosition( position );

    previousGamepadState = gamePadState;
    previousMouseState   = mouseState;
}

void CameraController::reset()
{
    position = initialPosition;
    rotation = initialRotation;

    deltaPosition = glm::vec3 { 0 };
    deltaRotation = glm::vec3 { 0 };

    // Build the rotation quaternion from eurler angles.
    glm::quat q = glm::quat { radians( glm::vec3 { rotation.x, rotation.y, rotation.z } ) };

    camera.setPosition( position );
    camera.setRotation( q );

    previousGamepadState = gamePad.getState();
    previousMouseState   = Mouse::getState();
}
