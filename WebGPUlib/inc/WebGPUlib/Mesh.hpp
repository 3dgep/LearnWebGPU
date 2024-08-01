#pragma once

#include <memory>
#include <unordered_map>

namespace WebGPUlib
{

class VertexBuffer;
class IndexBuffer;
class Material;

class Mesh
{
public:
protected:
private:
    std::unordered_map<uint32_t, std::shared_ptr<VertexBuffer>> vertexBuffers;
    std::shared_ptr<IndexBuffer>                                indexBuffer;
};
}  // namespace WebGPUlib