#include "GamePad.hpp"

#include <algorithm>
#include <cmath>

float ApplyLinearDeadZone( float value, float maxValue, float deadZoneSize = 0.0f ) noexcept
{
    if ( value < -deadZoneSize )
    {
        // Increase negative values to remove the dead-zone discontinuity.
        value += deadZoneSize;
    }
    else if ( value > deadZoneSize )
    {
        // Decrease positive values to remove the dead-zone discontinuity.
        value -= deadZoneSize;
    }
    else
    {
        // Values inside the dead-zone come out zero.
        return 0;
    }

    // Scale into 0-1 range.
    const float scaledValue = value / ( maxValue - deadZoneSize );
    return std::clamp( scaledValue, -1.0f, 1.f );
}

void ApplyStickDeadZone( float x, float y, DeadZone deadZoneMode, float maxValue, float deadZoneSize, float& resultX,
                         float& resultY ) noexcept
{
    switch ( deadZoneMode )
    {
    case DeadZone::IndependentAxis:
        resultX = ApplyLinearDeadZone( x, maxValue, deadZoneSize );
        resultY = ApplyLinearDeadZone( y, maxValue, deadZoneSize );
        break;
    case DeadZone::Circular:
    {
        const float dist   = std::sqrt( x * x + y * y );
        const float wanted = ApplyLinearDeadZone( dist, maxValue, deadZoneSize );

        const float scale = ( wanted > 0.f ) ? ( wanted / dist ) : 0.0f;

        resultX = std::clamp( x * scale, -1.0f, 1.f );
        resultY = std::clamp( y * scale, -1.0f, 1.0f );
    }
    break;
    case DeadZone::None:
        resultX = ApplyLinearDeadZone( x, maxValue, 0 );
        resultY = ApplyLinearDeadZone( y, maxValue, 0 );
        break;
    }
}

GamePad::GamePad( int player )
: playerId { player }
{
    pollGamePad();
}

GamePadState GamePad::getState( DeadZone deadZone )
{
    GamePadState state {};
    if ( !pollGamePad() )
    {
        state.connected = false;
        return state;
    }

    state.connected = true;

    state.buttons.a             = SDL_GameControllerGetButton( controller, SDL_CONTROLLER_BUTTON_A ) != 0;
    state.buttons.b             = SDL_GameControllerGetButton( controller, SDL_CONTROLLER_BUTTON_B ) != 0;
    state.buttons.x             = SDL_GameControllerGetButton( controller, SDL_CONTROLLER_BUTTON_X ) != 0;
    state.buttons.y             = SDL_GameControllerGetButton( controller, SDL_CONTROLLER_BUTTON_Y ) != 0;
    state.buttons.leftStick     = SDL_GameControllerGetButton( controller, SDL_CONTROLLER_BUTTON_LEFTSTICK ) != 0;
    state.buttons.rightStick    = SDL_GameControllerGetButton( controller, SDL_CONTROLLER_BUTTON_RIGHTSTICK ) != 0;
    state.buttons.leftShoulder  = SDL_GameControllerGetButton( controller, SDL_CONTROLLER_BUTTON_LEFTSHOULDER ) != 0;
    state.buttons.rightShoulder = SDL_GameControllerGetButton( controller, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER ) != 0;
    state.buttons.back          = SDL_GameControllerGetButton( controller, SDL_CONTROLLER_BUTTON_BACK ) != 0;
    state.buttons.start         = SDL_GameControllerGetButton( controller, SDL_CONTROLLER_BUTTON_START ) != 0;

    state.dPad.up    = SDL_GameControllerGetButton( controller, SDL_CONTROLLER_BUTTON_DPAD_UP ) != 0;
    state.dPad.down  = SDL_GameControllerGetButton( controller, SDL_CONTROLLER_BUTTON_DPAD_DOWN ) != 0;
    state.dPad.left  = SDL_GameControllerGetButton( controller, SDL_CONTROLLER_BUTTON_DPAD_LEFT ) != 0;
    state.dPad.right = SDL_GameControllerGetButton( controller, SDL_CONTROLLER_BUTTON_DPAD_RIGHT ) != 0;

    state.thumbSticks.leftX = SDL_GameControllerGetAxis( controller, SDL_CONTROLLER_AXIS_LEFTX );
    state.thumbSticks.leftY = SDL_GameControllerGetAxis( controller, SDL_CONTROLLER_AXIS_LEFTY );
    state.thumbSticks.rightX = SDL_GameControllerGetAxis( controller, SDL_CONTROLLER_AXIS_RIGHTX );
    state.thumbSticks.rightY = SDL_GameControllerGetAxis( controller, SDL_CONTROLLER_AXIS_RIGHTY );

    state.triggers.left = SDL_GameControllerGetAxis( controller, SDL_CONTROLLER_AXIS_TRIGGERLEFT );
    state.triggers.right = SDL_GameControllerGetAxis( controller, SDL_CONTROLLER_AXIS_TRIGGERRIGHT );

    float deadzoneSize = deadZone == DeadZone::None ? 0.0f : 30.0f; // TODO: Check if this is reasonable deadzone size for triggers.

    state.triggers.left = ApplyLinearDeadZone( state.triggers.left, SDL_JOYSTICK_AXIS_MAX, deadzoneSize );
    state.triggers.right = ApplyLinearDeadZone( state.triggers.right , SDL_JOYSTICK_AXIS_MAX, deadzoneSize );

    ApplyStickDeadZone( state.thumbSticks.leftX, state.thumbSticks.leftY, deadZone, SDL_JOYSTICK_AXIS_MAX, 7849.0f,
                        state.thumbSticks.leftX, state.thumbSticks.leftY );
    ApplyStickDeadZone( state.thumbSticks.rightX, state.thumbSticks.rightY, deadZone, SDL_JOYSTICK_AXIS_MAX, 8689.0f,
                        state.thumbSticks.rightX, state.thumbSticks.rightY );

    return state;
}

bool GamePad::pollGamePad()
{
    if ( controller )
    {
        // Controller is detached.
        if ( SDL_GameControllerGetAttached( controller ) == SDL_FALSE )
        {
            SDL_GameControllerClose( controller );
            controller = nullptr;
        }
    }
    else
    {
        if ( SDL_NumJoysticks() > playerId && SDL_IsGameController( playerId ) )
        {
            controller = SDL_GameControllerOpen( playerId );
        }
    }

    return controller != nullptr;
}
