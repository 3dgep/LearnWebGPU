#include "WebGPUlib/ComputeCommandBuffer.hpp"

#include <WebGPUlib/Device.hpp>
#include <WebGPUlib/GenerateMipsPipelineState.hpp>

using namespace WebGPUlib;

GenerateMipsPipelineState::GenerateMipsPipelineState()
{
    // Load the shader module.
    const char* shaderCode = {
#include "../shaders/GenerateMips.wgsl"
    };

    auto device = Device::get().getWGPUDevice();

    // Load the compute shader module.
    WGPUShaderModuleWGSLDescriptor shaderCodeDesc {};
    shaderCodeDesc.chain.next  = nullptr;
    shaderCodeDesc.chain.sType = WGPUSType_ShaderModuleWGSLDescriptor;
    shaderCodeDesc.code        = shaderCode;

    WGPUShaderModuleDescriptor shaderModuleDesc {};
    shaderModuleDesc.nextInChain  = &shaderCodeDesc.chain;
    shaderModuleDesc.label        = "Generate Mips Shader Module";
    WGPUShaderModule shaderModule = wgpuDeviceCreateShaderModule( device, &shaderModuleDesc );

    // Setup the binding layout for the generate mips compute shader.
    //@group(0) @binding(0) var<uniform> mip : Mip;
    //@group(0) @binding(1) var srcMip : texture_2d<f32>;
    //@group(0) @binding(2) var dstMip1 : texture_storage_2d<rgba8unorm, write>;
    //@group(0) @binding(3) var dstMip2 : texture_storage_2d<rgba8unorm, write>;
    //@group(0) @binding(4) var dstMip3 : texture_storage_2d<rgba8unorm, write>;
    //@group(0) @binding(5) var dstMip4 : texture_storage_2d<rgba8unorm, write>;
    //@group(0) @binding(6) var linearClampSampler : sampler;
    WGPUBindGroupLayoutEntry bindGroupLayoutEntries[7] {};
    bindGroupLayoutEntries[0].binding               = 0;
    bindGroupLayoutEntries[0].visibility            = WGPUShaderStage_Compute;
    bindGroupLayoutEntries[0].buffer.type           = WGPUBufferBindingType_Uniform;
    bindGroupLayoutEntries[0].buffer.minBindingSize = sizeof( Mip );

    bindGroupLayoutEntries[1].binding               = 1;
    bindGroupLayoutEntries[1].visibility            = WGPUShaderStage_Compute;
    bindGroupLayoutEntries[1].texture.sampleType    = WGPUTextureSampleType_Float;
    bindGroupLayoutEntries[1].texture.viewDimension = WGPUTextureViewDimension_2D;

    for ( int i = 2; i <= 5; ++i )
    {
        bindGroupLayoutEntries[i].binding                      = i;
        bindGroupLayoutEntries[i].visibility                   = WGPUShaderStage_Compute;
        bindGroupLayoutEntries[i].storageTexture.access        = WGPUStorageTextureAccess_WriteOnly;
        bindGroupLayoutEntries[i].storageTexture.format        = WGPUTextureFormat_RGBA8Unorm;
        bindGroupLayoutEntries[i].storageTexture.viewDimension = WGPUTextureViewDimension_2D;
    }

    bindGroupLayoutEntries[6].binding      = 6;
    bindGroupLayoutEntries[6].visibility   = WGPUShaderStage_Compute;
    bindGroupLayoutEntries[6].sampler.type = WGPUSamplerBindingType_Filtering;

    WGPUBindGroupLayoutDescriptor bindGroupLayoutDesc {};
    bindGroupLayoutDesc.label      = "Generate Mips Bind Group Layout";
    bindGroupLayoutDesc.entryCount = std::size( bindGroupLayoutEntries );
    bindGroupLayoutDesc.entries    = bindGroupLayoutEntries;
    bindGroupLayout                = wgpuDeviceCreateBindGroupLayout( device, &bindGroupLayoutDesc );

    // Setup the pipeline layout.
    WGPUPipelineLayoutDescriptor pipelineLayoutDesc {};
    pipelineLayoutDesc.label                = "Generate Mips Pipeline Layout";
    pipelineLayoutDesc.bindGroupLayoutCount = 1;
    pipelineLayoutDesc.bindGroupLayouts     = &bindGroupLayout;
    WGPUPipelineLayout pipelineLayout       = wgpuDeviceCreatePipelineLayout( device, &pipelineLayoutDesc );

    // Setup the pipeline state.
    WGPUComputePipelineDescriptor pipelineDesc {};
    pipelineDesc.label              = "Generate Mips Pipeline";
    pipelineDesc.layout             = pipelineLayout;
    pipelineDesc.compute.module     = shaderModule;
    pipelineDesc.compute.entryPoint = "main";
    pipeline                        = wgpuDeviceCreateComputePipeline( device, &pipelineDesc );

    // We are done with the shader module.
    wgpuShaderModuleRelease( shaderModule );
    // We are done with the pipeline layout.
    wgpuPipelineLayoutRelease( pipelineLayout );
}

GenerateMipsPipelineState::~GenerateMipsPipelineState()
{
    if ( bindGroupLayout )
        wgpuBindGroupLayoutRelease( bindGroupLayout );
}

void GenerateMipsPipelineState::bind( ComputeCommandBuffer& commandBuffer )
{
    auto passEncoder = commandBuffer.getWGPUPassEncoder();
    wgpuComputePassEncoderSetPipeline( passEncoder, pipeline );
}