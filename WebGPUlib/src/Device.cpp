#include <WebGPULib/Device.hpp>

#include <cassert>
#include <filesystem>
#include <iostream>

using namespace WebGPUlib;

Device::Device()
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

    wgpuInstanceRequestAdapter(
        instance, nullptr,
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
    while ( !userData.done )
    {
        emscripten_sleep( 100 );
    }
#endif

    // The request must complete.
    assert( adapterData.done );
    WGPUAdapter adapter = adapterData.adapter;

    // Create a minimal device with no special features and default limits.
    WGPUDeviceDescriptor deviceDescriptor {};
    deviceDescriptor.label                    = "WebGPUlib";  // You can use anything here.
    deviceDescriptor.requiredFeatureCount     = 0;            // We don't require any extra features.
    deviceDescriptor.requiredFeatures         = nullptr;
    deviceDescriptor.requiredLimits           = nullptr;  // We don't require any specific limits.
    deviceDescriptor.defaultQueue.nextInChain = nullptr;
    deviceDescriptor.defaultQueue.label       = "Queue";  // You can use anything here.
    deviceDescriptor.deviceLostCallbackInfo   = {
          .mode     = WGPUCallbackMode_WaitAnyOnly,
          .callback = onDeviceLostCallback,
          .userdata = nullptr,
    };
    deviceDescriptor.uncapturedErrorCallbackInfo = {
        .callback = onGPUErrorCallback,
        .userdata = nullptr,
    };

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
    while ( !userData.done )
    {
        emscripten_sleep( 100 );
    }
#endif

    // We are done with the adapter. It is safe to release it.
    wgpuAdapterRelease( adapter );

    // The request must complete.
    assert( deviceData.done );
    device = deviceData.device;

    if ( !device )
    {
        std::cerr << "Failed to create device." << std::endl;
        return;
    }

    // Get the device queue.
    queue = wgpuDeviceGetQueue( device );

    if ( !queue )
    {
        std::cerr << "Failed to get device queue." << std::endl;
        return;
    }
}

Device::~Device()
{
    if ( queue )
    {
        wgpuQueueRelease( queue );
    }

    if ( device )
    {
        wgpuDeviceRelease( device );
    }

    if ( instance )
    {
        wgpuInstanceRelease( instance );
    }
}

Device& Device::get()
{
    static Device device{};
    return device;
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

void WebGPUlib::Device::onDeviceLostCallback( WGPUDevice const* device, WGPUDeviceLostReason reason,
                                              char const* message, void* userdata )
{
    std::cerr << "Device lost: " << std::hex << reason << std::dec;
    if ( message )
        std::cerr << " (" << message << ")";
    std::cerr << std::endl;
}

void WebGPUlib::Device::onGPUErrorCallback( WGPUErrorType type, const char* message, void* userdata )
{
    std::cerr << "Uncaptured device error: " << std::hex << type << std::dec;
    if ( message )
        std::cerr << " (" << message << ")";
    std::cerr << std::endl;
}