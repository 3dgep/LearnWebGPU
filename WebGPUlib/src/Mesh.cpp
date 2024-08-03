#include <WebGPUlib/Mesh.hpp>
#include <utility>

using namespace WebGPUlib;

Mesh::Mesh( std::shared_ptr<VertexBuffer> vertexBuffer, std::shared_ptr<IndexBuffer> indexBuffer,
            std::shared_ptr<Material> material )
: indexBuffer { std::move( indexBuffer ) }
, material { std::move( material ) }
{
    vertexBuffers[0] = std::move( vertexBuffer );
}

void Mesh::setVertexBuffer( uint32_t slot, std::shared_ptr<VertexBuffer> vertexBuffer )
{
    vertexBuffers[slot] = std::move( vertexBuffer );
}

std::shared_ptr<VertexBuffer> Mesh::getVertexBuffer( uint32_t slot ) const
{
    auto it = vertexBuffers.find( slot );
    return it != vertexBuffers.end() ? it->second : nullptr;
}

void Mesh::setIndexBuffer( std::shared_ptr<IndexBuffer> _indexBuffer )
{
    indexBuffer = std::move( _indexBuffer );
}

std::shared_ptr<IndexBuffer> Mesh::getIndexBuffer() const
{
    return indexBuffer;
}

void Mesh::setMaterial( std::shared_ptr<Material> _material )
{
    material = std::move( _material );
}

std::shared_ptr<Material> Mesh::getMaterial() const
{
    return material;
}