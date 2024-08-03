#include <WebGPUlib/UniformBuffer.hpp>

#include <algorithm>

using namespace WebGPUlib;

UniformBuffer::UniformBuffer( WGPUBuffer&& buffer, std::size_t size )
: Buffer { std::move( buffer ) }  // NOLINT(performance-move-const-arg)
, size { size }
{}

