#include <WebGPUlib/IndexBuffer.hpp>

#include <utility>

using namespace WebGPUlib;

IndexBuffer::IndexBuffer( WGPUBuffer&& _buffer, std::size_t _indexCount, std::size_t _indexStride )
: Buffer( std::move( _buffer ) )  // NOLINT(performance-move-const-arg)
, indexCount( _indexCount )
, indexStride( _indexStride )
{}