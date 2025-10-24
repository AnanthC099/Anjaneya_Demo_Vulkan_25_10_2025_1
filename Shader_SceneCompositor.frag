#version 460 core
#extension GL_ARB_separate_shader_objects : enable
layout(location = 0) in vec2 out_TexCoord;
layout(binding = 0) uniform sampler2D uSceneTextures[3]; // Scene0, Scene1, Scene2
layout(binding = 1) uniform TransitionData
{
    int currentScene;
    int targetScene;
    float transitionParameter;
} uTransition;

layout(location = 0) out vec4 vFragColor;

void main (void)
{
    // Get colors from current and target scenes
    vec4 currentColor = vec4(0.0, 0.0, 0.0, 1.0);
    vec4 targetColor = vec4(0.0, 0.0, 0.0, 1.0);
    
    // Sample from current scene texture
    if (uTransition.currentScene >= 0 && uTransition.currentScene < 3)
    {
        currentColor = texture(uSceneTextures[uTransition.currentScene], out_TexCoord);
    }
    
    // Sample from target scene texture
    if (uTransition.targetScene >= 0 && uTransition.targetScene < 3)
    {
        targetColor = texture(uSceneTextures[uTransition.targetScene], out_TexCoord);
    }
    
    // Blend between current and target scenes based on transition parameter
    float t = uTransition.transitionParameter;
    vFragColor = mix(currentColor, targetColor, t);
}
