#pragma once

#include <glm/mat4x4.hpp>

struct Matrices
{
    glm::mat4 model;
    glm::mat4 modelView;
    glm::mat4 modelViewIT;
    glm::mat4 modelViewProjection;
};