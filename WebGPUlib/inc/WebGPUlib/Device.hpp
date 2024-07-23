#pragma once

#include "webgpu/webgpu.h"

#include <memory>
#include <span>

struct SDL_Window;

namespace WebGPUlib
{

class Queue;
class IndexBuffer;
class Surface;
class VertexBuffer;

class Device
{
public:
    Device( SDL_Window* window );
    ~Device();

    // Get the device queue.
    std::shared_ptr<Queue> getQueue() const;

    // Get the surface.
    std::shared_ptr<Surface> getSurface() const;

    template<typename T>
    std::shared_ptr<VertexBuffer> createVertexBuffer( std::span<const T> vertices ) const;
    std::shared_ptr<VertexBuffer> createVertexBuffer( const void* vertexData, std::size_t size,
                                                      std::size_t vertexCount ) const;

    template<typename T>
    std::shared_ptr<IndexBuffer> createIndexBuffer( std::span<const T> indices ) const;
    std::shared_ptr<IndexBuffer> createIndexBuffer( const void* indexData, std::size_t size,
                                                    std::size_t indexCount ) const;

    // Poll the GPU to allow work to be done on the device queue.
    void poll( bool sleep = false );

protected:
private:
    static void onDeviceLostCallback( WGPUDevice const* device, WGPUDeviceLostReason reason, char const* message,
                                      void* userdata );
    static void onGPUErrorCallback( WGPUErrorType type, const char* message, void* userdata );

    WGPUInstance             instance = nullptr;
    WGPUAdapter              adapter  = nullptr;
    WGPUDevice               device   = nullptr;
    std::shared_ptr<Queue>   queue    = nullptr;
    std::shared_ptr<Surface> surface  = nullptr;
};

template<typename T>
std::shared_ptr<VertexBuffer> Device::createVertexBuffer( std::span<const T> vertices ) const
{
    return createVertexBuffer( vertices.data(), vertices.size_bytes(), vertices.size() );
}

template<typename T>
std::shared_ptr<IndexBuffer> Device::createIndexBuffer( std::span<const T> indices ) const
{
    return createIndexBuffer( indices.data(), indices.size_bytes(), indices.size() );
}

}  // namespace WebGPUlib
