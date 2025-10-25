#version 460 core
#extension GL_ARB_separate_shader_objects : enable

// Vertex attributes
layout(location = 0) in vec4 vPosition;
layout(location = 1) in vec2 vTexCoord;

// Output to fragment shader
layout(location = 0) out vec2 out_TexCoord;

// Uniform buffer for MVP matrices
layout(binding = 0) uniform MVPMatrix
{
    mat4 modelMatrix;
    mat4 viewMatrix;
    mat4 projectionMatrix;
} uMVP;

// Uniform buffer for compositing parameters
layout(binding = 1) uniform CompositingParams
{
    float scene0Fade;    // Fade factor for Scene0 (0.0 = invisible, 1.0 = fully visible)
    float scene1Fade;    // Fade factor for Scene1 (0.0 = invisible, 1.0 = fully visible)
    float blendMode;     // 0.0 = additive, 1.0 = alpha blend, 2.0 = multiply
    float time;          // Time for animations
} uCompositing;

void main(void)
{
    // Transform vertex position
    gl_Position = uMVP.projectionMatrix * uMVP.viewMatrix * uMVP.modelMatrix * vPosition;
    
    // Pass texture coordinates to fragment shader
    out_TexCoord = vTexCoord;
}