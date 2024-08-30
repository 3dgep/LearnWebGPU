#include <Keyboard.hpp>

namespace Keyboard
{

KeyboardState getState()
{
    KeyboardState state {};

    int          numKeys;
    const Uint8* keys = SDL_GetKeyboardState( &numKeys );

    memcpy_s( state.keys.data(), sizeof( Uint8 ) * state.keys.size(), keys, sizeof( Uint8 ) * numKeys );

    return state;
}

}  // namespace Keyboard

bool KeyboardState::isKeyDown( SDL_Scancode key ) const noexcept
{
    return keys[key] == 1;
}

bool KeyboardState::isKeyUp( SDL_Scancode key ) const noexcept
{
    return keys[key] == 0;
}
