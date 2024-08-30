#pragma once

#include <SDL2/SDL_keyboard.h>

#include <array>

struct KeyboardState
{
    // Is the key on the keyboard currently being held down?
    bool isKeyDown( SDL_Scancode key ) const noexcept;

    // Is the key on the keyboard currently not being held down?
    bool isKeyUp( SDL_Scancode key ) const noexcept;

    bool operator[]( SDL_Scancode key ) const noexcept
    {
        return isKeyDown( key );
    }

    std::array<Uint8, SDL_NUM_SCANCODES> keys;
};

namespace Keyboard
{
KeyboardState getState();
}