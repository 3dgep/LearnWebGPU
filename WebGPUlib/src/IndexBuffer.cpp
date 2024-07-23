#include <WebGPUlib/IndexBuffer.hpp>

#include <utility>

using namespace WebGPUlib;

IndexBuffer::IndexBuffer(WGPUBuffer&& _buffer, std::size_t _indexCount)  // NOLINT(cppcoreguidelines-rvalue-reference-param-not-moved)
    : Buffer( std::move( _buffer) )  // NOLINT(performance-move-const-arg)
    , indexCount(_indexCount)
{}

