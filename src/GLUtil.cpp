//===----------------------------------------------------------------------===//
//
// This file is part of the Noxoscope project
//
// Copyright (c) 2016 Niklas Helmertz
//
//===----------------------------------------------------------------------===//

#include "GLUtil.h"

#include <tuple>

#include "Logging.h"
#include "Model.h"

void attachTextures(const ShaderProgram& shader, const std::initializer_list<std::tuple<GLuint, const char*>>& textureTuples) {
	GLuint tex = 0;
	for (auto& tuple : textureTuples) {
		GLuint texID;
		const char* texName;
		std::tie(texID, texName) = tuple;
		glActiveTexture(GL_TEXTURE0 + tex);
		glBindTexture(GL_TEXTURE_2D, texID);
		glUniform1i(shader[texName], tex);
		tex++;
	}
}

void checkFboStatus() {
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE) {
		fatalError("Framebuffer not complete (code: 0x{:X})!", status);
	}
}
