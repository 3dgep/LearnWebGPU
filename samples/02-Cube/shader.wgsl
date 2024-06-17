R"(
struct IN
{
    @location(0) position: vec3f,
    @location(1) color: vec3f,
};

struct V_OUT
{
    @builtin(position) position: vec4f,
    @location(0) color: vec3f,
};

@vertex
fn vs_main(in: IN) -> V_OUT
{
    var out: V_OUT;
    out.position = vec4f(in.position, 1.0);
    out.color = in.color;
    return out;
}

@fragment
fn fs_main(in: V_OUT) -> @location(0) vec4f {
    return vec4f(in.color, 1.0);
}
)"