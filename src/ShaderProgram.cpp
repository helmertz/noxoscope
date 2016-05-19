//===----------------------------------------------------------------------===//
//
// This file is part of the Noxoscope project
//
// Copyright (c) 2016 Niklas Helmertz
//
//===----------------------------------------------------------------------===//

#include "ShaderProgram.h"

#include <vector>
#include <limits>
#include <algorithm>

#include "Logging.h"
#include "FileTools.h"

ShaderProgram::ShaderProgram(const char* vertShader, const char* fragShader) :
	vertexPath{std::string(vertShader)},
	fragmentPath{std::string(fragShader)} {
	reload(true);
}

ShaderProgram::~ShaderProgram() {
	if (handle != 0) {
		glDeleteProgram(handle);
	}
}

ShaderProgram::ShaderProgram(ShaderProgram&& o) {
	std::swap(handle, o.handle);
	std::swap(fileModificationTime, o.fileModificationTime);
	std::swap(fragmentPath, o.fragmentPath);
	std::swap(vertexPath, o.vertexPath);
}

ShaderProgram& ShaderProgram::operator=(ShaderProgram&& o) {
	std::swap(handle, o.handle);
	std::swap(fileModificationTime, o.fileModificationTime);
	std::swap(fragmentPath, o.fragmentPath);
	std::swap(vertexPath, o.vertexPath);
	return *this;
}

void ShaderProgram::use() const {
	glUseProgram(handle);
}

void ShaderProgram::reload(bool alwaysReload) {
	struct stat vertStat;
	stat(vertexPath.c_str(), &vertStat);
	struct stat fragStat;
	stat(fragmentPath.c_str(), &fragStat);
	time_t latestModified = std::max(vertStat.st_mtime, fragStat.st_mtime);

	if (alwaysReload || latestModified > fileModificationTime) {
		fileModificationTime = latestModified;
		auto newProg = loadShader(vertexPath.c_str(), fragmentPath.c_str());
		if (newProg == 0) {
			return;
		}
		glDeleteProgram(handle);
		debug("Reloaded {}, {}", vertexPath.c_str(), fragmentPath.c_str());
		handle = newProg;
	}
}

GLint ShaderProgram::getUniform(const char* name) const {
	auto res = glGetUniformLocation(handle, name);
	return res;
}

GLint ShaderProgram::operator[](const char* uniformName) const {
	return getUniform(uniformName);
}

bool compileShader(GLuint shaderID, const char* source) {
	glShaderSource(shaderID, 1, &source, nullptr);
	glCompileShader(shaderID);

	GLint isCompiled = 0;
	glGetShaderiv(shaderID, GL_COMPILE_STATUS, &isCompiled);

	GLint msgLength = 0;
	glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &msgLength);
	std::vector<GLchar> compileMsg(msgLength + 1);
	glGetShaderInfoLog(shaderID, msgLength, &msgLength, &compileMsg[0]);
	compileMsg[msgLength] = '\0';
	if (!isCompiled) {
		errorLog("Shader compile error:\n{}", &compileMsg[0]);
		glDeleteShader(shaderID);
		return false;
	}
	if (msgLength > 0) {
		warn("Shader compile message:\n{}", &compileMsg[0]);
	}
	return true;
}

GLuint loadShader(const char* vertexPath, const char* fragmentPath) {
	auto vertShaderSrcStr = readFile(vertexPath);
	auto fragShaderSrcStr = readFile(fragmentPath);
	auto vertShaderSrc = vertShaderSrcStr.c_str();
	auto fragShaderSrc = fragShaderSrcStr.c_str();

	auto vertShader = glCreateShader(GL_VERTEX_SHADER);
	auto fragShader = glCreateShader(GL_FRAGMENT_SHADER);

	debug("Compiling vertex shader");
	if (!compileShader(vertShader, vertShaderSrc)) {
		return 0;
	}

	debug("Compiling fragment shader");
	if (!compileShader(fragShader, fragShaderSrc)) {
		return 0;
	}

	debug("Linking shader program");
	GLuint program = glCreateProgram();
	debug("Create shader {}", program);
	glAttachShader(program, vertShader);
	glAttachShader(program, fragShader);
	glLinkProgram(program);

	GLint isLinked;
	glGetProgramiv(program, GL_LINK_STATUS, &isLinked);
	if (!isLinked) {
		GLint length = 0;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);

		std::vector<GLchar> infoLog(length);
		if (length > 0) {
			glGetProgramInfoLog(program, length, &length, &infoLog[0]);
		}
		infoLog.push_back('\0');
		glDeleteProgram(program);

		errorLog("Shader link error:\n{}", &infoLog[0]);
		return 0;
	}

	glDeleteShader(vertShader);
	glDeleteShader(fragShader);

	return program;
}
