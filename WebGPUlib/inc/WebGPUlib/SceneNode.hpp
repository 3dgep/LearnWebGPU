#pragma once

#include <glm/mat4x4.hpp>

#include <memory>
#include <string>
#include <vector>

namespace WebGPUlib
{

class Mesh;

class SceneNode : public std::enable_shared_from_this<SceneNode>
{
public:
    explicit SceneNode( const glm::mat4& localTransform = glm::mat4 { 1 } );

    void setName( const std::string& name );
    const std::string& getName() const;

    void setLocalTransform( const glm::mat4& localTransform );
    const glm::mat4& getLocalTransform() const;

    const glm::mat4& getInverseLocalTransform() const;

    glm::mat4 getWorldTransform() const;

    glm::mat4 getInverseWorldTransform() const;

    void addChild( std::shared_ptr<SceneNode> child );
    void removeChild( std::shared_ptr<SceneNode> child );
    const std::vector<std::shared_ptr<SceneNode>>& getChildren() const;

    void setParent( std::shared_ptr<SceneNode> parent );
    std::shared_ptr<SceneNode> getParent() const;

    void addMesh( std::shared_ptr<Mesh> mesh );
    const std::vector<std::shared_ptr<Mesh>>& getMeshes() const;

protected:
    glm::mat4 getParentWorldTransform() const;

private:
    std::string name;

    // Local transformation of the node (relative to its parent)
    glm::mat4 localTransform;
    glm::mat4 inverseTransform;

    std::weak_ptr<SceneNode>                parent;
    std::vector<std::shared_ptr<SceneNode>> children;
    std::vector<std::shared_ptr<Mesh>>      meshes;
};
}  // namespace WebGPUlib