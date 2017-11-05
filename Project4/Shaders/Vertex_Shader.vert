#version 420
#extension GL_ARB_explicit_uniform_location : require

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;

layout (location = 0) uniform mat4 perspective;
layout (location = 1) uniform mat4 view;
layout (location = 2) uniform mat4 model;

layout (std140, binding = 1) uniform lightParams {
	vec4 pointLightPos;
	vec4 pointLightInt;
	vec4 dirLightDir;
	vec4 dirLightInt;
	vec4 specularParams;
};

out vec3 position_fs;
out vec3 normal_fs;
out vec3 pointLightDir_fs;
out vec3 eyeDir_fs;

void main(){
	position_fs = position;
	normal_fs = (model * vec4(normal, 0)).xyz;
	vec4 wldPos = model * vec4(position, 1.0);
	gl_Position = (perspective * view) * wldPos;
	
	pointLightDir_fs = pointLightPos.xyz - wldPos.xyz;
	vec3 eyePosition = vec3(view[0][3], view[1][3], view[2][3]);
	eyePosition = (inverse(view) * vec4(eyePosition, 1)).xyz;
	eyeDir_fs = eyePosition.xyz - wldPos.xyz;
}