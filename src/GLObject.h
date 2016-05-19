//===----------------------------------------------------------------------===//
//
// This file is part of the Noxoscope project
//
// Copyright (c) 2016 Niklas Helmertz
//
//===----------------------------------------------------------------------===//

#ifndef GLObject_H
#define GLObject_H

#include <algorithm>

#include <GL/glew.h>

/// \brief Simple GL wrapper class. The main purpose is to automate the GL destruction.
template <typename T>
struct GLObject {
	GLObject() = default;

	explicit GLObject(typename T::handleType handle) : handle{handle} {}

	~GLObject() {
		del();
	}

	GLObject(GLObject<T> const&) = delete;
	void operator=(GLObject<T> const&) = delete;

	GLObject(GLObject<T>&& o) {
		std::swap(handle, o.handle);
	}

	GLObject<T>& operator=(GLObject<T>&& o) {
		std::swap(handle, o.handle);
		return *this;
	}

	void gen() {
		handle = T::gen();
	}

	void regen() {
		del();
		gen();
	}

	void del() {
		if (handle != 0) {
			T::del(handle);
			handle = 0;
		}
	}

	typename T::handleType handle = 0;
};

#define BASIC_GL_OBJECT(n, s, t, g, d) \
	struct s { \
		typedef t handleType; \
		static handleType gen() {t handle; g; return handle; } \
		static void del(handleType handle) { d; } \
	}; \
	typedef GLObject<s> n;

BASIC_GL_OBJECT(GLTexture, GLTextureTraits, GLuint, glGenTextures(1, &handle), glDeleteTextures(1, &handle))
BASIC_GL_OBJECT(GLFramebuffer, GLFramebufferTraits, GLuint, glGenFramebuffers(1, &handle), glDeleteFramebuffers(1, &handle))
BASIC_GL_OBJECT(GLRenderBuffer, GLRenderbufferTraits, GLuint, glGenRenderbuffers(1, &handle), glDeleteRenderbuffers(1, &handle))
BASIC_GL_OBJECT(GLVertexArray, GLVertexArrayTraits, GLuint, glGenVertexArrays(1, &handle), glDeleteVertexArrays(1, &handle))
BASIC_GL_OBJECT(GLBuffer, GLBufferTraits, GLuint, glGenBuffers(1, &handle), glDeleteBuffers(1, &handle))

#endif // GLObject_H
