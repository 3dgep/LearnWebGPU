#pragma once

#include <glm/gtc/quaternion.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

class Camera
{
public:
    enum class Space
    {
        Local,
        World,
    };

    Camera() = default;

    void setLookAt( const glm::vec3& eye, const glm::vec3& target, const glm::vec3& up );

    void setProjection( float fov, float aspect, float near, float far );

    void setPosition( const glm::vec3& position );
    const glm::vec3& getPosition() const;

    void setRotation( const glm::quat& rotation );
    const glm::quat& getRotation() const;

    const glm::mat4& getViewMatrix() const;

    const glm::mat4& getProjectionMatrix() const;

private:
    void updateViewMatrix() const;
    void updateProjectionMatrix() const;

    // Camera position in world space.
    glm::vec3 position{0};
    // Camera rotation quaternion.
    glm::quat rotation;

    // Vertical field of view.
    float fov;
    // Aspect ratio of the viewport.
    float aspectRatio;
    // Near clipping plane.
    float near;
    // Far clipping plane.
    float far;

    mutable bool isViewDirty = true;
    mutable bool isProjectionDirty = true;

    mutable glm::mat4 viewMatrix{1};
    mutable glm::mat4 projectionMatrix{1};
};