#include <WebGPUlib/Material.hpp>

using namespace WebGPUlib;

Material::Material( const MaterialProperties& properties )
: properties { std::make_unique<MaterialProperties>( properties ) } // Is this aligned?
{}

const glm::vec4& Material::getDiffuse() const noexcept
{
    return properties->diffuse;
}

void Material::setDiffuse( const glm::vec4& diffuse ) noexcept
{
    properties->diffuse = diffuse;
}

const glm::vec4& Material::getSpecular() const noexcept
{
    return properties->specular;
}

void Material::setSpecular( const glm::vec4& specular ) noexcept
{
    properties->specular = specular;
}

const glm::vec4& Material::getEmissive() const noexcept
{
    return properties->emissive;
}

void Material::setEmissive( const glm::vec4& emissive ) noexcept
{
    properties->emissive = emissive;
}

const glm::vec4& Material::getAmbient() const noexcept
{
    return properties->ambient;
}

void Material::setAmbient( const glm::vec4& ambient ) noexcept
{
    properties->ambient = ambient;
}

const glm::vec4& Material::getReflectance() const noexcept
{
    return properties->reflectance;
}

void Material::setReflectance( const glm::vec4& reflectance ) noexcept
{
    properties->reflectance = reflectance;
}

float Material::getOpacity() const noexcept
{
    return properties->opacity;
}

void Material::setOpacity( float opacity ) noexcept
{
    properties->opacity = opacity;
}

float Material::getSpecularPower() const noexcept
{
    return properties->specularPower;
}

void Material::setSpecularPower( float specularPower ) noexcept
{
    properties->specularPower = specularPower;
}

