//===----------------------------------------------------------------------===//
//
// This file is part of the Noxoscope project
//
// Copyright (c) 2016 Niklas Helmertz
//
//===----------------------------------------------------------------------===//

#include <cstdio>
#include <string>

#include <SDL.h>
#include <GL/glew.h>

#define NOMINMAX
#include <rlutil.h>
#undef near
#undef far

#include "Noxoscope.h"
#include "Logging.h"
#include "Constants.h"

auto RENDERING_INFO =
R"(==============
Renderer Info
==============
SDL video driver: {}
Version: {}
Vendor: {}
Renderer: {}
Shading language version: {}
)";

int main(int argc, char* argv[]) {
	// Initialize SDL
	SDL_version compiled;
	SDL_version linked;

	SDL_VERSION(&compiled);
	SDL_GetVersion(&linked);

	debug("Compiled with SDL {}.{}.{}", compiled.major, compiled.minor, compiled.patch);
	debug("Linked with SDL {}.{}.{}", linked.major, linked.minor, linked.patch);

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		fatalError("SDL init failed: {}", SDL_GetError());
	}

	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(
		SDL_GL_CONTEXT_FLAGS,
		SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG
#ifndef NDEBUG
		| SDL_GL_CONTEXT_DEBUG_FLAG
#endif
	);

	Uint32 flags =
		SDL_WINDOW_OPENGL |
		SDL_WINDOW_SHOWN |
		SDL_WINDOW_RESIZABLE;

	auto mainWindow = SDL_CreateWindow(PROGRAM_NAME, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		DEFAULT_WIDTH, DEFAULT_HEIGHT, flags);

	if (!mainWindow) {
		fatalError("Creating SDL window failed: {}", SDL_GetError());
	}

	auto mainContext = SDL_GL_CreateContext(mainWindow);
	if (!mainContext) {
		fatalError("Creating SDL GL context failed: {}", SDL_GetError());
	}

	debugColored("SDL setup successful", rlutil::LIGHTGREEN);

	// Load GLEW
	GLenum glewResult;
	auto glewVersion = glewGetString(GLEW_VERSION);
	debug("Loading GLEW version {}", glewVersion);
	glewExperimental = GL_TRUE;
	glewResult = glewInit();
	if (GLEW_OK != glewResult) {
		fatalError("Error: {}", glewGetErrorString(glewResult));
	}
	debugColored("GLEW setup successful", rlutil::LIGHTGREEN);
	debug("");

#ifndef NDEBUG
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
#endif

	// Show renderer information
	debug(RENDERING_INFO,
		SDL_GetCurrentVideoDriver(),
		glGetString(GL_VERSION),
		glGetString(GL_VENDOR),
		glGetString(GL_RENDERER),
		glGetString(GL_SHADING_LANGUAGE_VERSION));

	// Start the main part of program
	Noxoscope::loadAndRun(mainWindow, mainContext);

	// Cleanup
	SDL_GL_DeleteContext(mainContext);
	SDL_DestroyWindow(mainWindow);
	SDL_Quit();

	return 0;
}
