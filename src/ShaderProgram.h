//===----------------------------------------------------------------------===//
//
// This file is part of the Noxoscope project
//
// Copyright (c) 2016 Niklas Helmertz
//
//===----------------------------------------------------------------------===//

#ifndef ShaderProgram_H
#define ShaderProgram_H

#include <string>
#include <time.h>

#include <GL/glew.h>

class ShaderProgram {
public:
	ShaderProgram() = default;
	ShaderProgram(const char* vertexPath, const char* fragmentPath);
	~ShaderProgram();

	ShaderProgram(ShaderProgram const&) = delete;
	void operator=(ShaderProgram const& x) = delete;

	ShaderProgram(ShaderProgram&& o);
	ShaderProgram& operator=(ShaderProgram&& o);

	void use() const;
	void reload(bool alwaysReload);
	GLint getUniform(const char* name) const;
	GLint operator[](const char* uniformName) const;

	GLuint handle = 0;
private:
	std::string vertexPath;
	std::string fragmentPath;
	time_t fileModificationTime = 0;
};

GLuint loadShader(const char* vertexPath, const char* fragmentPath);

#endif // ShaderProgram_H
