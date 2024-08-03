#include <WebGPUlib/Vertex.hpp>

#include <cstddef>

using namespace WebGPUlib;

WGPUVertexAttribute VertexPositionNormalTexture::attributes[3] = {
    {
        WGPUVertexFormat_Float32x3,
        offsetof(VertexPositionNormalTexture, position),
        0,
    },
    {
        WGPUVertexFormat_Float32x3,
        offsetof(VertexPositionNormalTexture, normal),
        1
    },
    {
        WGPUVertexFormat_Float32x3,
        offsetof(VertexPositionNormalTexture, texCoord),
        2
    }
};

WGPUVertexAttribute VertexPositionNormalTangentBitangentTexture::attributes[5] = {
    {
        WGPUVertexFormat_Float32x3,
        offsetof(VertexPositionNormalTangentBitangentTexture, position),
        0,
    },
    {
        WGPUVertexFormat_Float32x3,
        offsetof(VertexPositionNormalTangentBitangentTexture, normal),
        1
    },
    {
        WGPUVertexFormat_Float32x3,
        offsetof( VertexPositionNormalTangentBitangentTexture, tangent ),
        2
    },
    {
        WGPUVertexFormat_Float32x3,
        offsetof(VertexPositionNormalTangentBitangentTexture, bitangent),
        3
    },
    {
        WGPUVertexFormat_Float32x3,
        offsetof(VertexPositionNormalTangentBitangentTexture, texCoord),
        4
    }
};