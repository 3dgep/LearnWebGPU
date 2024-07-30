#pragma once

#include "../bitmask_operators.hpp"

#include <memory>

namespace WebGPUlib
{

class Texture;

enum class AttachmentPoint
{
    Color0,
    Color1,
    Color2,
    Color3,
    Color4,
    Color5,
    Color6,
    Color7,
    Depth,
    Stencil,
    DepthStencil
};

class RenderTarget
{
public:
    
    void attachTexture( AttachmentPoint attachmentPoint, std::shared_ptr<Texture> texture );

    std::shared_ptr<Texture> getTexture(AttachmentPoint attachmentPoint) const;

    void resize(uint32_t width, uint32_t height);

protected:
private:
};
}  // namespace WebGPUlib

// Enable bitmask operators on AttachmentPoint enum.
template<>
struct enable_bitmask_operators<WebGPUlib::AttachmentPoint>
{
    static constexpr bool enable = true;
};