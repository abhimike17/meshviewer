#include <fstream>
#include <sstream>
#include <string>
#include <math.h>
#include <vector>
#include <algorithm>
#include <unordered_map>

#include "GL/glew.h"
#include "GL/freeglut.h"

#include "vec.h"
#include "mat.h"

#include "ShaderLoader.h"

enum Uniform { pMatrix = 0, vMatrix, mMatrix, invertColor, shadowPlane };
enum Attribute { position = 0, normal };

struct IslandColors {
	vec4 islandColor1;
	vec4 islandColor2;
	vec4 waterColor1;
	vec4 waterColor2;
};
IslandColors islandColors;

struct LightData {
	vec4 pointLightPos;
	vec4 pointLightIntensity;
	vec4 dirLightDir;
	vec4 dirLightIntensity;
	vec4 specularParams;
} lightData{
	{ 0, 5, 10, 0 },
	{ 10.f, 15.f, 25.f, 0 },
	{ 1.f, -1.f, 1.f, 0 },
	{ 0.75f, 0.7f, 0.3f, 0 },
	{ 10.f, 0.05f, 0, 0 }
};

struct VAO {
	GLuint id;
	int size;
};

VAO bunny;
VAO quad;


GLuint ubo;
GLuint ubo_lights;

GLuint shaderProagram;
GLuint grassShader;
GLuint shadowProgram;

float fov = 60;
float aspectRatio = 1;
float zNear = .1f;
float zFar = 1000;
mat4 perspective = Perspective(fov, aspectRatio, zNear, zFar);

float pitch = -15;
float yaw = 0;
vec3 eye(-3, 5, 15);

vec3 pos[2] = { vec3(-3, 0, 7), vec3(-3, 1, 7) };
vec3 rot[2] = { vec3(), vec3() };
vec3 scale[2] = { vec3(2), vec3(.02f) };

int transformMode = 0;
bool uniformScale = true;
float speed[4] = { .1, 15, .1, 1 };

const int NUM_ANIMALS = 1;
mat4 modelMatricies[NUM_ANIMALS];

bool transparency = false;


mat4 FPSViewRH(vec3 eye, float pitch, float yaw)
{
	pitch *= DegreesToRadians;
	yaw *= DegreesToRadians;

	float cosPitch = cos(pitch);
	float sinPitch = sin(pitch);
	float cosYaw = cos(yaw);
	float sinYaw = sin(yaw);

	vec3 xaxis = { cosYaw, 0, -sinYaw };
	vec3 yaxis = { sinYaw * sinPitch, cosPitch, cosYaw * sinPitch };
	vec3 zaxis = { sinYaw * cosPitch, -sinPitch, cosPitch * cosYaw };

	// Create a 4x4 view matrix from the right, up, forward and eye position vectors
	mat4 viewMatrix = {
		xaxis.x,            yaxis.x,            zaxis.x,      0,
		xaxis.y,            yaxis.y,            zaxis.y,      0,
		xaxis.z,            yaxis.z,            zaxis.z,      0,
		-dot(xaxis, eye), -dot(yaxis, eye), -dot(zaxis, eye), 1
	};

	return transpose(viewMatrix);


}
void display() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(grassShader);
	glUniformMatrix4fv(Uniform::vMatrix, 1, GL_TRUE, &FPSViewRH(eye, pitch, yaw)[0][0]);

	glBindVertexArray(quad.id);
	glDrawElements(GL_TRIANGLES, quad.size, GL_UNSIGNED_INT, 0);

	glUseProgram(shaderProagram);
	glUniformMatrix4fv(Uniform::vMatrix, 1, GL_TRUE, &FPSViewRH(eye, pitch, yaw)[0][0]);

	mat4 scaleMat = uniformScale ? Scale(vec3(scale[0].x)) : Scale(scale[0]);
	mat4 model = Translate(pos[0]) * RotateY(rot[0].y) * RotateZ(rot[0].z) * RotateX(rot[0].x) * scaleMat;

	if (transparency) {
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_CULL_FACE);
	}

	glBindVertexArray(bunny.id);
	glUniformMatrix4fv(Uniform::mMatrix, 1, GL_TRUE, &model[0][0]);
	glUniform1i(Uniform::invertColor, true);
	if (transparency) {
		glFrontFace(GL_CW);
		glDrawElements(GL_TRIANGLES, bunny.size, GL_UNSIGNED_INT, 0);
		glFrontFace(GL_CCW);
	}
	glDrawElements(GL_TRIANGLES, bunny.size, GL_UNSIGNED_INT, 0);
	glUniform1i(Uniform::invertColor, false);
	for (int i = 0; i < NUM_ANIMALS / 2; i++) {
		glUniformMatrix4fv(Uniform::mMatrix, 1, GL_TRUE, &modelMatricies[i][0][0]);
		if (transparency) {
			glFrontFace(GL_CW);
			glDrawElements(GL_TRIANGLES, bunny.size, GL_UNSIGNED_INT, 0);
			glFrontFace(GL_CCW);
		}
		glDrawElements(GL_TRIANGLES, bunny.size, GL_UNSIGNED_INT, 0);
	}

	scaleMat = uniformScale ? Scale(vec3(scale[1].x)) : Scale(scale[1]);
	model = Translate(pos[1]) * RotateY(rot[1].y) * RotateZ(rot[1].z) * RotateX(rot[1].x) * scaleMat;


	glUseProgram(shadowProgram);
	glUniformMatrix4fv(Uniform::vMatrix, 1, GL_TRUE, &FPSViewRH(eye, pitch, yaw)[0][0]);
	glUniform4f(Uniform::shadowPlane, 0, 1, 0, 0);

	scaleMat = uniformScale ? Scale(vec3(scale[0].x)) : Scale(scale[0]);
	model = Translate(pos[0]) * RotateY(rot[0].y) * RotateZ(rot[0].z) * RotateX(rot[0].x) * scaleMat;

	glBindVertexArray(bunny.id);
	glUniformMatrix4fv(Uniform::mMatrix, 1, GL_TRUE, &model[0][0]);
	glDrawElements(GL_TRIANGLES, bunny.size, GL_UNSIGNED_INT, 0);
	for (int i = 0; i < NUM_ANIMALS / 2; i++) {
		glUniformMatrix4fv(Uniform::mMatrix, 1, GL_TRUE, &modelMatricies[i][0][0]);
		glDrawElements(GL_TRIANGLES, bunny.size, GL_UNSIGNED_INT, 0);
	}

	scaleMat = uniformScale ? Scale(vec3(scale[1].x)) : Scale(scale[1]);
	model = Translate(pos[1]) * RotateY(rot[1].y) * RotateZ(rot[1].z) * RotateX(rot[1].x) * scaleMat;


	glutSwapBuffers();
}

int timeNow;
int timePrev;
void idle() {
	timeNow = glutGet(GLUT_ELAPSED_TIME);
	int timeElapsed = timeNow - timePrev;
	if (timeElapsed > 16) {

		
		glutPostRedisplay();
		timePrev = timeNow;
	}
}

std::unordered_map<char, vec3> directions = { { 'w', vec3(0, 0, -1) },{ 's', vec3(0, 0, 1) },
{ 'd', vec3(1, 0, 0) },{ 'a', vec3(-1, 0, 0) },
{ 'e', vec3(0, -1, 0) },{ 'q', vec3(0, 1, 0) },
{ 'i', vec3(0, 0, -1) },{ 'k', vec3(0, 0, 1) },
{ 'l', vec3(1, 0, 0) },{ 'j', vec3(-1, 0, 0) },
{ 'o', vec3(0, -1, 0) },{ 'u', vec3(0, 1, 0) } };
std::unordered_map<char, int> selectionMap = { { 'z', 0 },{ 'x', 1 } };
int selected = 0;
void keyboard(unsigned char key, int x, int y) {
	if (key == 'w' || key == 's' || key == 'a' || key == 'd' || key == 'q' || key == 'e') {
		switch (transformMode) {
		case 0:
			pos[selected] += speed[transformMode] * directions[key];
			break;
		case 1:
			rot[selected] += speed[transformMode] * vec3(directions[key].z, directions[key].x, directions[key].y);
			break;
		case 2:
			scale[selected] += speed[transformMode] * directions[key];
			break;
		default:
			break;
		}
	}

	if (key == 'i' || key == 'k' || key == 'j' || key == 'l' || key == 'u' || key == 'o') {
		vec4 eyeChange = transpose(FPSViewRH(eye, pitch, yaw)) * (speed[3] * vec4(directions[key], 0));
		eye += vec3(eyeChange.x, eyeChange.y, eyeChange.z);
	}
	if (key == 'r')
		speed[transformMode] *= 1.1;
	if (key == 'f')
		speed[transformMode] *= .9;

	if (key == 'z' || key == 'x')
		selected = selectionMap[key];

	if (key == '1' || key == '2' || key == '3' || key == '4')
		transformMode = key - '1';

	if (key == 'c') {
		islandColors.islandColor1 = vec4(1, 1, 1, 0) - islandColors.islandColor1;
		islandColors.islandColor2 = vec4(1, 1, 1, 0) - islandColors.islandColor2;
		islandColors.waterColor1 = vec4(1, 1, 1, 0) - islandColors.waterColor1;
		islandColors.waterColor2 = vec4(1, 1, 1, 0) - islandColors.waterColor2;

		glBindBuffer(GL_UNIFORM_BUFFER, ubo);
		GLvoid* p = glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);
		memcpy(p, &islandColors, sizeof(IslandColors));
		glUnmapBuffer(GL_UNIFORM_BUFFER);
	}
	if (key == 'm')
		transparency = !transparency;
}

int prevX, prevY;
bool leftMouseDown = false;
void mouse(int button, int state, int x, int y) {
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		prevX = x;
		prevY = y;
	}
	leftMouseDown = button == GLUT_LEFT_BUTTON && state == GLUT_DOWN;
}

void mouseMotion(int x, int y) {
	if (leftMouseDown) {
		int xDif = x - prevX;
		int yDif = y - prevY;
		float rotSpeed = .1;
		pitch -= rotSpeed * yDif;
		yaw -= rotSpeed * xDif;
		prevX = x;
		prevY = y;
	}
}



static VAO CreateVertexBuffer(std::string filePath) {
	VAO mesh;

	std::vector<float> positions;
	std::vector<float> normals;
	std::vector<GLuint> indices;

	std::string contents;
	std::ifstream in(filePath, std::ios::in | std::ios::binary);
	if (in.is_open()) {
		in.seekg(0, std::ios::end);
		contents.resize(in.tellg());
		in.seekg(0, std::ios::beg);
		in.read(&contents[0], contents.size());
		in.close();
	}
	else
		throw(errno);

	const std::string vertex = "Vertex ";
	const std::string normal = "normal=(";
	const std::string face = "Face ";
	size_t line = 0;
	while (true) {
		if (contents[line] == '#') {
			line = contents.find('\n', line + 1);
			if (line != std::string::npos)
				line++;
			else
				break;
			continue;
		}

		if (contents.substr(line, vertex.length()).compare(vertex) == 0) {
			size_t current = contents.find(' ', line + vertex.length()) + 2;
			size_t next;
			positions.push_back(std::stof(contents.substr(current, contents.find(' ', current) - current), &next));
			current += next + 1;
			positions.push_back(std::stof(contents.substr(current, contents.find(' ', current) - current), &next));
			current += next + 1;
			positions.push_back(std::stof(contents.substr(current, contents.find(' ', current) - current), &next));
			current = current + next + 2;
			if ((current = contents.find(normal, line + vertex.length())) != std::string::npos) {
				
				current += normal.length();
				normals.push_back(std::stof(contents.substr(current, contents.find(' ', current) - current), &next));
				current += next + 1;
				normals.push_back(std::stof(contents.substr(current, contents.find(' ', current) - current), &next));
				current += next + 1;
				normals.push_back(std::stof(contents.substr(current, contents.find(' ', current) - current), &next));
				
			}
		}

		if (contents.substr(line, face.length()).compare(face) == 0) {
			
			size_t current = contents.find(' ', line + face.length()) + 2;
			
			size_t next;
			indices.push_back((GLuint)std::stoul(contents.substr(current, contents.find(' ', current) - current), &next) - 1);
			current += next + 1;
			indices.push_back((GLuint)std::stoul(contents.substr(current, contents.find(' ', current) - current), &next) - 1);
			current += next + 1;
			indices.push_back((GLuint)std::stoul(contents.substr(current, contents.find(' ', current) - current), &next) - 1);
			
		}

		line = contents.find('\n', line + 1);
		if (line != std::string::npos)
			line++;
		else
			break;
	}
	mesh.size = indices.size();

	glGenVertexArrays(1, &mesh.id);
	glBindVertexArray(mesh.id);


	GLuint positionVBO;
	glGenBuffers(1, &positionVBO);
	glBindBuffer(GL_ARRAY_BUFFER, positionVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * positions.size(), positions.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(Attribute::position, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(Attribute::position);

	GLuint normalVBO;
	glGenBuffers(1, &normalVBO);
	glBindBuffer(GL_ARRAY_BUFFER, normalVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * normals.size(), normals.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(Attribute::normal, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(Attribute::normal);

	GLuint indicesVBO;
	glGenBuffers(1, &indicesVBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indicesVBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * indices.size(), indices.data(), GL_STATIC_DRAW);

	return mesh;
}

float randf(float min, float max) {
	float random = ((float)rand()) / (float)RAND_MAX;
	float range = max - min;
	return (random*range) + min;
}

void init() {
	bunny = CreateVertexBuffer("bunny.m");
	quad = CreateVertexBuffer("quad.m");

	shaderProagram = loadShaders("Shaders/Vertex_Shader.vert", "Shaders/Fragment_Shader.frag");
	grassShader = loadShaders("Shaders/TerrainShader.vert", "Shaders/TerrainShader.frag");
	shadowProgram = loadShaders("Shaders/shadow_shader.vert", "Shaders/shadow_shader.frag");

	islandColors.islandColor1 = vec4(.93, .79, .69, 1);
	islandColors.islandColor2 = vec4(.13, .55, .13, 1);
	islandColors.waterColor1 = vec3(.73, .85, .88);
	islandColors.waterColor2 = vec3(0, .2, .4);

	glGenBuffers(1, &ubo);
	glBindBuffer(GL_UNIFORM_BUFFER, ubo);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(IslandColors), &islandColors, GL_DYNAMIC_DRAW);
	
	glBindBufferBase(GL_UNIFORM_BUFFER, 4, ubo);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	glGenBuffers(1, &ubo_lights);
	glBindBuffer(GL_UNIFORM_BUFFER, ubo_lights);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(LightData), &lightData, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, ubo_lights);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	glUseProgram(shaderProagram);
	glUniformMatrix4fv(Uniform::pMatrix, 1, GL_TRUE, &perspective[0][0]);

	glUseProgram(grassShader);
	glUniformMatrix4fv(Uniform::pMatrix, 1, GL_TRUE, &perspective[0][0]);

	float scale = 1000;
	mat4 quadModel = Translate(scale*vec3(-.5, 0, .5 + -5 / scale)) * RotateX(-90) * Scale(vec3(scale));
	glUniformMatrix4fv(Uniform::mMatrix, 1, GL_TRUE, &quadModel[0][0]);

	glUseProgram(shadowProgram);
	glUniformMatrix4fv(Uniform::pMatrix, 1, GL_TRUE, &perspective[0][0]);

	for (int i = 0; i < NUM_ANIMALS / 2; i++)
		modelMatricies[i] = Translate(vec3(randf(-10, 10) - 6, -.5, randf(-10, 10) - 6)) * RotateY(randf(0, 360)) * Scale(vec3(randf(.7, 1)));
	for (int i = NUM_ANIMALS / 2; i < NUM_ANIMALS; i++)
		modelMatricies[i] = Translate(vec3(randf(-10, 10) + 3, 0, randf(-10, 10) + 2)) * RotateY(randf(0, 360)) * Scale(vec3(randf(.005, .009)));

	glEnable(GL_DEPTH_TEST);
}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(600, 600);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("Bugs Bunny");

	glutDisplayFunc(display);
	glutIdleFunc(idle);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mouse);
	glutMotionFunc(mouseMotion);


	GLenum res = glewInit();
	if (res != GLEW_OK) {
		fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
		return 1;
	}


	printf("GL version: %s\n", glGetString(GL_VERSION));

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	init();

	glutMainLoop();

	return 0;
}