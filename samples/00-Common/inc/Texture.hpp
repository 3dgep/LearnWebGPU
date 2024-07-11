#pragma once

#include <webgpu/webgpu.h>

class Texture
{
public:
    Texture() = default;


private:
    WGPUTexture texture = nullptr;
    WGPUTextureView textureView = nullptr;

};