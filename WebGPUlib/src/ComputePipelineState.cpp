#include <WebGPUlib/ComputePipelineState.hpp>

using namespace WebGPUlib;

ComputePipelineState::~ComputePipelineState()
{
    if ( pipeline )
        wgpuComputePipelineRelease( pipeline );
}