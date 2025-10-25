#version 460 core
#extension GL_ARB_separate_shader_objects : enable
layout(location = 0) in vec2 out_TexCoord;
layout(binding = 1) uniform sampler2D uTextureSampler;
layout(binding = 2) uniform sampler2D uSecondaryTextureSampler;
layout(std140, binding = 3) uniform BlendParams
{
    float blendFactor;
} uBlendParams;
layout(location = 0) out vec4 vFragColor;
void main (void)
{
    //code
    vec4 colorPrimary = texture(uTextureSampler, out_TexCoord);
    vec4 colorSecondary = texture(uSecondaryTextureSampler, out_TexCoord);
    float t = clamp(uBlendParams.blendFactor, 0.0, 1.0);
    vFragColor = mix(colorPrimary, colorSecondary, t);
}
