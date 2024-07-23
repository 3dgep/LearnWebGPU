#pragma once

#include "webgpu/webgpu.h"

#include <memory>
#include <span>

namespace WebGPUlib
{

class Queue;
class IndexBuffer;
class VertexBuffer;

class Device
{
public:
    // Get the singleton instance of the device.
    static Device& get();

    // Get the device queue.
    std::shared_ptr<Queue> getQueue();

    template<typename T>
    std::shared_ptr<VertexBuffer> createVertexBuffer( std::span<const T> vertices ) const;
    std::shared_ptr<VertexBuffer> createVertexBuffer( const void* vertexData, std::size_t size ) const;

    template<typename T>
    std::shared_ptr<IndexBuffer> createIndexBuffer( std::span<const T> indices ) const;
    std::shared_ptr<IndexBuffer> createIndexBuffer( const void* indexData, std::size_t size ) const;

    // Poll the GPU to allow work to be done on the device queue.
    void poll( bool sleep = false );

protected:
    Device();
    ~Device();

private:
    static void onDeviceLostCallback( WGPUDevice const* device, WGPUDeviceLostReason reason, char const* message,
                                      void* userdata );
    static void onGPUErrorCallback( WGPUErrorType type, const char* message, void* userdata );

    WGPUInstance instance = nullptr;
    WGPUDevice   device   = nullptr;
    WGPUQueue    queue    = nullptr;
};

template<typename T>
std::shared_ptr<VertexBuffer> Device::createVertexBuffer( std::span<const T> vertices ) const
{
    return createVertexBuffer( vertices.data(), vertices.size_bytes() );
}

template<typename T>
std::shared_ptr<IndexBuffer> Device::createIndexBuffer( std::span<const T> indices ) const
{
    return createIndexBuffer( indices.data(), indices.size_bytes() );
}

}  // namespace WebGPUlib
