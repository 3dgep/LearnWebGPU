#pragma once

#include <filesystem>
#include <webgpu/webgpu.h>

#include <memory>
#include <vector>

struct SDL_Window;

namespace WebGPUlib
{

class Queue;
class IndexBuffer;
class Mesh;
class Sampler;
class Surface;
class Texture;
class UniformBuffer;
class VertexBuffer;

class Device
{
public:
    Device()                           = delete;
    Device( const Device& )            = delete;
    Device( Device&& )                 = delete;
    Device& operator=( const Device& ) = delete;
    Device& operator=( Device&& )      = delete;

    static void    create( SDL_Window* window );
    static void    destroy();
    static Device& get();

    // Get the device queue.
    std::shared_ptr<Queue> getQueue() const;

    // Get the surface.
    std::shared_ptr<Surface> getSurface() const;

    std::shared_ptr<Mesh> createCube( float size = 1.0f, bool reverseWinding = false ) const;

    std::shared_ptr<Texture> createTexture( const WGPUTextureDescriptor& textureDescriptor );

    std::shared_ptr<Texture> loadTexture( const std::filesystem::path& filePath );

    void generateMips( const Texture& texture );

    template<typename T>
    std::shared_ptr<VertexBuffer> createVertexBuffer( const std::vector<T>& vertices ) const;
    std::shared_ptr<VertexBuffer> createVertexBuffer( const void* vertexData, std::size_t vertexCount,
                                                      std::size_t vertexStride ) const;

    template<typename T>
    std::shared_ptr<IndexBuffer> createIndexBuffer( const std::vector<T>& indices ) const;
    std::shared_ptr<IndexBuffer> createIndexBuffer( const void* indexData, std::size_t indexCount,
                                                    std::size_t indexStride ) const;

    template<typename T>
    std::shared_ptr<UniformBuffer> createUniformBuffer( const T& data ) const;
    std::shared_ptr<UniformBuffer> createUniformBuffer( const void* data, std::size_t size ) const;

    std::shared_ptr<Sampler> createSampler( const WGPUSamplerDescriptor& samplerDescriptor ) const;

    void poll( bool sleep = false );

    WGPUInstance getWGPUInstance() const noexcept
    {
        return instance;
    }

    WGPUDevice getWGPUDevice() const noexcept
    {
        return device;
    }

private:
    friend struct std::default_delete<Device>;
    Device( SDL_Window* window );
    ~Device();

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

template<typename T>
std::shared_ptr<UniformBuffer> Device::createUniformBuffer( const T& data ) const
{
    return createUniformBuffer( &data, sizeof( T ) );
}

}  // namespace WebGPUlib
