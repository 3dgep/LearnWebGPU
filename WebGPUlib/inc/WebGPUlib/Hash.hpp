#pragma once

#include <webgpu/webgpu.h>

#include <functional> // std::hash

namespace std
{
// Source: https://stackoverflow.com/questions/2590677/how-do-i-combine-hash-values-in-c0x
template<typename T>
void hash_combine( std::size_t& seed, const T& v )  // NOLINT(cert-dcl58-cpp)
{
    std::hash<T> hasher;
    seed ^= hasher( v ) + 0x9e3779b9 + ( seed << 6 ) + ( seed >> 2 );
}

template<>
struct hash<WGPUTextureViewDescriptor>
{
    std::size_t operator()( const WGPUTextureViewDescriptor& textureViewDescriptor ) const noexcept
    {
        std::size_t seed = 0;
        hash_combine( seed, textureViewDescriptor.format );
        hash_combine( seed, textureViewDescriptor.dimension );
        hash_combine( seed, textureViewDescriptor.aspect );
        hash_combine( seed, textureViewDescriptor.baseMipLevel );
        hash_combine( seed, textureViewDescriptor.mipLevelCount );
        hash_combine( seed, textureViewDescriptor.baseArrayLayer );
        hash_combine( seed, textureViewDescriptor.arrayLayerCount );
        return seed;
    }
};

}  // namespace std

inline bool operator<( const WGPUTextureViewDescriptor& lhs, const WGPUTextureViewDescriptor& rhs ) noexcept
{
    if ( lhs.format != rhs.format )
        return lhs.format < rhs.format;

    if ( lhs.dimension != rhs.dimension )
        return lhs.dimension < rhs.dimension;

    if ( lhs.aspect != rhs.aspect )
        return lhs.aspect < rhs.aspect;

    if ( lhs.baseMipLevel != rhs.baseMipLevel )
        return lhs.baseMipLevel < rhs.baseMipLevel;

    if ( lhs.mipLevelCount != rhs.mipLevelCount )
        return lhs.mipLevelCount < rhs.mipLevelCount;

    if ( lhs.baseArrayLayer != rhs.baseArrayLayer )
        return lhs.baseArrayLayer < rhs.baseArrayLayer;

    return lhs.arrayLayerCount < rhs.arrayLayerCount;
}

inline bool operator==( const WGPUTextureViewDescriptor& lhs, const WGPUTextureViewDescriptor& rhs ) noexcept
{
    return lhs.format == rhs.format
        && lhs.dimension == rhs.dimension
        && lhs.aspect == rhs.aspect
        && lhs.baseMipLevel == rhs.baseMipLevel
        && lhs.mipLevelCount == rhs.mipLevelCount
        && lhs.baseArrayLayer == rhs.baseArrayLayer
        && lhs.arrayLayerCount == rhs.arrayLayerCount;
}