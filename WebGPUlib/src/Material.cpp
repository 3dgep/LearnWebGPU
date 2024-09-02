#include <WebGPUlib/Material.hpp>
#include <utility>

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

float Material::getIndexOfRefraction() const noexcept
{
    return properties->indexOfRefraction;
}

void Material::setIndexOfRefraction( float indexOfRefraction ) noexcept
{
    properties->indexOfRefraction = indexOfRefraction;
}

float Material::getBumpIntensity() const noexcept
{
    return properties->bumpIntensity;
}

void Material::setBumpIntensity( float bumpIntensity ) noexcept
{
    properties->bumpIntensity = bumpIntensity;
}

std::shared_ptr<Texture> Material::getTexture( TextureSlot slot ) const
{
    auto iter = textures.find( slot );
    if (iter != textures.end())
        return iter->second;

    return nullptr;
}

void Material::setTexture( TextureSlot slot, std::shared_ptr<Texture> texture )
{
    textures[slot] = std::move( texture );

    switch ( slot )
    {
    case TextureSlot::Ambient:
        properties->hasAmbientTexture = texture != nullptr;
        break;
    case TextureSlot::Diffuse:
        properties->hasDiffuseTexture = texture != nullptr;
        break;
    case TextureSlot::Emissive:
        properties->hasEmissiveTexture = texture != nullptr;
        break;
    case TextureSlot::Specular:
        properties->hasSpecularTexture = texture != nullptr;
        break;
    case TextureSlot::SpecularPower:
        properties->hasSpecularPowerTexture = texture != nullptr;
        break;
    case TextureSlot::Normal:
        properties->hasNormalTexture = texture != nullptr;
        break;
    case TextureSlot::Bump:
        properties->hasBumpTexture = texture != nullptr;
        break;
    case TextureSlot::Opacity:
        properties->hasOpacityTexture = texture != nullptr;
        break;
    case TextureSlot::NumTextureSlots:
        break;
    }
}

bool Material::isTransparent() const noexcept
{
    return properties->opacity < 1.0f || textures.find( TextureSlot::Opacity ) != textures.end();
}

const MaterialProperties& Material::getProperties() const noexcept
{
    return *properties;
}

void Material::setProperties( const MaterialProperties& _properties ) noexcept
{
    *properties = _properties;
}

