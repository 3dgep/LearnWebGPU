#include <WebGPUlib/Scene.hpp>

using namespace WebGPUlib;

std::shared_ptr<SceneNode> Scene::getRootNode() const
{
    return rootNode;
}

void Scene::setRootNode( std::shared_ptr<SceneNode> root )
{
    rootNode = std::move( root );
}