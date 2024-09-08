#pragma once

#include <glm/vec4.hpp>

struct PointLight
{
    PointLight()
    : positionWS( 0.0f, 0.0f, 0.0f, 1.0f )
    , positionVS( 0.0f, 0.0f, 0.0f, 1.0f )
    , color( 1.0f, 1.0f, 1.0f, 1.0f )
    , ambient( 0.01f )
    , constantAttenuation( 1.0f )
    , linearAttenuation( 0.0f )
    , quadraticAttenuation( 0.0f )
    {}

    glm::vec4 positionWS;  // Light position in world space.
    //----------------------------------- (16 byte boundary)
    glm::vec4 positionVS;  // Light position in view space.
    //----------------------------------- (16 byte boundary)
    glm::vec4 color;
    //----------------------------------- (16 byte boundary)
    float ambient;
    float constantAttenuation;
    float linearAttenuation;
    float quadraticAttenuation;
    //----------------------------------- (16 byte boundary)
    // Total:                              16 * 4 = 64 bytes
};

struct SpotLight
{
    SpotLight()
    : positionWS( 0.0f, 0.0f, 0.0f, 1.0f )
    , positionVS( 0.0f, 0.0f, 0.0f, 1.0f )
    , directionWS( 0.0f, 0.0f, 1.0f, 0.0f )
    , directionVS( 0.0f, 0.0f, 1.0f, 0.0f )
    , color( 1.0f, 1.0f, 1.0f, 1.0f )
    , ambient( 0.01f )
    , spotAngle( 1.5707963267948966192313216916398f ) // pi/2
    , constantAttenuation( 1.0f )
    , linearAttenuation( 0.0f )
    , quadraticAttenuation( 0.0f )
    {}

    glm::vec4 positionWS;  // Light position in world space.
    //----------------------------------- (16 byte boundary)
    glm::vec4 positionVS;  // Light position in view space.
    //----------------------------------- (16 byte boundary)
    glm::vec4 directionWS;  // Light direction in world space.
    //----------------------------------- (16 byte boundary)
    glm::vec4 directionVS;  // Light direction in view space.
    //----------------------------------- (16 byte boundary)
    glm::vec4 color;
    //----------------------------------- (16 byte boundary)
    float ambient;
    float spotAngle;
    float constantAttenuation;
    float linearAttenuation;
    //----------------------------------- (16 byte boundary)
    float quadraticAttenuation;
    float padding[3];
    //----------------------------------- (16 byte boundary)
    // Total:                              16 * 7 = 112 bytes
};
