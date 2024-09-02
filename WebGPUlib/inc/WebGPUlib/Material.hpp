#pragma once

#include <glm/vec4.hpp>

#include <memory>
#include <unordered_map>

namespace WebGPUlib
{
// clang-format off

// The material properties need to stored in aligned memory when uploading
// to a uniform buffer on the GPU. To ensure proper alignment, the material
// properties are allocated in 16-byte aligned memory.
struct alignas( 16 ) MaterialProperties
{
    MaterialProperties( 
        const glm::vec4& diffuse = { 1, 1, 1, 1 }, 
        const glm::vec4& specular = { 1, 1, 1, 1 },
        float specularPower = 128.0f, 
        const glm::vec4& ambient = { 0, 0, 0, 1 },
        const glm::vec4& emissive = { 0, 0, 0, 1 }, 
        const glm::vec4& reflectance = { 0, 0, 0, 1 },
        float opacity = 1.0f,
        float indexOfRefraction = 0.0f,
        float bumpIntensity = 1.0f
    )
    : diffuse( diffuse )
    , specular( specular )
    , emissive( emissive )
    , ambient( ambient )
    , reflectance( reflectance )
    , opacity( opacity )
    , specularPower( specularPower )
    , indexOfRefraction( indexOfRefraction )
    , bumpIntensity( bumpIntensity )
    , hasAmbientTexture( false )
    , hasDiffuseTexture( false )
    , hasEmissiveTexture( false )
    , hasSpecularTexture( false )
    , hasSpecularPowerTexture( false )
    , hasNormalTexture( false )
    , hasBumpTexture( false )
    , hasOpacityTexture( false )
    {}

    MaterialProperties(const MaterialProperties&) = default;
    MaterialProperties(MaterialProperties&&) noexcept = default;
    ~MaterialProperties() = default;

    MaterialProperties& operator=(const MaterialProperties&) = default;
    MaterialProperties& operator=(MaterialProperties&&) noexcept = default;

    glm::vec4 diffuse;
    //------------------------------------ ( 16 bytes )
    glm::vec4 specular;
    //------------------------------------ ( 16 bytes )
    glm::vec4 emissive;
    //------------------------------------ ( 16 bytes )
    glm::vec4 ambient;
    //------------------------------------ ( 16 bytes )
    glm::vec4 reflectance;
    //------------------------------------ ( 16 bytes )
    float     opacity;            // If opacity is < 1, the material is transparent.
    float     specularPower;
    float     indexOfRefraction;  // For transparent materials, IOR > 0.
    float     bumpIntensity;      // Used for scaling bump maps.
    //------------------------------------ ( 16 bytes )
    uint32_t  hasAmbientTexture;
    uint32_t  hasDiffuseTexture;
    uint32_t  hasEmissiveTexture;
    uint32_t  hasSpecularTexture;
    //------------------------------------ ( 16 bytes )
    uint32_t  hasSpecularPowerTexture;
    uint32_t  hasNormalTexture;
    uint32_t  hasBumpTexture;
    uint32_t  hasOpacityTexture;
    //------------------------------------ ( 16 bytes )
    // Total:                              ( 16 * 8 = 128 bytes )
};
// clang-format on

// The slots that a texture can be bound to the material.
enum class TextureSlot
{
    Ambient,
    Diffuse,
    Emissive,
    Specular,
    SpecularPower,
    Normal,
    Bump,
    Opacity,
    NumTextureSlots
};

class Texture;

class Material
{
public:
    Material( const MaterialProperties& properties = {} );
    ~Material() = default;

    const glm::vec4& getDiffuse() const noexcept;
    void             setDiffuse( const glm::vec4& diffuse ) noexcept;

    const glm::vec4& getSpecular() const noexcept;
    void             setSpecular( const glm::vec4& specular ) noexcept;

    const glm::vec4& getEmissive() const noexcept;
    void             setEmissive( const glm::vec4& emissive ) noexcept;

    const glm::vec4& getAmbient() const noexcept;
    void             setAmbient( const glm::vec4& ambient ) noexcept;

    const glm::vec4& getReflectance() const noexcept;
    void             setReflectance( const glm::vec4& reflectance ) noexcept;

    float getOpacity() const noexcept;
    void  setOpacity( float opacity ) noexcept;

    float getSpecularPower() const noexcept;
    void  setSpecularPower( float specularPower ) noexcept;

    float getIndexOfRefraction() const noexcept;
    void  setIndexOfRefraction( float indexOfRefraction ) noexcept;

    float getBumpIntensity() const noexcept;
    void  setBumpIntensity( float bumpIntensity ) noexcept;

    std::shared_ptr<Texture> getTexture( TextureSlot slot ) const;
    void                     setTexture( TextureSlot slot, std::shared_ptr<Texture> texture );

    // The material is transparent if the opacity is < 1 or there is an
    // opacity texture.
    bool isTransparent() const noexcept;

    const MaterialProperties& getProperties() const noexcept;
    void                      setProperties( const MaterialProperties& properties ) noexcept;

private:
    std::unique_ptr<MaterialProperties>                       properties;
    std::unordered_map<TextureSlot, std::shared_ptr<Texture>> textures;
};
}  // namespace WebGPUlib