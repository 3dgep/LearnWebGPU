#include <Mouse.hpp>

#include <SDL2/SDL_events.h>
#include <SDL2/SDL_mouse.h>

namespace Mouse
{

MouseState getState()
{
    MouseState state {};

    const Uint32 buttons = SDL_GetMouseState( &state.x, &state.y );

    state.leftButton = ( buttons & SDL_BUTTON_LMASK ) != 0;
    state.middleButton = ( buttons & SDL_BUTTON_MMASK ) != 0;
    state.rightButton = ( buttons & SDL_BUTTON_RMASK ) != 0;
    state.x1Button = ( buttons & SDL_BUTTON_X1MASK ) != 0;
    state.x2Button = ( buttons & SDL_BUTTON_X2MASK ) != 0;

    return state;
}
}  // namespace Mouse
