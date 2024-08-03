#pragma once

#include <webgpu/webgpu.h>

#include <glm/vec3.hpp>

namespace WebGPUlib
{
struct VertexPositionNormalTexture
{
    VertexPositionNormalTexture() = default;
    VertexPositionNormalTexture( const glm::vec3& position, const glm::vec3& normal, const glm::vec3& texCoord )
    : position( position )
    , normal( normal )
    , texCoord( texCoord )
    {}

    glm::vec3 position;
    glm::vec3 normal;
    glm::vec3 texCoord;

    static WGPUVertexAttribute attributes[3];
};

struct VertexPositionNormalTangentBitangentTexture
{
    VertexPositionNormalTangentBitangentTexture() = default;
    VertexPositionNormalTangentBitangentTexture( const glm::vec3& position, const glm::vec3& normal,
                                                 const glm::vec3& texCoord, const glm::vec3& tangent = glm::vec3 { 0 },
                                                 const glm::vec3& bitangent = glm::vec3 { 0 } )

    : position( position )
    , normal( normal )
    , tangent( tangent )
    , bitangent( bitangent )
    , texCoord( texCoord )
    {}

    glm::vec3 position;
    glm::vec3 normal;
    glm::vec3 tangent;
    glm::vec3 bitangent;
    glm::vec3 texCoord;

    static WGPUVertexAttribute attributes[5];
};
}  // namespace WebGPUlib