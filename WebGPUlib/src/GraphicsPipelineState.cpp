#include <WebGPUlib/GraphicsPipelineState.hpp>

using namespace WebGPUlib;

GraphicsPipelineState::~GraphicsPipelineState()
{
    if (pipeline)
        wgpuRenderPipelineRelease(pipeline);
}