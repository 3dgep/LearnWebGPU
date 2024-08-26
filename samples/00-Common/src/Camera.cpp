#include <Camera.hpp>

void Camera::setLookAt( const glm::vec3& eye, const glm::vec3& target, const glm::vec3& up )
{
    viewMatrix = glm::lookAtRH( eye, target, up );

    position = eye;
    rotation = glm::quat_cast( glm::transpose( viewMatrix ) );

    isViewDirty = false;
}

void Camera::setProjection( float _fov, float _aspect, float _near, float _far )
{
    fov         = _fov;
    aspectRatio = _aspect;
    near        = _near;
    far         = _far;

    isProjectionDirty = true;
}

void Camera::setPosition( const glm::vec3& _position )
{
    position = _position;
    isViewDirty = true;
}

const glm::vec3& Camera::getPosition() const
{
    return position;
}

void Camera::setRotation( const glm::quat& _rotation )
{
    rotation = _rotation;
    isViewDirty = true;
}

const glm::quat& Camera::getRotation() const
{
    return rotation;
}

const glm::mat4& Camera::getViewMatrix() const
{
    if ( isViewDirty )
        updateViewMatrix();

    return viewMatrix;
}

const glm::mat4& Camera::getProjectionMatrix() const
{
    if ( isProjectionDirty )
        updateProjectionMatrix();

    return projectionMatrix;
}

void Camera::updateViewMatrix() const
{
    auto rotationMatrix    = glm::mat4_cast( glm::inverse( rotation ) );
    auto translationMatrix = glm::translate( glm::mat4 { 1 }, -position );
    viewMatrix             = rotationMatrix * translationMatrix;

    isViewDirty = false;
}

void Camera::updateProjectionMatrix() const
{
    projectionMatrix  = glm::perspectiveRH_ZO( fov, aspectRatio, near, far );
    isProjectionDirty = false;
}