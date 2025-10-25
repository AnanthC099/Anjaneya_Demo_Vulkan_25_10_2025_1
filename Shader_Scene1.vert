#version 450 core
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec3 vNormal;

layout(location = 0) out vec3 out_transformedNormal;
layout(location = 1) out vec3 out_lightDirection0;
layout(location = 2) out vec3 out_viewerVector;
layout(location = 3) out vec3 out_lightDirection1;

layout(binding = 0) uniform mvpMatrix {
        mat4 modelMatrix;
        mat4 viewMatrix;
        mat4 projectionMatrix;

        vec4 lightAmbient[2];
        vec4 lightDiffuse[2];
        vec4 lightSpecular[2];
        vec4 lightPosition[2];

        vec4 materialAmbient;
        vec4 materialDiffuse;
        vec4 materialSpecular;
        float materialShininess;
        vec4 padding;
} uMVP;

void main(void)
{
        mat4 modelViewMatrix = uMVP.viewMatrix * uMVP.modelMatrix;
        vec4 eyeCoordinates = modelViewMatrix * vec4(vPosition, 1.0);
        mat3 normalMatrix = mat3(transpose(inverse(modelViewMatrix)));

        vec4 lightPositionEye0 = uMVP.viewMatrix * uMVP.lightPosition[0];
        vec4 lightPositionEye1 = uMVP.viewMatrix * uMVP.lightPosition[1];

        out_transformedNormal = normalMatrix * vNormal;
        out_lightDirection0 = vec3(lightPositionEye0 - eyeCoordinates);
        out_lightDirection1 = vec3(lightPositionEye1 - eyeCoordinates);
        out_viewerVector = -eyeCoordinates.xyz;

        gl_Position = uMVP.projectionMatrix * eyeCoordinates;
}
