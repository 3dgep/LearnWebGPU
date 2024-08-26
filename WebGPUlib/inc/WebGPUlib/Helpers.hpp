#pragma once

namespace WebGPUlib
{
/***************************************************************************
 * These functions were taken from the MiniEngine.
 * Source code available here:
 * https://github.com/Microsoft/DirectX-Graphics-Samples/blob/master/MiniEngine/Core/Math/Common.h
 * Retrieved: January 13, 2016
 **************************************************************************/
template<typename T>
constexpr T AlignUpWithMask( T value, size_t mask )
{
    return (T)( ( static_cast<size_t>(value) + mask ) & ~mask );
}

template<typename T>
constexpr T AlignDownWithMask( T value, size_t mask )
{
    return (T)( static_cast<size_t>(value) & ~mask );
}

template<typename T>
constexpr T AlignUp( T value, size_t alignment )
{
    return AlignUpWithMask( value, alignment - 1 );
}

template<typename T>
constexpr T AlignDown( T value, size_t alignment )
{
    return AlignDownWithMask( value, alignment - 1 );
}
template<typename T>
constexpr bool IsAligned( T value, size_t alignment )
{
    return 0 == ( static_cast<size_t>(value) & ( alignment - 1 ) );
}

template<typename T>
constexpr T DivideByMultiple( T value, size_t alignment )
{
    return (T)( ( value + alignment - 1 ) / alignment );
}
/***************************************************************************/

}  // namespace WebGPUlib