#version 450 core
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 FragColor;

layout(location = 0) in vec3 out_transformedNormal;
layout(location = 1) in vec3 out_lightDirection[3];
layout(location = 4) in vec3 out_viewerVector;

layout(binding = 0) uniform MyUniformData {
	mat4 modelMatrix;
	mat4 viewMatrix;
	mat4 projectionMatrix;
	
	//Lighting related Uniforms
        vec4 lightAmbient[3];
        vec4 lightDiffuse[3];
        vec4 lightSpecular[3];
        vec4 lightPosition[3];
	
	//Material related Uniforms
	vec4 materialAmbient;
	vec4 materialDiffuse;
	vec4 materialSpecular;
	float materialShininess;
	
	//Key press related uniforms
	uint lKeyIsPressed;
	
}UMyUniformData;

void main(void)
{
        vec3 phong_ads_light;
        if(UMyUniformData.lKeyIsPressed == 1)
        {
                vec3 normalizedTransformedNormal = normalize(out_transformedNormal);
                vec3 normalizedViewerVector = normalize(out_viewerVector);
                phong_ads_light = vec3(0.0);
                for(int i = 0; i < 3; i++)
                {
                        vec3 normalizedLightDirection = normalize(out_lightDirection[i]);
                        vec3 ambientLight = vec3(UMyUniformData.lightAmbient[i]) * vec3(UMyUniformData.materialAmbient);
                        vec3 diffusedLight =  vec3(UMyUniformData.lightDiffuse[i]) * vec3(UMyUniformData.materialDiffuse) * max(dot(normalizedLightDirection, normalizedTransformedNormal), 0.0);
                        vec3 reflectionVector = reflect(-normalizedLightDirection, normalizedTransformedNormal);
                        vec3 specularLight = vec3(UMyUniformData.lightSpecular[i]) * vec3(UMyUniformData.materialSpecular) * pow(max(dot(reflectionVector, normalizedViewerVector), 0.0), UMyUniformData.materialShininess);
                        phong_ads_light += ambientLight + diffusedLight + specularLight;
                }
        }
        else
        {
                phong_ads_light = vec3(1.0, 1.0, 1.0);
        }
        FragColor = vec4(phong_ads_light, 1.0);
}
