#pragma once

#include "../bitmask_operators.hpp"
#include <webgpu/webgpu.h>

#include <memory>

namespace WebGPUlib
{
class Buffer;
class GraphicsCommandBuffer;
class RenderTarget;

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
    void writeBuffer( const std::shared_ptr<Buffer>& buffer, const void* data, std::size_t size ) const;

    std::shared_ptr<GraphicsCommandBuffer> createGraphicsCommandBuffer( const RenderTarget& renderTarget,
                                                                        ClearFlags       clearFlags = ClearFlags::All,
                                                                        const WGPUColor& clearColor = { 0, 0, 0, 0 },
                                                                        float            depth      = 1.0f,
                                                                        uint32_t         stencil    = 0 ) const;

protected:
    Queue( WGPUQueue&& queue );
    virtual ~Queue();

private:
    WGPUQueue queue;
};
}  // namespace WebGPUlib

template<>
struct enable_bitmask_operators<WebGPUlib::ClearFlags>
{
    static const bool enable = true;
};