#version 450 core
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 FragColor;

layout(location = 0) in vec3 out_transformedNormal;
layout(location = 1) in vec3 out_lightDirection0;
layout(location = 2) in vec3 out_viewerVector;
layout(location = 3) in vec3 out_lightDirection1;

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
        uint lKeyIsPressed;
        vec2 padding;
} uMVP;

void main(void)
{
        vec3 phong_ads_light = vec3(0.0);

        if(uMVP.lKeyIsPressed == 1u)
        {
                vec3 normalizedNormal = normalize(out_transformedNormal);
                vec3 normalizedViewerVector = normalize(out_viewerVector);
                vec3 lightDirections[2] = vec3[2](out_lightDirection0, out_lightDirection1);

                for(int i = 0; i < 2; ++i)
                {
                        vec3 normalizedLightDirection = normalize(lightDirections[i]);
                        vec3 reflectionVector = reflect(-normalizedLightDirection, normalizedNormal);

                        vec3 ambientLight = vec3(uMVP.lightAmbient[i]) * vec3(uMVP.materialAmbient);
                        vec3 diffusedLight = vec3(uMVP.lightDiffuse[i]) * vec3(uMVP.materialDiffuse) *
                                             max(dot(normalizedLightDirection, normalizedNormal), 0.0);
                        vec3 specularLight = vec3(uMVP.lightSpecular[i]) * vec3(uMVP.materialSpecular) *
                                             pow(max(dot(reflectionVector, normalizedViewerVector), 0.0), uMVP.materialShininess);

                        phong_ads_light += ambientLight + diffusedLight + specularLight;
                }
        }
        else
        {
                phong_ads_light = vec3(1.0);
        }

        FragColor = vec4(phong_ads_light, 1.0);
}
