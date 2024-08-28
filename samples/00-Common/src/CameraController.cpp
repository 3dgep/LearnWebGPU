#include <CameraController.hpp>

#include <algorithm>
#include <iostream>

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
        x = lerp( x1, x0, std::powf( 0.6f, deltaTime * 60.0f ) );
    }
    else  // Slowing down
    {
        x = lerp( x1, x0, std::powf( 0.8f, deltaTime * 60.0f ) );
    }

    x0 = x;
    x1 = x;
}

CameraController::CameraController( Camera& camera, const glm::vec3& initialPosition, const glm::vec3& initialRotation )
: camera { camera }
, initialPosition{initialPosition}
, initialRotation{initialRotation}
{
    reset();
}

CameraController::~CameraController() = default;

void CameraController::update( float deltaTime )
{
    GamePadState state = gamePad.getState( DeadZone::Circular );

    if ( state.connected )
    {
        rotation.x -= state.thumbSticks.rightY; // Pitch
        rotation.y -= state.thumbSticks.rightX; // Yaw

        float y = (state.buttons.rightShoulder ? 1.0f : 0.0f) - (state.buttons.leftShoulder ? 1.0f : 0.0f);

        glm::vec3 t { state.thumbSticks.leftX, y, state.thumbSticks.leftY };

        // Convert to rotation quaternion.
        glm::quat q = glm::quat { glm::radians( rotation ) };

        position += q * t;

        camera.setRotation( glm::quat { glm::radians( rotation ) } );
        camera.setPosition( position );
    }

    previousGamepadState = state;
}

void CameraController::reset()
{
    position = initialPosition;
    rotation = initialRotation;

    // Build the rotation quaternion from eurler angles.
    glm::quat q = glm::quat { radians( glm::vec3 { rotation.x, rotation.y, rotation.z } ) };

    camera.setPosition( position );
    camera.setRotation( q );
}
