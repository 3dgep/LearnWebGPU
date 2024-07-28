#pragma once

#include <webgpu/webgpu.h>

#include <memory>
#include <vector>

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
    std::shared_ptr<VertexBuffer> createVertexBuffer( const std::vector<T>& vertices ) const;
    std::shared_ptr<VertexBuffer> createVertexBuffer( const void* vertexData, std::size_t vertexCount,
                                                      std::size_t vertexStride ) const;

    template<typename T>
    std::shared_ptr<IndexBuffer> createIndexBuffer( const std::vector<T>& indices ) const;
    std::shared_ptr<IndexBuffer> createIndexBuffer( const void* indexData, std::size_t indexCount,
                                                    std::size_t indexStride ) const;

    // Poll the GPU to allow work to be done on the device queue.
    void poll( bool sleep = false );

protected:
private:
    static void onDeviceLostCallback( WGPUDeviceLostReason reason, char const* message, void* userdata );
    static void onUncapturedErrorCallback( WGPUErrorType type, const char* message, void* userdata );

    WGPUInstance             instance = nullptr;
    WGPUAdapter              adapter  = nullptr;
    WGPUDevice               device   = nullptr;
    std::shared_ptr<Queue>   queue    = nullptr;
    std::shared_ptr<Surface> surface  = nullptr;
};

template<typename T>
std::shared_ptr<VertexBuffer> Device::createVertexBuffer( const std::vector<T>& vertices ) const
{
    return createVertexBuffer( vertices.data(), vertices.size(), sizeof( T ) );
}

template<typename T>
std::shared_ptr<IndexBuffer> Device::createIndexBuffer( const std::vector<T>& indices ) const
{
    return createIndexBuffer( indices.data(), indices.size(), sizeof( T ) );
}

}  // namespace WebGPUlib
