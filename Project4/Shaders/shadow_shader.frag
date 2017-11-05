#version 420
//NOTE: need version 420 to use bindings otherwise you have to get location from c++
layout (std140, binding = 4) uniform IslandColors{ 
  vec4 islandColor1;
  vec4 islandColor2;
  vec4 waterColor1;
  vec4 waterColor2;
};

layout (std140, binding = 1) uniform lightParams {
	vec4 pointLightPos;
	vec4 pointLightInt;
	vec4 dirLightDir;
	vec4 dirLightInt;
	vec4 specularParams;
};

in vec2 uv_fs;
in vec3 normal_fs;

out vec4 FragColor;

vec3 dirLight() {
	vec3 lightDir = -normalize(dirLightDir.xyz); // uLightDir is direction light goes, we need direction to light
	vec3 norm = normalize(normal_fs);
	float intensity = dot(norm, lightDir);
	if (intensity <= 0.0)
		return vec3(0.0, 0.0, 0.0);
	else
		return intensity * dirLightInt.xyz;
}

float hash( vec2 p ){
	float h = dot(p,vec2(127.1,311.7));
    return -1.0 + 2.0*fract(sin(h)*43758.5453123);// + .0001 * time);
}

float valueNoise(vec2 p){
    vec2 i = floor(p);
    vec2 f = fract(p);
	
	vec2 u = f*f*f*(f*(f*6-15)+10);

    return mix( mix( hash( i + vec2(0.0,0.0) ), 
                     hash( i + vec2(1.0,0.0) ), u.x),
                mix( hash( i + vec2(0.0,1.0) ), 
                     hash( i + vec2(1.0,1.0) ), u.x), u.y);
}

float f(vec2 uv){
	float f = 0, freq = 1;
	mat2 m = mat2( 1.6,  1.2, -1.2,  1.6 );
	for(int i = 0; i < 16; i++){
		f += freq*valueNoise( uv );
		//f += freq*perlinNoise( uv );
		uv = m*uv;
		freq /= 2;
	}
	//f  = 0.5000*valueNoise( uv ); uv = m*uv;
	//f += 0.2500*valueNoise( uv ); uv = m*uv;
	//f += 0.1250*valueNoise( uv ); uv = m*uv;
	//f += 0.0625*valueNoise( uv ); uv = m*uv;
	return f;
}

vec4 getColor(vec2 p){
	//p = p / 64;
	float d = f(p);
	//return mix(vec3(0, 0, .5), vec3(0, 1, 1), (d + 1) / 2);
	if(d > 0)
		return mix(islandColor1, islandColor2, d);
	else
		return mix(waterColor1, waterColor2, -d);
	//return vec3(d, d, d);
}

void main(){
	vec3 ambientLight = vec3(0.05, 0.05, 0.05);
	vec4 color = getColor(11*uv_fs);
	
	vec3 light = ambientLight;
	light += dirLight();
	
    FragColor = vec4(color.xyz * light, 1);
}