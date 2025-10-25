#version 450 core
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec4 vPosition;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec2 vTexCoord;

layout(location = 0) out vec3 out_transformedNormal;
layout(location = 1) out vec3 out_lightDirection[3];
layout(location = 4) out vec3 out_viewerVector;

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
        if(UMyUniformData.lKeyIsPressed == 1)
        {
                vec4 eyeCoordinates = UMyUniformData.viewMatrix * UMyUniformData.modelMatrix * vPosition;
                mat3 normalMatrix = mat3(transpose(inverse(UMyUniformData.viewMatrix * UMyUniformData.modelMatrix)));
                out_transformedNormal = normalMatrix * vNormal;
                for(int i = 0; i < 3; i++)
                {
                        out_lightDirection[i] = vec3(UMyUniformData.lightPosition[i] - eyeCoordinates);
                }
                out_viewerVector = -eyeCoordinates.xyz;
        }
        gl_Position = UMyUniformData.projectionMatrix * UMyUniformData.viewMatrix * UMyUniformData.modelMatrix * vPosition;
}
