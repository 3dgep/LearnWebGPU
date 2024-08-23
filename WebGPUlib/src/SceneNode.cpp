#include <WebGPUlib/SceneNode.hpp>

using namespace WebGPUlib;

SceneNode::SceneNode( const glm::mat4& localTransform )
: localTransform { localTransform }
{
    inverseTransform = glm::inverse( localTransform );
}

void SceneNode::setName( const std::string& _name )
{
    name = _name;
}

const std::string& SceneNode::getName() const
{
    return name;
}

void SceneNode::setLocalTransform( const glm::mat4& _localTransform )
{
    localTransform   = _localTransform;
    inverseTransform = glm::inverse( localTransform );
}

const glm::mat4& SceneNode::getLocalTransform() const
{
    return localTransform;
}

const glm::mat4& SceneNode::getInverseLocalTransform() const
{
    return inverseTransform;
}

glm::mat4 SceneNode::getWorldTransform() const
{
    return getParentWorldTransform() * localTransform;
}

glm::mat4 SceneNode::getInverseWorldTransform() const
{
    return glm::inverse( getWorldTransform() );
}

void SceneNode::addChild( std::shared_ptr<SceneNode> child )
{
    if (child)
    {
        auto iter = std::find( children.begin(), children.end(), child );
        if (iter == children.end())
        {
            child->parent = shared_from_this();
            glm::mat4 worldTransform = child->getWorldTransform();
            glm::mat4 _localTransform = getInverseWorldTransform() * worldTransform;
            child->setLocalTransform( _localTransform );
            children.push_back( child );
        }
    }
}

void SceneNode::removeChild( std::shared_ptr<SceneNode> child )
{
    if (child)
    {
        auto iter = std::find( children.begin(), children.end(), child );
        if (iter != children.end())
        {
            child->setParent( nullptr );
            children.erase( iter );
        }
        else
        {
            // Not a direct child of this node. Search lower in the scene hierarchy.
            for (auto& c : children )
            {
                c->removeChild( child );
            }
        }
    }
}

const std::vector<std::shared_ptr<SceneNode>>& SceneNode::getChildren() const
{
    return children;
}

void SceneNode::setParent( std::shared_ptr<SceneNode> _parent )
{
    // Since parent's own their children, removing this node from its
    // current parent will cause this node to get deleted.
    // To ensure this doesn't happen, we need to store a shared pointer to myself
    // until the new parent takes ownership.
    auto me = shared_from_this();
    if (_parent)
    {
        _parent->addChild( me );
    }
    else if (auto currentParent = parent.lock())
    {
        // Remove this node from its current parent.
        auto worldTransform = getWorldTransform();
        currentParent->removeChild( me );
        setLocalTransform( worldTransform );
        parent.reset();
    }
}

std::shared_ptr<SceneNode> SceneNode::getParent() const
{
    return parent.lock();
}

void SceneNode::addMesh( std::shared_ptr<Mesh> mesh )
{
    // Avoid adding the same mesh multiple times.
    auto iter = std::find( meshes.begin(), meshes.end(), mesh );
    if (iter == meshes.end())
    {
        meshes.push_back( std::move(mesh) );
    }
}

const std::vector<std::shared_ptr<Mesh>>& SceneNode::getMeshes() const
{
    return meshes;
}

glm::mat4 SceneNode::getParentWorldTransform() const
{
    glm::mat4 parentTransform { 1 };
    if ( auto parentNode = parent.lock() )
    {
        parentTransform = parentNode->getWorldTransform();
    }

    return parentTransform;
}