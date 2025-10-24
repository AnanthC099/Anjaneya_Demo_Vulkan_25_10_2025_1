#version 460 core
#extension GL_ARB_separate_shader_objects : enable
//attribute
layout(location = 0) in vec4 vPosition;
layout(location = 1) in vec2 vTexCoord;
layout(location = 0) out vec2 out_TexCoord;

void main (void)
{
    //code
    // For full-screen quad, we can use the position directly
    gl_Position = vPosition;
    out_TexCoord = vTexCoord;
}
