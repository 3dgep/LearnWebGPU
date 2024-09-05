#include <WebGPUlib/StorageBuffer.hpp>

#include <algorithm>

using namespace WebGPUlib;

StorageBuffer::StorageBuffer( WGPUBuffer&& buffer, std::size_t _elementCount, std::size_t _elementSize )
: Buffer { std::move( buffer ) }  // NOLINT(performance-move-const-arg)
, elementCount { _elementCount }
, elementSize { _elementSize }
{}
