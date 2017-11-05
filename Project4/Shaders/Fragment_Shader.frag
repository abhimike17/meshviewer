#version 420
#extension GL_ARB_explicit_uniform_location : require

in vec3 normal_fs;
in vec3 pointLightDir_fs;
in vec3 eyeDir_fs;

layout (std140, binding = 1) uniform lightParams {
	vec4 pointLightPos;
	vec4 pointLightInt;
	vec4 dirLightDir;
	vec4 dirLightInt;
	vec4 specularParams;
};

layout (location = 3) uniform bool selected;

out vec4 FragColor;

vec3 phong() {
	float distance = length(pointLightDir_fs);
	vec3 pointLightDir = normalize(pointLightDir_fs);
	vec3 norm = normalize(normal_fs);
	float intensity = dot(norm, pointLightDir);
	if (intensity <= 0.0)
		return vec3(0.0, 0.0, 0.0);
	float falloff = 1.0 / (1.0 + distance*distance);
	
	vec3 diffuse = intensity * falloff * pointLightInt.xyz; // this is the diffuse component
	
	// compute specular reflection:
	float specPow = specularParams.x;
	float specBoost = specularParams.y;
	float specValue = clamp(dot(normalize(eyeDir_fs), reflect(-pointLightDir, norm)), 0.0, 1.0);
	vec3 spec = pow(specValue, specPow) * specBoost * pointLightInt.xyz; // this is the specular component
	
	return diffuse + spec;
}

vec3 dirLight() {
	vec3 lightDir = -normalize(dirLightDir.xyz); // uLightDir is direction light goes, we need direction to light
	vec3 norm = normalize(normal_fs);
	float intensity = dot(norm, lightDir);
	if (intensity <= 0.0)
		return vec3(0.0, 0.0, 0.0);
	else
		return intensity * dirLightInt.xyz;
}

void main(){
	vec3 nColor = .5*normal_fs+.5;
	//FragColor = vec4(selected ? normal_fs : color, 1.0);
	//FragColor = vec4(eyeDir_fs, 1);
	
	vec3 ambientLight = vec3(0.05, 0.05, 0.05);
	vec3 baseColor = vec3(0.8, 0.8, 0.8);
	
	vec3 color = vec3(0, 0, 0);
	color += baseColor;
	color += nColor * 0.2;
	vec3 light = ambientLight;
	light += phong();
	light += dirLight();
	
	//light = vec3(1,1,1);
	//color = vec3(1, 1, 1);
	
	float alpha = 0.8 * clamp(length(color * light), 0.0, 0.8);
	
	FragColor = vec4(color * light, alpha);
}