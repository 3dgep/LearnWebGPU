#include <WebGPUlib/Device.hpp>
#include <WebGPUlib/IndexBuffer.hpp>
#include <WebGPUlib/Queue.hpp>
#include <WebGPUlib/Surface.hpp>
#include <WebGPUlib/VertexBuffer.hpp>

#ifdef __EMSCRIPTEN__
    #include <emscripten/emscripten.h>
#endif
#ifdef WEBGPU_BACKEND_WGPU
    #include <webgpu/wgpu.h>  // Include non-standard functions.
#endif
#include <sdl2webgpu.h>

#include <cassert>
#include <filesystem>
#include <iostream>

using namespace WebGPUlib;

std::unique_ptr<Device> pDevice { nullptr };

struct MakeQueue : Queue
{
    MakeQueue( WGPUQueue&& queue )
    : Queue( std::move( queue ) )  // NOLINT(performance-move-const-arg)
    {}
};

struct MakeSurface : Surface
{
    MakeSurface( WGPUSurface&& surface, const WGPUSurfaceConfiguration& config, SDL_Window* window )
    : Surface( std::move( surface ), config, window )  // NOLINT(performance-move-const-arg)
    {}
};

struct MakeVertexBuffer : VertexBuffer
{
    MakeVertexBuffer( WGPUBuffer&& buffer, std::size_t vertexCount, std::size_t vertexStride )
    : VertexBuffer( std::move( buffer ), vertexCount, vertexStride )  // NOLINT(performance-move-const-arg)
    {}
};

struct MakeIndexBuffer : IndexBuffer
{
    MakeIndexBuffer( WGPUBuffer&& buffer, std::size_t indexCount, std::size_t indexStride )
    : IndexBuffer( std::move( buffer ), indexCount, indexStride )  // NOLINT(performance-move-const-arg)
    {}
};

void Device::create( SDL_Window* window )
{
    assert( !pDevice );
    pDevice = std::unique_ptr<Device>( new Device( window ) );
}

void Device::destroy()
{
    pDevice.reset();
}

Device& Device::get()
{
    assert( pDevice );
    return *pDevice;
}

Device::Device( SDL_Window* window )
{
#ifdef WEBGPU_BACKEND_EMSCRIPTEN
    // For some reason, the instance descriptor must be null when using emscripten.
    instance = wgpuCreateInstance( nullptr );
#else
    WGPUInstanceDescriptor instanceDescriptor {};

    #ifdef WEBGPU_BACKEND_DAWN
    // Make sure the uncaptured error callback is called as soon as an error
    // occurs, rather than waiting for the next Tick. This enables using the
    // stack trace in which the uncaptured error occurred when breaking into the
    // uncaptured error callback.
    const char*               enabledToggles[] = { "enable_immediate_error_handling" };
    WGPUDawnTogglesDescriptor toggles {};
    toggles.chain.next             = nullptr;
    toggles.chain.sType            = WGPUSType_DawnTogglesDescriptor;
    toggles.disabledToggleCount    = 0;
    toggles.enabledToggleCount     = std::size( enabledToggles );
    toggles.enabledToggles         = enabledToggles;
    instanceDescriptor.nextInChain = &toggles.chain;
    #endif

    instance = wgpuCreateInstance( &instanceDescriptor );
#endif

    if ( !instance )
    {
        std::cerr << "Failed to create WebGPU instance." << std::endl;
        return;
    }

    // Used by the requestAdapterCallback function to store the adapter and to notify
    // us when the request is complete.
    struct AdapterData
    {
        WGPUAdapter adapter = nullptr;
        bool        done    = false;
    } adapterData;

    WGPURequestAdapterOptions requestAdapterOptions {};
    requestAdapterOptions.backendType     = WGPUBackendType_Undefined;  // WGPUBackendType_Vulkan;
    requestAdapterOptions.powerPreference = WGPUPowerPreference_HighPerformance;

    wgpuInstanceRequestAdapter(
        instance, &requestAdapterOptions,
        []( WGPURequestAdapterStatus status, WGPUAdapter adapter, const char* message, void* userData ) {
            auto& data = *static_cast<AdapterData*>( userData );
            if ( status == WGPURequestAdapterStatus_Success )
            {
                data.adapter = adapter;
            }
            else
            {
                std::cerr << "Failed to request adapter: " << message << std::endl;
            }
            data.done = true;
        },
        &adapterData );

    // When using Emscripten, we have to wait for the request to complete.
#ifdef __EMSCRIPTEN__
    while ( !adapterData.done )
    {
        emscripten_sleep( 100 );
    }
#endif

    // The request must complete.
    assert( adapterData.done );
    adapter = adapterData.adapter;

    // Create a minimal device with no special features and default limits.
    WGPUDeviceDescriptor deviceDescriptor {};
    deviceDescriptor.label                    = "WebGPUlib";  // You can use anything here.
    deviceDescriptor.requiredFeatureCount     = 0;            // We don't require any extra features.
    deviceDescriptor.requiredFeatures         = nullptr;
    deviceDescriptor.requiredLimits           = nullptr;  // We don't require any specific limits.
    deviceDescriptor.defaultQueue.nextInChain = nullptr;
    deviceDescriptor.defaultQueue.label       = "Queue";  // You can use anything here.
    deviceDescriptor.deviceLostCallback       = onDeviceLostCallback;

    struct DeviceData
    {
        WGPUDevice device = nullptr;
        bool       done   = false;
    } deviceData;

    wgpuAdapterRequestDevice(
        adapter, &deviceDescriptor,
        []( WGPURequestDeviceStatus _status, WGPUDevice _device, const char* _message, void* _userData ) {
            auto& data = *static_cast<DeviceData*>( _userData );
            if ( _status == WGPURequestDeviceStatus_Success )
            {
                data.device = _device;
            }
            else
            {
                std::cerr << "Failed to request device: " << _message << std::endl;
            }
            data.done = true;
        },
        &deviceData );

    // When using Emscripten, we have to wait for the request to complete.
#ifdef __EMSCRIPTEN__
    while ( !deviceData.done )
    {
        emscripten_sleep( 100 );
    }
#endif

    // The request must complete.
    assert( deviceData.done );
    device = deviceData.device;

    if ( !device )
    {
        std::cerr << "Failed to create device." << std::endl;
        return;
    }

    // Set the uncaptured error callback.
    wgpuDeviceSetUncapturedErrorCallback( device, onUncapturedErrorCallback, nullptr );

    // Configure the surface.
    WGPUSurface _surface = SDL_GetWGPUSurface( instance, window );

    if ( !_surface )
    {
        std::cerr << "Failed to get the surface." << std::endl;
        return;
    }

    int windowWidth, windowHeight;
    SDL_GetWindowSize( window, &windowWidth, &windowHeight );

    WGPUSurfaceCapabilities surfaceCapabilities {};
    wgpuSurfaceGetCapabilities( _surface, adapter, &surfaceCapabilities );
    WGPUTextureFormat surfaceFormat = surfaceCapabilities.formats[0];

    // Set the surface configuration.
    WGPUSurfaceConfiguration surfaceConfiguration {};
    surfaceConfiguration.device          = device;
    surfaceConfiguration.format          = surfaceFormat;
    surfaceConfiguration.usage           = WGPUTextureUsage_RenderAttachment;
    surfaceConfiguration.viewFormatCount = 0;
    surfaceConfiguration.viewFormats     = nullptr;
    surfaceConfiguration.alphaMode       = WGPUCompositeAlphaMode_Auto;
    surfaceConfiguration.width           = windowWidth;
    surfaceConfiguration.height          = windowHeight;
    surfaceConfiguration.presentMode     = WGPUPresentMode_Fifo;  // This must be Fifo on Emscripten.

    wgpuSurfaceConfigure( _surface, &surfaceConfiguration );

    surface = std::make_shared<MakeSurface>( std::move( _surface ),  // NOLINT(performance-move-const-arg)
                                             surfaceConfiguration, window );

    // Get the device queue.
    WGPUQueue _queue = wgpuDeviceGetQueue( device );

    if ( !_queue )
    {
        std::cerr << "Failed to get device queue." << std::endl;
        return;
    }

    queue = std::make_shared<MakeQueue>( std::move( _queue ) );  // NOLINT(performance-move-const-arg)
}

Device::~Device()
{
    surface.reset();
    queue.reset();

    if ( device )
        wgpuDeviceRelease( device );

    if ( adapter )
        wgpuAdapterRelease( adapter );

    if ( instance )
        wgpuInstanceRelease( instance );
}

std::shared_ptr<Queue> Device::getQueue() const
{
    return queue;
}

std::shared_ptr<Surface> Device::getSurface() const
{
    return surface;
}

std::shared_ptr<VertexBuffer> Device::createVertexBuffer( const void* vertexData, std::size_t vertexCount,
                                                          std::size_t vertexStride ) const
{
    std::size_t          size = vertexCount * vertexStride;
    WGPUBufferDescriptor bufferDescriptor {};
    bufferDescriptor.size  = size;
    bufferDescriptor.usage = WGPUBufferUsage_Vertex | WGPUBufferUsage_CopyDst;
    WGPUBuffer buffer      = wgpuDeviceCreateBuffer( device, &bufferDescriptor );

    auto vertexBuffer = std::make_shared<MakeVertexBuffer>( std::move( buffer ),  // NOLINT(performance-move-const-arg)
                                                            vertexCount, vertexStride );

    queue->writeBuffer( vertexBuffer, vertexData, size );

    return vertexBuffer;
}

std::shared_ptr<IndexBuffer> Device::createIndexBuffer( const void* indexData, std::size_t indexCount,
                                                        std::size_t indexStride ) const
{
    std::size_t          size = indexCount * indexStride;
    WGPUBufferDescriptor bufferDescriptor {};
    bufferDescriptor.size  = size;
    bufferDescriptor.usage = WGPUBufferUsage_Index | WGPUBufferUsage_CopyDst;
    WGPUBuffer buffer      = wgpuDeviceCreateBuffer( device, &bufferDescriptor );

    auto indexBuffer = std::make_shared<MakeIndexBuffer>( std::move( buffer ),  // NOLINT(performance-move-const-arg)
                                                          indexCount, indexStride );

    queue->writeBuffer( indexBuffer, indexData, size );

    return indexBuffer;
}

void Device::poll( bool sleep )
{
#if defined( WEBGPU_BACKEND_DAWN )
    wgpuDeviceTick( device );
#elif defined( WEBGPU_BACKEND_WGPU )
    wgpuDevicePoll( device, false, nullptr );
#elif defined( WEBGPU_BACKEND_EMSCRIPTEN )
    if ( sleep )
    {
        emscripten_sleep( 100 );
    }
#endif
}

void WebGPUlib::Device::onDeviceLostCallback( WGPUDeviceLostReason reason, char const* message, void* userdata )
{
    std::cerr << "Device lost: " << std::hex << reason << std::dec;
    if ( message )
        std::cerr << " (" << message << ")";
    std::cerr << std::endl;
}

void WebGPUlib::Device::onUncapturedErrorCallback( WGPUErrorType type, const char* message, void* userdata )
{
    std::cerr << "Uncaptured device error: " << std::hex << type << std::dec;
    if ( message )
        std::cerr << " (" << message << ")";
    std::cerr << std::endl;
}