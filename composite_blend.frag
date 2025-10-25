#version 450

layout(set = 0, binding = 0) uniform sampler2D uScene0;
layout(set = 0, binding = 1) uniform sampler2D uScene1;

layout(push_constant) uniform PC
{
    float t;
} pc;

layout(location = 0) in vec2 vUV;
layout(location = 0) out vec4 outColor;

void main()
{
    vec4 c0 = texture(uScene0, vUV);
    vec4 c1 = texture(uScene1, vUV);
    outColor = mix(c0, c1, clamp(pc.t, 0.0, 1.0));
}
