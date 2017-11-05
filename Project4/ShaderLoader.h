#pragma once
#include <fstream>
#include <string>
#include <vector>

#include "GL/glew.h"

std::string readFile(const char *filePath) {
	std::ifstream in(filePath, std::ios::in | std::ios::binary);
	if (in.is_open()){
		std::string contents;
		in.seekg(0, std::ios::end);
		contents.resize(in.tellg());
		in.seekg(0, std::ios::beg);
		in.read(&contents[0], contents.size());
		in.close();
		return contents ;
	}
	throw(errno);
}

void compileShader(GLuint program, GLuint shader, const char *path) {
	// Read shader
	std::string shaderStr = readFile(path);
	const char *shaderSrc = shaderStr.c_str();

	GLint result = GL_FALSE;
	int logLength;

	// Compile compute shader
	std::cout << "Compiling " << path << std::endl;
	glShaderSource(shader, 1, &shaderSrc, NULL);
	glCompileShader(shader);


	// Check compute shader
	glGetShaderiv(shader, GL_COMPILE_STATUS, &result);
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
	std::vector<GLchar> shaderError((logLength > 1) ? logLength : 1);
	glGetShaderInfoLog(shader, logLength, NULL, &shaderError[0]);
	std::cout << &shaderError[0] << std::endl;

	glAttachShader(program, shader);
}

void linkProgram(GLuint program) {
	GLint result = GL_FALSE;
	int logLength;

	std::cout << "Linking program" << std::endl;
	glLinkProgram(program);

	glGetProgramiv(program, GL_LINK_STATUS, &result);
	glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
	std::vector<char> programError((logLength > 1) ? logLength : 1);
	glGetProgramInfoLog(program, logLength, NULL, &programError[0]);
	std::cout << &programError[0] << std::endl;
}

GLuint loadShaders(const char *vertex_path, const char *fragment_path) {
	GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
	GLuint program = glCreateProgram();
	compileShader(program, vertShader, vertex_path);
	compileShader(program, fragShader, fragment_path);
	linkProgram(program);
	glDetachShader(program, vertShader);
	glDetachShader(program, fragShader);
	glDeleteShader(vertShader);
	glDeleteShader(fragShader);

	return program;
}