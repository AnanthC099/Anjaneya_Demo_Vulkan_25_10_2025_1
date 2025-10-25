#version 460 core
#extension GL_ARB_separate_shader_objects : enable

// Input from vertex shader
layout(location = 0) in vec2 out_TexCoord;

// Output color
layout(location = 0) out vec4 vFragColor;

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

// Texture samplers
layout(binding = 2) uniform sampler2D uScene0Texture;  // Scene0 offscreen texture
layout(binding = 3) uniform sampler2D uScene1Texture;  // Scene1 offscreen texture
layout(binding = 4) uniform sampler2D uQuadTexture;    // Original quad texture (fallback)

void main(void)
{
    // Sample textures
    vec4 scene0Color = texture(uScene0Texture, out_TexCoord);
    vec4 scene1Color = texture(uScene1Texture, out_TexCoord);
    vec4 quadColor = texture(uQuadTexture, out_TexCoord);
    
    // Apply fade factors
    scene0Color *= uCompositing.scene0Fade;
    scene1Color *= uCompositing.scene1Fade;
    
    // Determine final color based on blend mode
    vec4 finalColor;
    
    if (uCompositing.blendMode < 0.5) {
        // Additive blending
        finalColor = scene0Color + scene1Color;
    } else if (uCompositing.blendMode < 1.5) {
        // Alpha blending
        finalColor = mix(scene0Color, scene1Color, scene1Color.a);
    } else {
        // Multiply blending
        finalColor = scene0Color * scene1Color;
    }
    
    // If both scenes are invisible, show the original quad texture
    if (uCompositing.scene0Fade <= 0.0 && uCompositing.scene1Fade <= 0.0) {
        finalColor = quadColor;
    }
    
    // Ensure alpha is properly set
    finalColor.a = max(finalColor.a, max(scene0Color.a, scene1Color.a));
    
    vFragColor = finalColor;
}