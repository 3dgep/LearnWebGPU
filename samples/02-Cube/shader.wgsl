R"(
struct VertexIn
{
    @location(0) position: vec3f,
    @location(1) color: vec3f,
};

struct VertexOut
{
    @builtin(position) position: vec4f,
    @location(0) color: vec3f,
};

struct FragmentIn
{
    @location(0) color: vec3f
};

// Model View Projection matrix.
@group(0) @binding(0) var<uniform> mvp : mat4x4f;

@vertex
fn vs_main(in: VertexIn) -> VertexOut
{
    var out: VertexOut;
    out.position =  mvp * vec4f(in.position, 1.0);
    out.color = in.color;
    return out;
}

@fragment
fn fs_main(in: FragmentIn) -> @location(0) vec4f {
    return vec4f(in.color, 1.0);
}
)"