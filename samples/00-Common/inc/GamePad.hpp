#pragma once

#include <SDL2/SDL_gamecontroller.h>

/// <summary>
/// Dead-zone mode.
/// </summary>
enum class DeadZone
{
    IndependentAxis = 0,
    Circular,
    None,
};

struct GamePadState
{
    struct Buttons
    {
        bool a;
        bool b;
        bool x;
        bool y;
        bool leftStick;
        bool rightStick;
        bool leftShoulder;
        bool rightShoulder;

        union
        {
            bool back;
            bool view;
        };

        union
        {
            bool start;
            bool menu;
        };
    } buttons;

    struct DPad
    {
        bool up;
        bool down;
        bool right;
        bool left;
    } dPad;

    struct ThumbSticks
    {
        float leftX;
        float leftY;
        float rightX;
        float rightY;
    } thumbSticks;

    struct Triggers
    {
        float left;
        float right;
    } triggers;

    bool connected;
};

class GamePad
{
public:
    GamePad( int player );

    GamePadState getState( DeadZone deadZone = DeadZone::IndependentAxis );

    bool setVibration( float leftMotor, float rightMotor, float leftTrigger = 0.0f,
                              float rightTrigger = 0.0f );

private:
    // Poll the gamepad if it is attached or detached.
    bool pollGamePad();

    int playerId = 0;
    SDL_GameController* controller = nullptr;
};