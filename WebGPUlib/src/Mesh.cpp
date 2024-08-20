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
    // Ensure the capacity of the vertex buffers vector.
    if ( vertexBuffers.size() <= slot )
        vertexBuffers.resize( slot + 1, nullptr );

    vertexBuffers[slot] = std::move( vertexBuffer );
}

std::shared_ptr<VertexBuffer> Mesh::getVertexBuffer( uint32_t slot ) const
{
    vertexBuffers.size() > slot ? vertexBuffers[slot] : nullptr;
}

const std::vector<std::shared_ptr<VertexBuffer>>& WebGPUlib::Mesh::getVertexBuffers() const
{
    return vertexBuffers;
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