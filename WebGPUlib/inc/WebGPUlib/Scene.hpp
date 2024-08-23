#pragma once
#include <memory>

namespace WebGPUlib
{

class SceneNode;

class Scene
{
public:
    Scene( std::shared_ptr<SceneNode> rootNode = nullptr)
    : rootNode { std::move( rootNode ) }
    {}

    std::shared_ptr<SceneNode> getRootNode() const;
    void                       setRootNode( std::shared_ptr<SceneNode> root );

protected:
private:
    std::shared_ptr<SceneNode> rootNode;
};
}  // namespace WebGPUlib