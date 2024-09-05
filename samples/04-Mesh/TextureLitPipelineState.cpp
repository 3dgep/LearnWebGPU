#include "TextureLitPipelineState.hpp"
#include "Light.hpp"
#include "Matrices.hpp"

#include <WebGPUlib/Device.hpp>
#include <WebGPUlib/GraphicsCommandBuffer.hpp>
#include <WebGPUlib/Material.hpp>
#include <WebGPUlib/Surface.hpp>
#include <WebGPUlib/Vertex.hpp>

#include <glm/mat4x4.hpp>

using namespace WebGPUlib;

TextureLitPipelineState::TextureLitPipelineState()
{
    const char* shaderCode = {
#include "TextureLitShader.wgsl"
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
    WGPUBindGroupLayoutEntry bindGroupLayoutEntries[13] {};

    // @group( 0 ) @binding( 0 ) var<uniform> matrices : Matrices;
    bindGroupLayoutEntries[0].binding               = 0;
    bindGroupLayoutEntries[0].visibility            = WGPUShaderStage_Vertex;
    bindGroupLayoutEntries[0].buffer.type           = WGPUBufferBindingType_Uniform;
    bindGroupLayoutEntries[0].buffer.minBindingSize = sizeof( Matrices );

    // @group( 0 ) @binding( 1 ) var<uniform> material : Material;
    bindGroupLayoutEntries[1].binding               = 1;
    bindGroupLayoutEntries[1].visibility            = WGPUShaderStage_Fragment;
    bindGroupLayoutEntries[1].buffer.type           = WGPUBufferBindingType_Uniform;
    bindGroupLayoutEntries[1].buffer.minBindingSize = sizeof( MaterialProperties );

    // @group( 0 ) @binding( 2 ) var ambientTexture : texture_2d<f32>;
    // @group( 0 ) @binding( 3 ) var emissiveTexture : texture_2d<f32>;
    // @group( 0 ) @binding( 4 ) var diffuseTexture : texture_2d<f32>;
    // @group( 0 ) @binding( 5 ) var specularTexture : texture_2d<f32>;
    // @group( 0 ) @binding( 6 ) var specularPowerTexture : texture_2d<f32>;
    // @group( 0 ) @binding( 7 ) var normalTexture : texture_2d<f32>;
    // @group( 0 ) @binding( 8 ) var bumpTexture : texture_2d<f32>;
    // @group( 0 ) @binding( 9 ) var opacityTexture : texture_2d<f32>;
    for ( int binding = 2; binding <= 9; ++binding )
    {
        bindGroupLayoutEntries[binding].binding               = binding;
        bindGroupLayoutEntries[binding].visibility            = WGPUShaderStage_Fragment;
        bindGroupLayoutEntries[binding].texture.sampleType    = WGPUTextureSampleType_Float;
        bindGroupLayoutEntries[binding].texture.viewDimension = WGPUTextureViewDimension_2D;
    }

    // @group( 0 ) @binding( 10 ) var linearRepeatSampler : sampler;
    bindGroupLayoutEntries[10].binding      = 10;
    bindGroupLayoutEntries[10].visibility   = WGPUShaderStage_Fragment;
    bindGroupLayoutEntries[10].sampler.type = WGPUSamplerBindingType_Filtering;

    // @group( 0 ) @binding( 11 ) var<storage> pointLights : array<PointLight>;
    bindGroupLayoutEntries[11].binding               = 11;
    bindGroupLayoutEntries[11].visibility            = WGPUShaderStage_Fragment;
    bindGroupLayoutEntries[11].buffer.type           = WGPUBufferBindingType_ReadOnlyStorage;
    bindGroupLayoutEntries[11].buffer.minBindingSize = sizeof(PointLight);

    // @group( 0 ) @binding( 12 ) var<storage> spotLights : array<SpotLight>;
    bindGroupLayoutEntries[12].binding               = 12;
    bindGroupLayoutEntries[12].visibility            = WGPUShaderStage_Fragment;
    bindGroupLayoutEntries[12].buffer.type           = WGPUBufferBindingType_ReadOnlyStorage;
    bindGroupLayoutEntries[12].buffer.minBindingSize = sizeof(SpotLight);

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
    // @location(0) position : vec3f,
    // @location(1) normal   : vec3f,
    // @location(2) tangent  : vec3f,
    // @location(3) bitangent: vec3f,
    // @location(4) uv       : vec3f,

    WGPUVertexAttribute vertexAttributes[5] {};
    // glm::vec3 position;
    vertexAttributes[0].format         = WGPUVertexFormat_Float32x3;
    vertexAttributes[0].offset         = offsetof( VertexPositionNormalTangentBitangentTexture, position );
    vertexAttributes[0].shaderLocation = 0;

    // glm::vec3 normal;
    vertexAttributes[1].format         = WGPUVertexFormat_Float32x3;
    vertexAttributes[1].offset         = offsetof( VertexPositionNormalTangentBitangentTexture, normal );
    vertexAttributes[1].shaderLocation = 1;

    // glm::vec3 tangent;
    vertexAttributes[2].format         = WGPUVertexFormat_Float32x3;
    vertexAttributes[2].offset         = offsetof( VertexPositionNormalTangentBitangentTexture, tangent );
    vertexAttributes[2].shaderLocation = 2;

    // glm::vec3 bitangent;
    vertexAttributes[3].format         = WGPUVertexFormat_Float32x3;
    vertexAttributes[3].offset         = offsetof( VertexPositionNormalTangentBitangentTexture, bitangent );
    vertexAttributes[3].shaderLocation = 3;

    // glm::vec3 texCoord;
    vertexAttributes[4].format         = WGPUVertexFormat_Float32x3;
    vertexAttributes[4].offset         = offsetof( VertexPositionNormalTangentBitangentTexture, texCoord );
    vertexAttributes[4].shaderLocation = 4;

    WGPUVertexBufferLayout vertexBufferLayout {};
    vertexBufferLayout.arrayStride    = sizeof( VertexPositionNormalTangentBitangentTexture );
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
    multisampleState.count                  = 4;
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

TextureLitPipelineState::~TextureLitPipelineState()
{
    if ( bindGroupLayout )
        wgpuBindGroupLayoutRelease( bindGroupLayout );
}

void TextureLitPipelineState::bind( GraphicsCommandBuffer& commandBuffer )
{
    auto passEncoder = commandBuffer.getWGPUPassEncoder();
    wgpuRenderPassEncoderSetPipeline( passEncoder, pipeline );
}