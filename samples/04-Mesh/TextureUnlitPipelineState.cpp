#include "TextureUnlitPipelineState.hpp"

#include <WebGPUlib/Device.hpp>
#include <WebGPUlib/GraphicsCommandBuffer.hpp>
#include <WebGPUlib/Surface.hpp>
#include <WebGPUlib/Vertex.hpp>

#include <glm/mat4x4.hpp>

using namespace WebGPUlib;

TextureUnlitPipelineState::TextureUnlitPipelineState()
{
    const char* shaderCode = {
#include "TextureUnlitShader.wgsl"
    };

    WGPUDevice        device        = Device::get().getWGPUDevice();
    WGPUTextureFormat surfaceFormat = Device::get().getSurface()->getSurfaceFormat();

    // Load the shader module.
    WGPUShaderModuleWGSLDescriptor shaderCodeDesc {};
    shaderCodeDesc.chain.sType = WGPUSType_ShaderModuleWGSLDescriptor;
    shaderCodeDesc.chain.next  = nullptr;
    shaderCodeDesc.code        = shaderCode;

    WGPUShaderModuleDescriptor shaderModuleDescriptor {};
    shaderModuleDescriptor.nextInChain = &shaderCodeDesc.chain;
    WGPUShaderModule shaderModule      = wgpuDeviceCreateShaderModule( device, &shaderModuleDescriptor );

    // Setup the binding layout.
    // @group( 0 ) @binding( 0 ) var<uniform> mvp : mat4x4f;
    // @group( 0 ) @binding( 1 ) var          albedoTexture : texture_2d<f32>;
    // @group( 0 ) @binding( 2 ) var          linearRepeatSampler : sampler;
    WGPUBindGroupLayoutEntry bindGroupLayoutEntries[3] {};
    bindGroupLayoutEntries[0].binding               = 0;
    bindGroupLayoutEntries[0].visibility            = WGPUShaderStage_Vertex;
    bindGroupLayoutEntries[0].buffer.type           = WGPUBufferBindingType_Uniform;
    bindGroupLayoutEntries[0].buffer.minBindingSize = sizeof( glm::mat4 );

    bindGroupLayoutEntries[1].binding               = 1;
    bindGroupLayoutEntries[1].visibility            = WGPUShaderStage_Fragment;
    bindGroupLayoutEntries[1].texture.sampleType    = WGPUTextureSampleType_Float;
    bindGroupLayoutEntries[1].texture.viewDimension = WGPUTextureViewDimension_2D;

    bindGroupLayoutEntries[2].binding      = 2;
    bindGroupLayoutEntries[2].visibility   = WGPUShaderStage_Fragment;
    bindGroupLayoutEntries[2].sampler.type = WGPUSamplerBindingType_Filtering;

    // Setup the binding group.
    WGPUBindGroupLayoutDescriptor bindGroupLayoutDescriptor {};
    bindGroupLayoutDescriptor.entryCount = std::size( bindGroupLayoutEntries );
    bindGroupLayoutDescriptor.entries    = bindGroupLayoutEntries;
    bindGroupLayout                      = wgpuDeviceCreateBindGroupLayout( device, &bindGroupLayoutDescriptor );

    // Setup the pipeline layout.
    WGPUPipelineLayoutDescriptor pipelineLayoutDescriptor {};
    pipelineLayoutDescriptor.bindGroupLayoutCount = 1;
    pipelineLayoutDescriptor.bindGroupLayouts     = &bindGroupLayout;
    WGPUPipelineLayout pipelineLayout             = wgpuDeviceCreatePipelineLayout( device, &pipelineLayoutDescriptor );

    // Setup the vertex layout.
    // glm::vec3 position;
    // glm::vec3 normal;
    // glm::vec3 texCoord;
    WGPUVertexAttribute vertexAttributes[3] {};
    // glm::vec3 position;
    vertexAttributes[0].format         = WGPUVertexFormat_Float32x3;
    vertexAttributes[0].offset         = offsetof( VertexPositionNormalTexture, position );
    vertexAttributes[0].shaderLocation = 0;

    // glm::vec3 normal;
    vertexAttributes[1].format         = WGPUVertexFormat_Float32x3;
    vertexAttributes[1].offset         = offsetof( VertexPositionNormalTexture, normal );
    vertexAttributes[1].shaderLocation = 1;

    // glm::vec3 texCoord;
    vertexAttributes[2].format         = WGPUVertexFormat_Float32x3;
    vertexAttributes[2].offset         = offsetof( VertexPositionNormalTexture, texCoord );
    vertexAttributes[2].shaderLocation = 2;

    WGPUVertexBufferLayout vertexBufferLayout {};
    vertexBufferLayout.arrayStride    = sizeof( VertexPositionNormalTexture );
    vertexBufferLayout.stepMode       = WGPUVertexStepMode_Vertex;
    vertexBufferLayout.attributeCount = std::size( vertexAttributes );
    vertexBufferLayout.attributes     = vertexAttributes;

    WGPUPrimitiveState primitiveState {};
    primitiveState.topology         = WGPUPrimitiveTopology_TriangleList;
    primitiveState.stripIndexFormat = WGPUIndexFormat_Undefined;
    primitiveState.frontFace        = WGPUFrontFace_CCW;
    primitiveState.cullMode         = WGPUCullMode_Back;

    // Setup the vertex shader stage.
    WGPUVertexState vertexState {};
    vertexState.module        = shaderModule;
    vertexState.entryPoint    = "vs_main";
    vertexState.constantCount = 0;
    vertexState.constants     = nullptr;
    vertexState.constants     = nullptr;
    vertexState.bufferCount   = 1;
    vertexState.buffers       = &vertexBufferLayout;

    WGPUColorTargetState colorTargetState {};
    colorTargetState.format    = surfaceFormat;
    colorTargetState.blend     = nullptr;
    colorTargetState.writeMask = WGPUColorWriteMask_All;

    // Setup the fragment shader stage.
    WGPUFragmentState fragmentState {};
    fragmentState.module        = shaderModule;
    fragmentState.entryPoint    = "fs_main";
    fragmentState.constantCount = 0;
    fragmentState.constants     = nullptr;
    fragmentState.targetCount   = 1;
    fragmentState.targets       = &colorTargetState;

    // Setup stencil face state.
    WGPUStencilFaceState stencilFaceState {};
    stencilFaceState.compare     = WGPUCompareFunction_Always;
    stencilFaceState.failOp      = WGPUStencilOperation_Keep;
    stencilFaceState.depthFailOp = WGPUStencilOperation_Keep;
    stencilFaceState.passOp      = WGPUStencilOperation_Keep;

    // Depth/Stencil state.
    WGPUDepthStencilState depthStencilState {};
    depthStencilState.format              = WGPUTextureFormat_Depth32Float;
    depthStencilState.depthWriteEnabled   = true;
    depthStencilState.depthCompare        = WGPUCompareFunction_Less;
    depthStencilState.stencilFront        = stencilFaceState;
    depthStencilState.stencilBack         = stencilFaceState;
    depthStencilState.stencilReadMask     = ~0u;
    depthStencilState.stencilWriteMask    = ~0u;
    depthStencilState.depthBias           = 0;
    depthStencilState.depthBiasSlopeScale = 0.0f;
    depthStencilState.depthBiasClamp      = 0.0f;

    // Multisampling
    WGPUMultisampleState multisampleState {};
    multisampleState.count                  = 1;
    multisampleState.mask                   = ~0u;
    multisampleState.alphaToCoverageEnabled = false;

    // Setup the pipeline state.
    WGPURenderPipelineDescriptor pipelineDescriptor {};
    pipelineDescriptor.layout       = pipelineLayout;
    pipelineDescriptor.vertex       = vertexState;
    pipelineDescriptor.primitive    = primitiveState;
    pipelineDescriptor.depthStencil = &depthStencilState;
    pipelineDescriptor.multisample  = multisampleState;
    pipelineDescriptor.fragment     = &fragmentState;
    pipeline                        = wgpuDeviceCreateRenderPipeline( device, &pipelineDescriptor );

    wgpuShaderModuleRelease( shaderModule );
    wgpuPipelineLayoutRelease( pipelineLayout );
}

TextureUnlitPipelineState::~TextureUnlitPipelineState()
{
    if ( bindGroupLayout )
        wgpuBindGroupLayoutRelease( bindGroupLayout );
}

void TextureUnlitPipelineState::bind( GraphicsCommandBuffer& commandBuffer )
{
    auto passEncoder = commandBuffer.getWGPUPassEncoder();
    wgpuRenderPassEncoderSetPipeline( passEncoder, pipeline );
}