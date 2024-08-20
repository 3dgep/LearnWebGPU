#pragma once

#include <memory>
#include <vector>

namespace WebGPUlib
{

class VertexBuffer;
class IndexBuffer;
class Material;

class Mesh
{
public:
    Mesh() = default;
    explicit Mesh( std::shared_ptr<VertexBuffer> vertexBuffer, std::shared_ptr<IndexBuffer> indexBuffer = nullptr,
                   std::shared_ptr<Material> material = nullptr );
    Mesh( const Mesh& )     = delete;
    Mesh( Mesh&& ) noexcept = default;
    ~Mesh()                 = default;

    Mesh& operator=( const Mesh& )     = delete;
    Mesh& operator=( Mesh&& ) noexcept = default;

    void                          setVertexBuffer( uint32_t slot, std::shared_ptr<VertexBuffer> vertexBuffer );
    std::shared_ptr<VertexBuffer> getVertexBuffer( uint32_t slot ) const;
    const std::vector<std::shared_ptr<VertexBuffer>>& getVertexBuffers() const;

    void                         setIndexBuffer( std::shared_ptr<IndexBuffer> indexBuffer );
    std::shared_ptr<IndexBuffer> getIndexBuffer() const;

    void                      setMaterial( std::shared_ptr<Material> material );
    std::shared_ptr<Material> getMaterial() const;

private:
    std::vector<std::shared_ptr<VertexBuffer>> vertexBuffers;
    std::shared_ptr<IndexBuffer>               indexBuffer;
    std::shared_ptr<Material>                  material;
};
}  // namespace WebGPUlib