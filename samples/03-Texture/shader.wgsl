R"(
struct VertexIn
{
    @location(0) position: vec3f,
    @location(1) uv: vec2f,
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

// Model View Projection matrix.
@group(0) @binding(0) var<uniform> mvp : mat4x4f;
@group(0) @binding(1) var albedoTexture : texture_2d<f32>;
@group(0) @binding(2) var linearRepeatSampler : sampler;

@vertex
fn vs_main(in: VertexIn) -> VertexOut
{
    var out: VertexOut;
    out.position =  mvp * vec4f(in.position, 1.0);
    out.uv = in.uv;
    return out;
}

@fragment
fn fs_main(in: FragmentIn) -> @location(0) vec4f {
    return textureSample(albedoTexture, linearRepeatSampler, in.uv);
}
)"