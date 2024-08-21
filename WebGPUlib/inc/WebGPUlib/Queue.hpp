#pragma once

#include "../bitmask_operators.hpp"
#include <webgpu/webgpu.h>

#include <memory>

namespace WebGPUlib
{
class Buffer;
class RenderTarget;
class Texture;
class CommandBuffer;
class GraphicsCommandBuffer;
class ComputeCommandBuffer;

enum class ClearFlags
{
    None         = 0,
    Color        = 1 << 0,
    Depth        = 1 << 2,
    Stencil      = 1 << 3,
    DepthStencil = Depth | Stencil,
    All          = Color | Depth | Stencil
};

class Queue
{
public:
    template<typename T>
    void writeBuffer( Buffer& buffer, const T& data ) const;
    void writeBuffer( const Buffer& buffer, const void* data, std::size_t size, uint64_t offset = 0 ) const;

    void writeTexture( Texture& texture, uint32_t mip, const void* data, std::size_t size ) const;

    std::shared_ptr<GraphicsCommandBuffer> createGraphicsCommandBuffer( const RenderTarget& renderTarget,
                                                                        ClearFlags       clearFlags = ClearFlags::All,
                                                                        const WGPUColor& clearColor = { 0, 0, 0, 0 },
                                                                        float            depth      = 1.0f,
                                                                        uint32_t         stencil    = 0 ) const;

    std::shared_ptr<ComputeCommandBuffer> createComputeCommandBuffer();

    void submit( CommandBuffer& commandBuffer );

    WGPUQueue getWGPUQueue() const
    {
        return queue;
    }

protected:
    Queue( WGPUQueue&& queue );
    virtual ~Queue();

private:
    WGPUQueue queue;
};

template<typename T>
void Queue::writeBuffer( Buffer& buffer, const T& data ) const
{
    writeBuffer( buffer, &data, sizeof( T ) );
}

}  // namespace WebGPUlib

template<>
struct enable_bitmask_operators<WebGPUlib::ClearFlags>
{
    static const bool enable = true;
};