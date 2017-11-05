#version 420
#extension GL_ARB_explicit_uniform_location : require

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;

layout (location = 0) uniform mat4 perspective;
layout (location = 1) uniform mat4 view;
layout (location = 2) uniform mat4 model;

layout (location = 4) uniform vec4 uShadowPlane;

layout (std140, binding = 1) uniform lightParams {
	vec4 pointLightPos;
	vec4 pointLightInt;
	vec4 dirLightDir;
	vec4 dirLightInt;
	vec4 specularParams;
};

out vec3 normal_fs;
out vec2 uv_fs;

void main(){
	vec4 wldPos = model * vec4(position, 1.0);
	vec3 ray = wldPos.xyz - pointLightPos.xyz;
	float r = -dot(uShadowPlane.xyz, wldPos.xyz) - uShadowPlane.w;
	float q = dot(uShadowPlane.xyz, ray);
	vec3 projectedVertex = r/q * ray + wldPos.xyz;
	projectedVertex += uShadowPlane.xyz * 0.01f; // offset the shadow off the plane a little to avoid z-fighting
	
	gl_Position = (perspective * view) * vec4(projectedVertex, 1);
	normal_fs = normalize(uShadowPlane.xyz);
	
	uv_fs = (projectedVertex.xz * vec2(1, -1)) / 1000 + 0.5;
	uv_fs.y -= 0.005;
}
