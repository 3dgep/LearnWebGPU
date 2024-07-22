#pragma once

#include "webgpu/webgpu.h"

namespace WebGPUlib
{
class Device
{
public:

    // Get the singleton instance of the device.
    static Device& get();

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
}  // namespace WebGPUlib
