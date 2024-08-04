#include <WebGPUlib/Sampler.hpp>

using namespace WebGPUlib;

Sampler::Sampler( WGPUSampler&& sampler, const WGPUSamplerDescriptor& descriptor )  // NOLINT(cppcoreguidelines-rvalue-reference-param-not-moved)
: sampler { sampler }
, samplerDescriptor { descriptor }
{}

Sampler::~Sampler()
{
    if ( sampler )
        wgpuSamplerRelease( sampler );
}


