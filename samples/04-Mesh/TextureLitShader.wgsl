R"(
struct VertexIn
{
    @location(0) position : vec3f,
    @location(1) normal   : vec3f,
    @location(2) tangent  : vec3f,
    @location(3) bitangent: vec3f,
    @location(4) uv       : vec3f,
};

struct VertexOut
{
    @location(0) positionVS : vec3f,
    @location(1) normalVS   : vec3f,
    @location(2) tangentVS  : vec3f,
    @location(3) bitangentVS: vec3f,
    @location(4) uv         : vec2f,
    @builtin(position) position : vec4f,
};

struct FragmentIn
{
    @location(0) positionVS : vec3f,
    @location(1) normalVS   : vec3f,
    @location(2) tangentVS  : vec3f,
    @location(3) bitangentVS: vec3f,
    @location(4) uv         : vec2f,
};

struct Matrices
{
    model               : mat4x4f,
    modelView           : mat4x4f,
    modelViewIT         : mat4x4f, // Inverse-transpose
    modelViewProjection : mat4x4f,
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
    hasAmbientTexture : u32,
    hasDiffuseTexture : u32,
    hasEmissiveTexture : u32,
    hasSpecularTexture : u32,
    //------------------------------------ ( 16 bytes )
    hasSpecularPowerTexture : u32,
    hasNormalTexture : u32,
    hasBumpTexture : u32,
    hasOpacityTexture : u32,
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
    ambient : f32,
    constantAttenuation : f32,
    linearAttenuation : f32,
    quadraticAttenuation : f32,
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
    ambient : f32,
    spotAngle : f32,
    constantAttenuation : f32,
    linearAttenuation : f32,
    //----------------------------------- (16 byte boundary)
    quadraticAttenuation : f32,
    //----------------------------------- (16 byte boundary)
    // Total:                              16 * 7 = 112 bytes
};

struct LightResult
{
    diffuse : vec4f,
    specular : vec4f,
    ambient : vec4f,
};

// Constants
@group(0) @binding(0) var<uniform> matrices : Matrices;
@group(0) @binding(1) var<uniform> material : Material;

// Textures
@group(0) @binding(2) var ambientTexture : texture_2d<f32>;
@group(0) @binding(3) var emissiveTexture : texture_2d<f32>;
@group(0) @binding(4) var diffuseTexture : texture_2d<f32>;
@group(0) @binding(5) var specularTexture : texture_2d<f32>;
@group(0) @binding(6) var specularPowerTexture : texture_2d<f32>;
@group(0) @binding(7) var normalTexture : texture_2d<f32>;
@group(0) @binding(8) var bumpTexture : texture_2d<f32>;
@group(0) @binding(9) var opacityTexture : texture_2d<f32>;

// Sampler.
@group(0) @binding(10) var linearRepeatSampler : sampler;

// Lights
@group(0) @binding(11) var<storage> pointLights : array<PointLight>;
// @group(0) @binding(12) var<storage> spotLights : array<SpotLight>;

fn toMat3x3( m : mat4x4f ) -> mat3x3f
{
    return mat3x3( m[0].xyz, m[1].xyz, m[2].xyz );
}

@vertex
fn vs_main(in: VertexIn) -> VertexOut
{
    var out: VertexOut;
    
    out.positionVS =  (matrices.modelView * vec4f(in.position, 1.0)).xyz;
    out.normalVS = toMat3x3(matrices.modelViewIT) * in.normal;
    out.tangentVS = toMat3x3(matrices.modelViewIT) * in.tangent;
    out.bitangentVS = toMat3x3(matrices.modelViewIT) * in.bitangent;
    out.uv = in.uv.xy;
    out.position = matrices.modelViewProjection * vec4f(in.position, 1.0);

    return out;
}

fn DoDiffuse( N : vec3f, L : vec3f ) -> f32
{
    return max( 0.0, dot(N, L) );
}

fn DoSpecular( V : vec3f, N : vec3f, L : vec3f, specularPower : f32 ) -> f32
{
    let R = normalize( reflect(-L, N) );
    let RdotV = max(0.0, dot( R, V ) );

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
    var L = light.positionVS.xyz - P;
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
    var L = light.positionVS.xyz - P;
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
    for( var i : u32 = 0; i < arrayLength(&pointLights); i++ )
    {
        let result = DoPointLight( pointLights[i], V, P, N, specularPower );

        totalResult.diffuse += result.diffuse;
        totalResult.specular += result.specular;
        totalResult.ambient += result.ambient;
    }

    // Iterate spot lights
    /*
    for( var i : u32 = 0; i < arrayLength(&spotLights); i++ )
    {
        let result = DoSpotLight( spotLights[i], V, P, N, specularPower );

        totalResult.diffuse += result.diffuse;
        totalResult.specular += result.specular;
        totalResult.ambient += result.ambient;
    }
    */

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
    N = TBN * N;

    return normalize(N);
}

fn DoBumpMapping( TBN : mat3x3f, tex : texture_2d<f32>, uv : vec2f, bumpScale : f32 ) -> vec3f
{
    let height_00 = textureSample( tex, linearRepeatSampler, uv ).r * bumpScale;
    let height_10 = textureSample( tex, linearRepeatSampler, uv, vec2i(1, 0) ).r * bumpScale;
    let height_01 = textureSample( tex, linearRepeatSampler, uv, vec2i(0, 1) ).r * bumpScale;

    let p_00 = vec3f( 0, 0, height_00 );
    let p_10 = vec3f( 0, 0, height_10 );
    let p_01 = vec3f( 0, 0, height_01 );

    let tangent = normalize( p_10 - p_00 );
    let bitangent = normalize( p_01 - p_00 );
    let normal = cross( tangent, bitangent );

    let N = TBN * normal;

    return N;
}

@fragment
fn fs_main(in: FragmentIn) -> @location(0) vec4f {
    
    // Use the alpha component of the diffuse color for opacity.
    var opacity = material.diffuse.a;
    if (material.hasOpacityTexture != 0)
    {
        opacity = textureSample(opacityTexture, linearRepeatSampler, in.uv ).r;
    }

    if (opacity < 0.1)
    {
        discard; // Discard the pixel if it is below a certain threshold.
    }

    var ambient = material.ambient;
    var emissive = material.emissive;
    var diffuse = material.diffuse;
    var specular = material.specular;
    var specularPower = material.specularPower;

    if (material.hasAmbientTexture != 0)
    {
        ambient = textureSample(ambientTexture, linearRepeatSampler, in.uv);
    }
    if (material.hasEmissiveTexture != 0)
    {
        emissive = textureSample(emissiveTexture, linearRepeatSampler, in.uv);
    }
    if (material.hasDiffuseTexture != 0)
    {
        diffuse = textureSample(diffuseTexture, linearRepeatSampler, in.uv);
    }
    if (material.hasSpecularTexture != 0)
    {
        specular = textureSample(specularTexture, linearRepeatSampler, in.uv);
    }
    if (material.hasSpecularPowerTexture != 0)
    {
        specularPower *= textureSample(specularPowerTexture, linearRepeatSampler, in.uv).x;
    }

    var N = normalize(in.normalVS);
    if (material.hasNormalTexture != 0)
    {
        var tangent = normalize(in.tangentVS);
        var bitangent = normalize(in.bitangentVS);
        var normal = normalize(in.normalVS);

        let TBN = mat3x3f(tangent, bitangent, normal);

        N = DoNormalMapping(TBN, normalTexture, in.uv);
    }
    else if (material.hasBumpTexture != 0)
    {
        var tangent = normalize(in.tangentVS);
        var bitangent = normalize(in.bitangentVS);
        var normal = normalize(in.normalVS);

        let TBN = mat3x3f(tangent, bitangent, normal);

        N = DoBumpMapping(TBN, bumpTexture, in.uv, material.bumpIntensity);
    }

    let lighting = DoLighting( in.positionVS, N, specularPower );

    ambient *= lighting.ambient;
    diffuse *= lighting.diffuse;
    specular *= lighting.specular;

    return vec4f( (emissive + ambient + diffuse + specular).rgb, opacity * material.opacity);
}
)"