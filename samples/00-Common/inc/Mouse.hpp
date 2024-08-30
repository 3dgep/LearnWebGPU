#pragma once

struct MouseState
{
    bool leftButton;
    bool middleButton;
    bool rightButton;
    bool x1Button;
    bool x2Button;

    // Mouse position relative to the focused window.
    int x, y;
};

namespace Mouse
{

// Get the current state of the mouse.
MouseState getState();

}  // namespace Mouse
