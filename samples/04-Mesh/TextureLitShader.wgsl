R"(
struct VertexIn
{
    @location(0) position : vec3f,
    @location(1) normal   : vec3f,
    @location(2) uv       : vec3f,
};

struct VertexOut
{
    @builtin(position) position: vec4f,
    @location(0) uv: vec2f,
};

struct FragmentIn
{
    @location(0) uv: vec2f
};

struct Material
{
    diffuse : vec4f,
    //------------------------------------ ( 16 bytes )
    specular : vec4f,
    //------------------------------------ ( 16 bytes )
    emissive : vec4f,
    //------------------------------------ ( 16 bytes )
    ambient : vec4f,
    //------------------------------------ ( 16 bytes )
    reflectance : vec4f,
    //------------------------------------ ( 16 bytes )
    opacity : f32,
    specularPower : f32,
    indexOfRefraction : f32,
    bumpIntensity : f32,
    //------------------------------------ ( 16 bytes )
    hasAmbientTexture : bool,
    hasDiffuseTexture : bool,
    hasEmissiveTexture : bool,
    hasSpecularTexture : bool,
    //------------------------------------ ( 16 bytes )
    hasSpecularPowerTexture : bool,
    hasNormalTexture : bool,
    hasBumpTexture : bool,
    hasOpacityTexture : bool,
    //------------------------------------ ( 16 bytes )
    // Total:                              ( 16 * 8 = 128 bytes )
};

struct PointLight
{
    positionWS : vec4f,
    //----------------------------------- (16 byte boundary)
    positionVS : vec4f,
    //----------------------------------- (16 byte boundary)
    color : vec4f,
    //----------------------------------- (16 byte boundary)
    ambient : float,
    constantAttenuation : float,
    linearAttenuation : float,
    quadraticAttenuation : float,
    //----------------------------------- (16 byte boundary)
    // Total:                              16 * 4 = 64 bytes
};

struct SpotLight
{
    positionWS : vec4f,
    //----------------------------------- (16 byte boundary)
    positionVS : vec4f,
    //----------------------------------- (16 byte boundary)
    directionWS : vec4f,
    //----------------------------------- (16 byte boundary)
    directionVS : vec4f,
    //----------------------------------- (16 byte boundary)
    color : vec4f,
    //----------------------------------- (16 byte boundary)
    ambient : float,
    spotAngle : float,
    constantAttenuation : float,
    linearAttenuation : float,
    //----------------------------------- (16 byte boundary)
    quadraticAttenuation : float,
    padding : vec3f,
    //----------------------------------- (16 byte boundary)
    // Total:                              16 * 7 = 112 bytes
};

struct LightResult
{
    diffuse : vec4f,
    specular : vec4f,
    ambient : vec4f,
};

// Model View Projection matrix.
@group(0) @binding(0) var<uniform> mvp : mat4x4f;
// Material properties.
@group(0) @binding(1) var<uniform> material : Material;

// Textures
@group(0) @binding(2) var ambientTexture : texture_2d<f32>;
@group(0) @binding(3) var emissiveTexture : texture_2d<f32>;
@group(0) @binding(4) var diffuseTexture : texture_2d<f32>;
@group(0) @binding(5) var specularTexture : texture_2d<f32>;
@group(0) @binding(6) var specularPowerTexture : texture_2d<f32>;
@group(0) @bidning(7) var normalTexture : texture_2d<f32>;
@group(0) @binding(8) var bumpTexture : texture_2d<f32>;
@group(0) @binding(9) var opacityTexture : texture_2d<f32>;

// Sampler.
@group(0) @binding(10) var linearRepeatSampler : sampler;

// Lights
@group(0) @binding(11) var<storage> pointLights : array<PointLight>;
@group(0) @binding(12) var<storage> spotLights : array<SpotLight>;

@vertex
fn vs_main(in: VertexIn) -> VertexOut
{
    var out: VertexOut;
    out.position =  mvp * vec4f(in.position, 1.0);
    out.uv = in.uv.xy;
    return out;
}

fn DoDiffuse( N : vec3f, L : vec3f ) -> f32
{
    return max( 0, dot(N, L) );
}

fn DoSpecular( V : vec3f, N : vec3f, L : vec3f, specularPower : f32 ) -> f32
{
    let R = normalize( reflect(-L, N) );
    let RdotV = max(0, dot( R, V ) );

    return pow( RdotV, specularPower );
}

fn DoAttenuation( c : f32, l : f32, q : f32, d : f32 ) -> f32
{
    return 1.0f / ( c + l * d + q * d * d );
}

fn DoSpotCone( spotDir : vec3f, L : vec3f, spotAngle : f32 ) -> f32
{
    let minCos = cos( spotAngle );
    let maxCos = (minCos + 1.0) / 2.0;
    let cosAngle = dot( spotDir, -L);

    return smoothstep( minCos, maxCos, cosAngle );
}

fn DoPointLight( light : PointLight, V : vec3f, P : vec3f, N : vec3f, specularPower : f32 ) -> LightResult
{
    var result : LightResult;
    let L = light.positionVS.xyz - P;
    let d = length( L );
    L = L / d;

    let attenuation = DoAttenuation( light.constantAttenuation,
                                     light.linearAttenuation,
                                     light.quadraticAttenuation,
                                     d );

    result.diffuse = DoDiffuse( N, L ) * attenuation * light.color;
    result.specular = DoSpecular( V, N, L, specularPower ) * attenuation * light.color;
    result.ambient = light.color * light.ambient;

    return result;
}

fn DoSpotLight( light : SpotLight, V : vec3f, P : vec3f, N : vec3f, specularPower : f32 ) -> LightResult
{
    var result : LightResult;
    let L = light.positionVS.xyz - P;
    let d = length( L );
    L = L / d;

    let attenuation = DoAttenuation( light.constantAttenuation,
                                     light.linearAttenuation,
                                     light.quadraticAttenuation,
                                     d );

    let spotIntensity = DoSpotCone( light.directionVS.xyz, L, light.spotAngle );

    result.diffuse = DoDiffuse( N, L ) * attenuation * spotIntensity * light.color;
    result.specular = DoSpecular( V, N, L, specularPower ) * attenuation * spotIntensity * light.color;
    result.ambient = light.color * light.ambient;

    return result;
}

fn DoLighting( P : vec3f, N : vec3f, specularPower : f32 ) -> LightResult
{
    // Lighting is computed in view space.
    let V = normalize( -P );

    var totalResult : LightResult; // is this 0 initialized?

    // Iterate point lights
    for( i : u32 = 0; i < arrayLength(pointLights); ++i )
    {
        let result = DoPointLight( pointLights[i], V, P, N, specularPower );

        totalResult.diffuse += result.diffuse;
        totalResult.specular += result.specular;
        totalResult.ambient += result.ambient;
    }

    // Iterate spot lights
    for( i : u32 = 0; i < arrayLength(spotLights); ++i )
    {
        let result = DoSpotLight( spotLights[i], V, P, N, specularPower );

        totalResult.diffuse += result.diffuse;
        totalResult.specular += result.specular;
        totalResult.ambient += result.ambient;
    }

    totalResult.diffuse = saturate(totalResult.diffuse);
    totalResult.specular = saturate(totalResult.specular);
    totalResult.ambient = saturate(totalResult.ambient);

    return totalResult;
}

fn ExpandNormal( n : vec3f ) -> vec3f
{
    return n * 2.0 - 1.0;
}

fn DoNormalMapping( TBN : mat3x3f, tex : texture_2d<f32>, uv : vec2f ) -> vec3f
{
    var N = textureSample( tex, linearRepeatSampler, uv ).xyz;
    N = ExpandNormal( N );

    // Transfrom normal from texture space to view space.
    N = N * TBN;

    return normalize(N);
}

@fragment
fn fs_main(in: FragmentIn) -> @location(0) vec4f {
    return textureSample(albedoTexture, linearRepeatSampler, in.uv);
}
)"