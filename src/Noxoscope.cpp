//===---------------------------------------------------------------------===//
//
// This file is part of the Noxoscope project
//
// Copyright (c) 2016 Niklas Helmertz
//
//===----------------------------------------------------------------------===//
//
// Definition of the Noxoscope class.
//
//===----------------------------------------------------------------------===//

#include "Noxoscope.h"

#include <random>
#include <vector>
#include <thread>
#include <algorithm>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <imgui.h>
#include <imgui_impl_sdl_gl3.h>

#include "Logging.h"
#include "Constants.h"
#include "FileTools.h"
#include "GeometryMath.h"
#include "GLUtil.h"

void Noxoscope::loadAndRun(SDL_Window* mainWindow, SDL_GLContext mainContext) {
	Noxoscope noxoscope(mainWindow, mainContext);
	noxoscope.run();
}

Noxoscope::Noxoscope(SDL_Window* window, SDL_GLContext context) : mainwindow{window}, maincontext{context} {}

Noxoscope::~Noxoscope() {
	glUseProgram(0);
	ImGui_ImplSdlGL3_Shutdown();
}

void Noxoscope::run() {
	initialize();

	namespace chrono = std::chrono;
	startTime = chrono::high_resolution_clock::now();
	lastRender = chrono::high_resolution_clock::now();

	setupImgui();

	while (!quit) {
		float frameDiff;
		while (true) {
			now = chrono::high_resolution_clock::now();
			frameDiff = chrono::duration<float>(now - lastRender).count();
			if (!frameCap || targetFramerate <= 0 || frameDiff >= 1.0f / targetFramerate) {
				break;
			}
			if (frameCapSleep) {
				SDL_Delay(1);
			}
		}
		numFrames++;
		frameTimeAccumulator += frameDiff;
		deviationAccumulator += std::abs(lastFrametime - frameDiff);
		maxDeviation = std::max(maxDeviation, std::abs(lastFrametime - frameDiff));

		update(frameDiff);

		if (showGui) {
			ImGui_ImplSdlGL3_NewFrame();
		}

		auto secsNow = chrono::duration_cast<chrono::seconds>(now - startTime);
		auto secsLast = chrono::duration_cast<chrono::seconds>(lastRender - startTime);

		if (secsNow > secsLast) {
			onSecondPassed();
		}

		render();

		if (showGui) {
			renderGui();
		}
		SDL_GL_SwapWindow(mainwindow);
		lastRender = now;
	}
}

void Noxoscope::initialize() {
	using namespace glm;

#ifndef NDEBUG
	if (GLEW_ARB_debug_output) {
		glDebugMessageCallbackARB(onDebugEvent, nullptr);
	}
#endif

	loadModels();

	mainForwardShader = ShaderProgram(
		baseDirRelative("assets/shaders/forward_shader.vert").c_str(),
		baseDirRelative("assets/shaders/forward_shader.frag").c_str()
	);
	gBufferShader = ShaderProgram(
		baseDirRelative("assets/shaders/gbufferfill.vert").c_str(),
		baseDirRelative("assets/shaders/gbufferfill.frag").c_str()
	);
	lightCombineShader = ShaderProgram(
		baseDirRelative("assets/shaders/basic_post_process.vert").c_str(),
		baseDirRelative("assets/shaders/lightcombine.frag").c_str()
	);
	ssaoShader = ShaderProgram(
		baseDirRelative("assets/shaders/basic_post_process.vert").c_str(),
		baseDirRelative("assets/shaders/ssao.frag").c_str()
	);
	ssrShader = ShaderProgram(
		baseDirRelative("assets/shaders/basic_post_process.vert").c_str(),
		baseDirRelative("assets/shaders/ssr.frag").c_str()
	);
	renderTextureShader = ShaderProgram(
		baseDirRelative("assets/shaders/basic_post_process.vert").c_str(),
		baseDirRelative("assets/shaders/render_texture.frag").c_str()
	);
	blurShader = ShaderProgram(
		baseDirRelative("assets/shaders/basic_post_process.vert").c_str(),
		baseDirRelative("assets/shaders/blur.frag").c_str()
	);
	lightShader = ShaderProgram(
		baseDirRelative("assets/shaders/basic_post_process.vert").c_str(),
		baseDirRelative("assets/shaders/lightpass.frag").c_str()
	);
	simpleShader = ShaderProgram(
		baseDirRelative("assets/shaders/simplemvp.vert").c_str(),
		baseDirRelative("assets/shaders/simple.frag").c_str()
	);

	onResize();
}

void Noxoscope::reloadShaders() {
	mainForwardShader.reload(false);
	gBufferShader.reload(false);
	lightCombineShader.reload(false);
	ssaoShader.reload(false);
	ssrShader.reload(false);
	renderTextureShader.reload(false);
	blurShader.reload(false);
	lightShader.reload(false);
	simpleShader.reload(false);
}

void Noxoscope::reloadBuffers() {
	using namespace glm;

	gBuffer.regen();
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer.handle);

	GLenum attachNum = 0;

	const int NUM_GEOMETRY = 5;
	auto gBuffers = {
		std::make_tuple(&gBufferDepth, GL_RGBA32F, GL_RGBA, GL_FLOAT),
		std::make_tuple(&gBufferNormal, GL_RGBA32F, GL_RGBA, GL_FLOAT),
		std::make_tuple(&gBufferDiffuse, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE),
		std::make_tuple(&gBufferSpecular, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE),
		std::make_tuple(&gBufferNormalMappedNormal, GL_RGBA32F, GL_RGBA, GL_FLOAT)
	};

	for (auto& buffer : gBuffers) {
		GLTexture* tex;
		GLsizei internalFormat;
		GLenum format;
		GLenum type;
		std::tie(tex, internalFormat, format, type) = buffer;

		tex->regen();
		glBindTexture(GL_TEXTURE_2D, tex->handle);
		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, internalWidth, internalHeight, 0, format, type, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachNum, GL_TEXTURE_2D, tex->handle, 0);
		attachNum++;
	}
	GLenum attachments[NUM_GEOMETRY];
	for (GLenum i = 0; i < NUM_GEOMETRY; i++) {
		attachments[i] = GL_COLOR_ATTACHMENT0 + i;
	}
	glDrawBuffers(NUM_GEOMETRY, attachments);

	sharedDepthStencil.regen();
	glBindRenderbuffer(GL_RENDERBUFFER, sharedDepthStencil.handle);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, internalWidth, internalHeight);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, sharedDepthStencil.handle);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, sharedDepthStencil.handle);

	checkFboStatus();

	// Final output texture

	finalFbo1.regen();
	glBindFramebuffer(GL_FRAMEBUFFER, finalFbo1.handle);

	finalTexture1.regen();
	glBindTexture(GL_TEXTURE_2D, finalTexture1.handle);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, internalWidth, internalHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, finalTexture1.handle, 0);

	checkFboStatus();

	finalFbo2.regen();
	glBindFramebuffer(GL_FRAMEBUFFER, finalFbo2.handle);

	finalTexture2.regen();
	glBindTexture(GL_TEXTURE_2D, finalTexture2.handle);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, internalWidth, internalHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, finalTexture2.handle, 0);

	checkFboStatus();

	currentFinalFbo = &finalFbo1;
	currentFinalTexture = &finalTexture1;
	lastFinalFbo = &finalFbo2;
	lastFinalTexture = &finalTexture2;

	lightFbo.regen();
	glBindFramebuffer(GL_FRAMEBUFFER, lightFbo.handle);

	lightTexture.regen();
	glBindTexture(GL_TEXTURE_2D, lightTexture.handle);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, internalWidth, internalHeight, 0, GL_RGBA, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, lightTexture.handle, 0);

	checkFboStatus();

	glBindRenderbuffer(GL_RENDERBUFFER, sharedDepthStencil.handle);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, sharedDepthStencil.handle);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, sharedDepthStencil.handle);

	checkFboStatus();

	std::uniform_real_distribution<float> randomFloats(0.0f, 1.0f);
	std::default_random_engine generator;
	const int NUM_SSAO_KERNEL = 64;
	const int NUM_SSAO_NOISE = 16;
	for (int i = 0; i < NUM_SSAO_KERNEL; ++i) {
		vec3 sample(
			randomFloats(generator) * 2.0f - 1.0f,
			randomFloats(generator) * 2.0f - 1.0f,
			randomFloats(generator)
		);
		sample = normalize(sample);
		sample *= randomFloats(generator);
		float scale = static_cast<float>(i) / NUM_SSAO_KERNEL;
		scale = lerp(0.1f, 1.0f, scale * scale);
		ssaoKernel.push_back(sample * scale);
	}

	for (int i = 0; i < NUM_SSAO_NOISE; i++) {
		vec3 noise(
			randomFloats(generator) * 2.0f - 1.0f,
			randomFloats(generator) * 2.0f - 1.0f,
			0.0f);
		noise = normalize(noise);
		ssaoNoise.push_back(noise);
	}

	noiseTexture.regen();
	glBindTexture(GL_TEXTURE_2D, noiseTexture.handle);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	ssaoFbo.regen();
	glBindFramebuffer(GL_FRAMEBUFFER, ssaoFbo.handle);

	ssaoBuffer.regen();
	glBindTexture(GL_TEXTURE_2D, ssaoBuffer.handle);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, ssaoWidth, ssaoHeight, 0, GL_RED, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoBuffer.handle, 0);

	checkFboStatus();

	ssaoBlurFbo.regen();
	glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFbo.handle);

	ssaoBlurTexture.regen();
	glBindTexture(GL_TEXTURE_2D, ssaoBlurTexture.handle);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, ssaoWidth, ssaoHeight, 0, GL_RED, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoBlurTexture.handle, 0);

	checkFboStatus();

	ssrFbo.regen();
	glBindFramebuffer(GL_FRAMEBUFFER, ssrFbo.handle);

	ssrTexture.regen();
	glBindTexture(GL_TEXTURE_2D, ssrTexture.handle);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ssrWidth, ssrHeight, 0, GL_RGBA, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssrTexture.handle, 0);

	checkFboStatus();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Noxoscope::setupImgui() {
	using namespace ImGui;

	auto& io = GetIO();
	ImFontConfig config;
	config.OversampleH = 8;
	config.OversampleV = 8;
	io.FontAllowUserScaling = true;
	io.Fonts->AddFontFromFileTTF(baseDirRelative("assets/fonts/Roboto-Regular.ttf").c_str(), 18, &config);
	monoFont = io.Fonts->AddFontFromFileTTF(baseDirRelative("assets/fonts/RobotoMono-Regular.ttf").c_str(), 18, &config);

	// Disable logging and settings
	io.IniFilename = nullptr;
	io.LogFilename = nullptr;

	ImGui_ImplSdlGL3_Init(mainwindow);

	auto& style = GetStyle();
	style.Colors[ImGuiCol_Text] = ImVec4(0.00f, 0.00f, 0.00f, 0.92f);
	style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
	style.Colors[ImGuiCol_WindowBg] = ImVec4(0.94f, 0.94f, 0.94f, 1.00f);
	style.Colors[ImGuiCol_ChildWindowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	style.Colors[ImGuiCol_Border] = ImVec4(0.00f, 0.00f, 0.00f, 0.39f);
	style.Colors[ImGuiCol_BorderShadow] = ImVec4(1.00f, 1.00f, 1.00f, 0.10f);
	style.Colors[ImGuiCol_FrameBg] = ImVec4(1.00f, 1.00f, 0.99f, 1.00f);
	style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
	style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
	style.Colors[ImGuiCol_TitleBg] = ImVec4(0.86f, 0.86f, 0.86f, 0.99f);
	style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(1.00f, 1.00f, 1.00f, 0.51f);
	style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.82f, 0.82f, 0.82f, 1.00f);
	style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
	style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.98f, 0.98f, 0.98f, 0.53f);
	style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.69f, 0.69f, 0.69f, 0.80f);
	style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.49f, 0.49f, 0.49f, 0.80f);
	style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.49f, 0.49f, 0.49f, 1.00f);
	style.Colors[ImGuiCol_ComboBg] = ImVec4(0.86f, 0.86f, 0.86f, 0.99f);
	style.Colors[ImGuiCol_CheckMark] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.26f, 0.59f, 0.98f, 0.78f);
	style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	style.Colors[ImGuiCol_Button] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
	style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
	style.Colors[ImGuiCol_Header] = ImVec4(0.26f, 0.59f, 0.98f, 0.31f);
	style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
	style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	style.Colors[ImGuiCol_Column] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
	style.Colors[ImGuiCol_ColumnHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.78f);
	style.Colors[ImGuiCol_ColumnActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	style.Colors[ImGuiCol_ResizeGrip] = ImVec4(1.00f, 1.00f, 1.00f, 0.00f);
	style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
	style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
	style.Colors[ImGuiCol_CloseButton] = ImVec4(0.59f, 0.59f, 0.59f, 0.50f);
	style.Colors[ImGuiCol_CloseButtonHovered] = ImVec4(0.98f, 0.39f, 0.36f, 1.00f);
	style.Colors[ImGuiCol_CloseButtonActive] = ImVec4(0.98f, 0.39f, 0.36f, 1.00f);
	style.Colors[ImGuiCol_PlotLines] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
	style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
	style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
	style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
	style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
	style.Colors[ImGuiCol_TooltipBg] = ImVec4(1.00f, 1.00f, 1.00f, 0.94f);
	style.Colors[ImGuiCol_ModalWindowDarkening] = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);

	style.WindowFillAlphaDefault = 1.0f;
	style.Alpha = 1.0f;
	style.AntiAliasedShapes = true;
	style.AntiAliasedLines = true;
	style.FrameRounding = 4;
}

void Noxoscope::loadModels() {
	using namespace glm;

	GLfloat quadVertices[] = {
		// Format: vvvtt, v = vertex, t = tex
		-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
		-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
		 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
		 1.0f, -1.0f, 0.0f, 1.0f, 0.0f
	};

	// Setup quad VAO
	quadVao.regen();
	quadVbo.regen();
	glBindVertexArray(quadVao.handle);
	glBindBuffer(GL_ARRAY_BUFFER, quadVbo.handle);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), nullptr);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), reinterpret_cast<GLvoid*>(3 * sizeof(GLfloat)));

	models = std::vector<Model>();
	models.reserve(MAX_MODELS);
	entities = std::vector<Entity>();
	entities.reserve(MAX_ENTITIES);
	lights = std::vector<PointLight>();
	lights.reserve(MAX_LIGHTS);

	lights.push_back({vec3(0.0f, 6.0f, 0.0f), 40, 0.3f * PAPAYA_WHIP});
	mainLight = &lights.back();
	lights.push_back({vec3(0.0f, 2.4f, 3.0f), 6, RED});
	redLight = &lights.back();
	lights.push_back({vec3(0.0f, 2.4f, -3.0f), 6, BLUE});
	blueLight = &lights.back();

	lights.push_back({vec3(-4.45f, 1.17f, -1.45f), 2.0f, YELLOW});
	lights.push_back({vec3(-4.45f, 1.17f, 1.45f), 2.0f, YELLOW});

	lights.push_back({vec3(4.395f, 1.17f, -1.45f), 2.0f, YELLOW});
	lights.push_back({vec3(4.395f, 1.17f, 1.45f), 2.0f, YELLOW});

	lights.push_back({vec3(9.8f, 0.3f, 0.0f), 7.0f, WHITE});
	lights.push_back({vec3(-9.8f, 0.3f, 0.0f), 7.0f, WHITE});

	lights.push_back({vec3(-5.75f, 1.09f, -0.18f), 1.5f, WHITE});

	auto& cornell = addModel("assets/models/cornell-box/CornellBox-Noxoscope.obj");
	addEntity(cornell, translate(vec3(0.0f, 0.047f, 0.0f)) * yawPitchRoll(radians(90.0f), 0.0f, 0.0f) * scale(vec3(0.5f)));

	auto& land = addModel("assets/models/groundplane/plane.obj", {GL_NEAREST, GL_NEAREST, 12000.0f});
	addEntity(land, translate(vec3(0.0f, 0.0f, 0.0f)) * scale(vec3(6000.0f)));

	auto& sponzaPlane = addModel("assets/models/groundplane/sponzaplane.obj", {GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, 40.0f});
	sponzaPlane.setReflectiveness(1.0f);
	addEntity(sponzaPlane, translate(vec3(0.0f, 0.045f, 0.0f)) * scale(vec3(40.0f)));

	tempSphere = &addModel("assets/models/sphere/sphere-lowpoly.obj");

	auto& sphere = addModel("assets/models/sphere/sphere.obj");
	sphere.setDiffuseColor(WHITE);
	sphere.setSpecular(1.0f);
	addEntity(sphere, translate(vec3(2.8f, 0.0f, 0.0f)));

	auto& normalMapCube = addModel("assets/models/normalmap-test-cube/normalmap-test-cube.obj");
	sphere.setDiffuseColor(WHITE);
	sphere.setSpecular(1.0f);
	addEntity(normalMapCube, translate(vec3(8.0f, 0.0f, 0.0f)));

	auto& reflCube = addModel("assets/models/cube/cube.obj");
	reflCube.setReflectiveness(1.0f);
	addEntity(reflCube, translate(vec3(5.0f, 0.0f, 0.0f)));

	auto& reflSphere = addModel("assets/models/sphere/sphere.obj");
	reflSphere.setReflectiveness(1.0f);
	addEntity(reflSphere, translate(vec3(6.5f, 0.0f, 0.0f)));

#ifdef NDEBUG
	auto& skydome = addModel("assets/models/skydome/linkeltje_skydome_linkeltje_2.obj");
	addEntity(skydome, translate(vec3(0.0f, -210.0f, 0.0f)) * scale(vec3(270.0f)));

	auto& sponza = addModel("assets/models/sponza/sponza.obj");
	sponza.setDiffuseColor(WHITE);
	addEntity(sponza, translate(vec3(0.0f, 0.03f, 0.0f)) * scale(vec3(0.008f)) * translate(vec3(62.5f, 0.0f, 38.0f)));

	auto& dragonModel = addModel("assets/models/stanford-dragon/dragon.obj");
	dragonModel.setDiffuseColor(1.1f * DARK_RED);
	dragonModel.setSpecular(1.0f);
	addEntity(dragonModel, translate(vec3(-7.0f, 0.0f, 0.0f)) * yawPitchRoll(radians(180.0f), 0.0f, 0.0f) * scale(vec3(1.0f)) * translate(vec3(0.0f, 0.0f, 0.0f)));
#endif
}

Model& Noxoscope::addModel(const char* path) {
	ModelProps props;
	return addModel(path, props);
}

Model& Noxoscope::addModel(const char* path, ModelProps props) {
	assert(modelCount < MAX_MODELS);
	models.emplace_back(baseDirRelative(path).c_str(), props, loadedTextures);
	return models[modelCount++];
}

Model& Noxoscope::addModelCopy(const Model& model) {
	assert(modelCount < MAX_MODELS);
	models.push_back(model);
	return models[modelCount++];
}

Entity& Noxoscope::addEntity(Model& model, glm::mat4 modelMatrix) {
	assert(modelCount < MAX_ENTITIES);
	entities.emplace_back(&model, modelMatrix);
	return entities[entityCount++];
}

void Noxoscope::getSize(int* w, int* h) const {
	SDL_GetWindowSize(mainwindow, w, h);
}

void Noxoscope::onResize() {
	int w, h;
	getSize(&w, &h);
	debug("Window size: {}x{}", w, h);
	reshape(w, h);
}

void Noxoscope::reshape(int width, int height) {
	this->width = width;
	this->height = height;

	internalWidth = std::max(1, int(round(internalResolutionScale * width)));
	internalHeight = std::max(1, int(round(internalResolutionScale * height)));

	ssaoWidth = int(round(ssaoResolutionScale * internalWidth));
	ssaoHeight = int(round(ssaoResolutionScale * internalHeight));
	ssrWidth = int(round(ssrResolutionScale * internalWidth));
	ssrHeight = int(round(ssrResolutionScale * internalHeight));

	glViewport(0, 0, width, height);
	aspect = float(width) / float(height);
	reloadBuffers();
}

void Noxoscope::enableDesktopFullscreen() const {
	SDL_DisplayMode mode;
	auto index = SDL_GetWindowDisplayIndex(mainwindow);
	SDL_GetDesktopDisplayMode(index, &mode);
	SDL_SetWindowDisplayMode(mainwindow, &mode);
	SDL_SetWindowFullscreen(mainwindow, SDL_WINDOW_FULLSCREEN);
}

void Noxoscope::toggleGui() {
	showGui = !showGui;
}

void Noxoscope::toggleMouseTrap() {
	SDL_SetRelativeMouseMode(SDL_GetRelativeMouseMode() ? SDL_FALSE : SDL_TRUE);
}

void Noxoscope::toggleFullscreen() const {
	auto flags = SDL_GetWindowFlags(mainwindow);
	if (!(flags & SDL_WINDOW_FULLSCREEN)) {
		enableDesktopFullscreen();
	} else {
		SDL_SetWindowFullscreen(mainwindow, 0);
	}
}

void Noxoscope::toggleVSync() {
	if (SDL_GL_GetSwapInterval()) {
		SDL_GL_SetSwapInterval(0);
	} else {
		SDL_GL_SetSwapInterval(1);
	}
}

void Noxoscope::onKeyPress(SDL_Keysym keysym) {
	switch (keysym.sym) {
	case SDLK_ESCAPE:
		quit = true;
		break;
	case SDLK_v:
		toggleVSync();
		break;
	case SDLK_h:
		toggleGui();
		break;
	case SDLK_t:
		toggleMouseTrap();
		break;
	case SDLK_r:
		addLightAtPlayer();
		break;
	case SDLK_f:
		fallbackRender = !fallbackRender;
		break;
	case SDLK_F11:
		toggleFullscreen();
		break;
	}
}

void Noxoscope::update(float fDiff) {
	using namespace glm;

	auto keystate = SDL_GetKeyboardState(nullptr);
	auto moveFactor = 3.2f;
	auto rotFactor = 2.0f;
	auto boostFactor = 5.0f;

	vec3 camMove = NULL_VECTOR;

	float yawRot = 0;
	float pitchRot = 0;

	if (keystate[SDL_SCANCODE_LSHIFT]) {
		moveFactor *= boostFactor;
	}
	if (keystate[SDL_SCANCODE_Z]) {
		moveFactor *= 0.45f;
		rotFactor *= 0.45f;
	}
	if (keystate[SDL_SCANCODE_W]) {
		camMove += cameraDirection;
	}
	if (keystate[SDL_SCANCODE_S]) {
		camMove -= cameraDirection;
	}
	if (keystate[SDL_SCANCODE_A]) {
		camMove += normalize(cross(UNIT_Y, cameraDirection));
	}
	if (keystate[SDL_SCANCODE_D]) {
		camMove -= normalize(cross(UNIT_Y, cameraDirection));
	}
	if (keystate[SDL_SCANCODE_E]) {
		camMove += UNIT_Y;
	}
	if (keystate[SDL_SCANCODE_Q]) {
		camMove -= UNIT_Y;
	}
	if (length(camMove) > 0.0f) {
		cameraPosition += fDiff * moveFactor * normalize(camMove);
	}

	if (keystate[SDL_SCANCODE_UP]) {
		pitchRot += rotFactor;
	}
	if (keystate[SDL_SCANCODE_DOWN]) {
		pitchRot += -rotFactor;
	}
	if (keystate[SDL_SCANCODE_LEFT]) {
		yawRot += rotFactor;
	}
	if (keystate[SDL_SCANCODE_RIGHT]) {
		yawRot += -rotFactor;
	}

	if (SDL_GetRelativeMouseMode()) {
		int relX;
		int relY;
		SDL_GetRelativeMouseState(&relX, &relY);
		float fRelX = int(relX);
		float fRelY = int(relY);
		yawRot += -0.05f * rotFactor * fRelX;
		pitchRot -= 0.05f * rotFactor * fRelY;
	}

	cameraDirection = cameraDirection / length(cameraDirection);
	float yaw = atan2(cameraDirection.z, cameraDirection.x);
	float pitch = acos(cameraDirection.y);
	pitch -= pitchRot * fDiff;
	pitch = clamp(pitch, 0.01f, PI_F - 0.01f);

	cameraDirection.x = sin(pitch) * cos(yaw);
	cameraDirection.y = cos(pitch);
	cameraDirection.z = sin(pitch) * sin(yaw);

	cameraDirection = rotate(cameraDirection, yawRot * fDiff, UNIT_Y);

	SDL_Event event;

	redLight->position = rotateY(redLight->position, fDiff * 0.3f);
	blueLight->position = rotateY(blueLight->position, fDiff * 0.3f);

	while (SDL_PollEvent(&event)) {
		if (showGui) ImGui_ImplSdlGL3_ProcessEvent(&event);

		switch (event.type) {
		case SDL_QUIT:
			quit = true;
			break;
		case SDL_WINDOWEVENT:
			switch (event.window.event) {
			case SDL_WINDOWEVENT_SIZE_CHANGED:
				onResize();
				break;
			}
			break;
		case SDL_KEYDOWN:
			onKeyPress(event.key.keysym);
			break;
		}
	}
	if (rotModel) {
		rotModel->modelMatrix = rotModel->modelMatrix * rotate(fDiff, UNIT_Y);
	}
}

void Noxoscope::onSecondPassed(void) {
	if (liveShaderReload) {
		reloadShaders();
	}

	float frameTime = frameTimeAccumulator / numFrames;
	float deviation = deviationAccumulator / numFrames;
	float frameRate = 1.0f / frameTime;

	lastOutFrametime = frameTime * 1000.0f;
	lastOutFramerate = frameRate;
	lastOutSD = deviation * 1000.0f;
	lastOutMaxSD = maxDeviation * 1000.0f;
	lastOutSDPerTime = 100.0f * deviation / frameTime;
	lastOutMaxSDPerTime = 100.0f * maxDeviation / frameTime;

	auto CONSOLE_DEBUG_PRINT_TEMPLATE =
R"(Pos                : {}
Dir                : {}
Frametime          : {:.3f} ms
Framerate          : {:.3f} FPS
Standard deviation : {:.3f} ms
Max deviation      : {:.3f} ms
SD / frametime     : {:.3f}%
MD / frametime     : {:.3f}%)";

	debug(CONSOLE_DEBUG_PRINT_TEMPLATE,
		to_string(cameraPosition).c_str(),
		to_string(cameraDirection).c_str(),
		lastOutFrametime,
		lastOutFramerate,
		lastOutSD,
		lastOutMaxSD,
		lastOutSDPerTime,
		lastOutMaxSDPerTime
	);
	debug("");

	auto STATISTICS_WINDOW_TEMPLATE =
R"(Pos                 : {}
Dir                 : {}
Frametime           : {:.3f} ms
Framerate           : {:.3f} FPS
Standard deviation  : {:.3f} ms
Max deviation       : {:.3f} ms
SD / frametime      : {:.3f}%
MD / frametime      : {:.3f}%
Resolution          : {}x{}
Internal resolution : {}x{}
SDL Swapinterval    : {})";

	cachedStatisticsWindowText = fmt::format(STATISTICS_WINDOW_TEMPLATE,
		to_string(cameraPosition).c_str(),
		to_string(cameraDirection).c_str(),
		lastOutFrametime,
		lastOutFramerate,
		lastOutSD,
		lastOutMaxSD,
		lastOutSDPerTime,
		lastOutMaxSDPerTime,
		width,
		height,
		internalWidth,
		internalHeight,
		SDL_GL_GetSwapInterval());

	frameTimeAccumulator = 0;
	deviationAccumulator = 0;
	numFrames = 0;
	lastFrametime = frameTime;
	lastDeviation = deviation;
	maxDeviation = 0;
}

void Noxoscope::addLightAtPlayer() {
	lights.push_back({cameraPosition + 2.0f * cameraDirection, 2, WHITE});
}

//===----------------------------------------------------------------------===//
// Rendering
//===----------------------------------------------------------------------===//

void Noxoscope::render() {
	using namespace glm;

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

	viewMatrix = lookAt(cameraPosition, cameraPosition + cameraDirection, UNIT_Y);
	projectionMatrix = perspective(70.0f, aspect, near, far);
	inverseProjection = inverse(projectionMatrix);

	if (!fallbackRender) {
		deferredRender();
	} else {
		forwardRender();
	}
	lastProjectionMatrix = projectionMatrix;
	lastViewMatrix = viewMatrix;
}

void Noxoscope::forwardRender() {
	mainForwardShader.use();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUniformMatrix4fv(mainForwardShader[UNIFORM_VIEW_MATRIX], 1, GL_FALSE, value_ptr(viewMatrix));
	glUniformMatrix4fv(mainForwardShader[UNIFORM_PROJECTION_MATRIX], 1, GL_FALSE, value_ptr(projectionMatrix));
	renderObjects(mainForwardShader);
}

void Noxoscope::deferredRender() {
	using namespace glm;

	// Render geometry to G-buffer

	glViewport(0, 0, internalWidth, internalHeight);
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer.handle);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearBufferfv(GL_COLOR, gBufferDepth.handle, value_ptr(WHITE));

	gBufferShader.use();
	glUniformMatrix4fv(gBufferShader[UNIFORM_VIEW_MATRIX], 1, GL_FALSE, value_ptr(viewMatrix));
	glUniformMatrix4fv(gBufferShader[UNIFORM_PROJECTION_MATRIX], 1, GL_FALSE, value_ptr(projectionMatrix));
	glUniform1f(gBufferShader["near"], near);
	glUniform1f(gBufferShader["far"], far);

	renderObjects(gBufferShader);

	if (ssao) {
		ssaoRender();
	}

	if (ssr) {
		ssrRender();
	}

	lightBufferRender();

	glBindFramebuffer(GL_FRAMEBUFFER, currentFinalFbo->handle);

	glClear(GL_COLOR_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);
	lightCombineShader.use();

	glUniform1f(lightCombineShader["screenWidth"], float(width));
	glUniform1f(lightCombineShader["screenHeight"], float(height));
	glUniform1i(lightCombineShader["ssao"], ssao ? GL_TRUE : GL_FALSE);
	glUniform1i(lightCombineShader["ssr"], ssr ? GL_TRUE : GL_FALSE);

	auto gMembers = {
		std::make_tuple(gBufferDepth.handle, "gPosition"),
		std::make_tuple(gBufferNormal.handle, "gNormal"),
		std::make_tuple(gBufferDiffuse.handle, "gDiffuse"),
		std::make_tuple(lastFinalTexture->handle, "lastFrame"),
		std::make_tuple(gBufferSpecular.handle, "gSpecular"),
		std::make_tuple(ssaoBlurTexture.handle, "postSSAO"),
		std::make_tuple(ssrTexture.handle, "ssrTexture"),
		std::make_tuple(lightTexture.handle, "lightTex")
	};

	attachTextures(lightCombineShader, gMembers);

	glUniformMatrix4fv(lightCombineShader[UNIFORM_INVERSE_PROJECTION_MATRIX], 1, GL_FALSE, value_ptr(inverseProjection));
	glUniformMatrix4fv(lightCombineShader[UNIFORM_PROJECTION_MATRIX], 1, GL_FALSE, value_ptr(projectionMatrix));
	glUniform1i(lightCombineShader["showDebugBar"], showDebugBar);

	renderQuad();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	renderTextureShader.use();
	glViewport(0, 0, width, height);

	glClear(GL_COLOR_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, currentFinalTexture->handle);

	glUniform1i(renderTextureShader["tex"], 0);
	renderQuad();
	std::swap(currentFinalFbo, lastFinalFbo);
	std::swap(currentFinalTexture, lastFinalTexture);
}

void Noxoscope::lightBufferRender() {
	using namespace glm;

	// Do shading calculation on G-buffer content
	glBindFramebuffer(GL_FRAMEBUFFER, lightFbo.handle);
	glViewport(0, 0, internalWidth, internalHeight);
	glClear(GL_COLOR_BUFFER_BIT);
	glEnable(GL_STENCIL_TEST);

	// Render lights using a stencil culling algorithm, using low-polygon spheres
	for (auto& light : lights) {
		simpleShader.use();

		glDisable(GL_CULL_FACE);
		glEnable(GL_DEPTH_TEST);
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		glDepthMask(GL_FALSE);
		glStencilFunc(GL_ALWAYS, 0, 0);
		glStencilOpSeparate(GL_BACK, GL_KEEP, GL_INCR_WRAP, GL_KEEP);
		glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_DECR_WRAP, GL_KEEP);
		glStencilMask(0xFF);

		glClear(GL_STENCIL_BUFFER_BIT);

		glUniformMatrix4fv(simpleShader[UNIFORM_MODEL_MATRIX], 1, GL_FALSE, value_ptr(translate(light.position) * scale(vec3(2 * light.radius))));
		glUniformMatrix4fv(simpleShader[UNIFORM_VIEW_MATRIX], 1, GL_FALSE, value_ptr(viewMatrix));
		glUniformMatrix4fv(simpleShader[UNIFORM_PROJECTION_MATRIX], 1, GL_FALSE, value_ptr(projectionMatrix));

		tempSphere->render(simpleShader);

		// Do light calculations using the stencil mask
		lightShader.use();

		glEnable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		glDepthMask(GL_TRUE);
		glStencilFunc(GL_NOTEQUAL, 0, 0xFF);
		glStencilMask(0x00);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_ONE, GL_ONE);

		auto lightInputTextures = {
			std::make_tuple(gBufferDepth.handle, "gPosition"),
			std::make_tuple(gBufferNormalMappedNormal.handle, "gNormal"),
			std::make_tuple(gBufferDiffuse.handle, "gDiffuse"),
			std::make_tuple(gBufferSpecular.handle, "gSpecular")
		};
		attachTextures(lightShader, lightInputTextures);

		glUniform1i(lightShader["stencilDebugRender"], stencilDebugRender ? GL_TRUE : GL_FALSE);

		auto vsLightPos = vec3(viewMatrix * vec4(light.position, 1));
		glUniform3fv(lightShader["lightPos"], 1, value_ptr(vsLightPos));
		glUniform3fv(lightShader["lightColor"], 1, value_ptr(light.color));

		// Determine an attenuation factor, for very simple linear attenuation
		glUniform1f(lightShader["lightStrength"], -1.0f / light.radius);
		renderQuad();

		glDisable(GL_BLEND);
		glDisable(GL_DEPTH_TEST);
		glDepthMask(GL_TRUE); // Don't write to depth buffer
	}
	glDisable(GL_STENCIL_TEST);
}

void Noxoscope::ssaoRender() {
	// use G-buffer to render SSAO texture
	glBindFramebuffer(GL_FRAMEBUFFER, ssaoFbo.handle);
	glClear(GL_COLOR_BUFFER_BIT);
	glViewport(0, 0, ssaoWidth, ssaoHeight);

	ssaoShader.use();

	glUniform1i(ssaoShader[UNIFORM_WIDTH], ssaoWidth);
	glUniform1i(ssaoShader[UNIFORM_HEIGHT], ssaoHeight); {
		GLenum texCount = 0;
		auto textures = {
			std::make_tuple(&gBufferDepth, "gPosition"),
			std::make_tuple(&gBufferNormal, "gNormal"),
			std::make_tuple(&noiseTexture, "texNoise")
		};

		for (auto& sampleTex : textures) {
			GLTexture* tex;
			const char* texName;
			std::tie(tex, texName) = sampleTex;
			glActiveTexture(GL_TEXTURE0 + texCount);
			glBindTexture(GL_TEXTURE_2D, tex->handle);
			glUniform1i(ssaoShader[texName], texCount);
			texCount++;
		}
	}
	glUniform3fv(ssaoShader[UNIFORM_SAMPLES], ssaoKernel.size(), value_ptr(ssaoKernel.front()));
	glUniformMatrix4fv(ssaoShader[UNIFORM_PROJECTION_MATRIX], 1, GL_FALSE, value_ptr(projectionMatrix));
	renderQuad();

	// Blur result
	glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFbo.handle);

	blurShader.use();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, ssaoBuffer.handle);
	glUniform1i(lightCombineShader["tex"], 0);

	renderQuad();
}

void Noxoscope::ssrRender() {
	glBindFramebuffer(GL_FRAMEBUFFER, ssrFbo.handle);
	glViewport(0, 0, ssrWidth, ssrHeight);

	ssrShader.use();
	glUniformMatrix4fv(ssrShader[UNIFORM_VIEW_MATRIX], 1, GL_FALSE, value_ptr(viewMatrix));
	glUniformMatrix4fv(ssrShader[UNIFORM_PROJECTION_MATRIX], 1, GL_FALSE, value_ptr(projectionMatrix));

	auto gMembers = {
		std::make_tuple(gBufferDepth.handle, "gPosition"),
		std::make_tuple(gBufferNormalMappedNormal.handle, "gNormal"),
		std::make_tuple(lastFinalTexture->handle, "lastFrame"),
		std::make_tuple(gBufferSpecular.handle, "gSpecular"),
	};

	attachTextures(ssrShader, gMembers);
	renderQuad();
}

void Noxoscope::renderObjects(const ShaderProgram& shaderProgram) {
	using namespace glm;
	for (auto& e : entities) {
		e.render(shaderProgram);
	}

	if (debugRenderLightSpheres) {
		for (auto& light : lights) {
			tempSphere->setSpecular(1.0f);
			tempSphere->setDiffuseColor(light.color);
			if (debugSpheresFullSize) {
				glUniformMatrix4fv(shaderProgram[UNIFORM_MODEL_MATRIX], 1, GL_FALSE, value_ptr(translate(light.position) * scale(vec3(2 * light.radius))));
				tempSphere->render(shaderProgram);
			}
			glUniformMatrix4fv(shaderProgram[UNIFORM_MODEL_MATRIX], 1, GL_FALSE, value_ptr(translate(light.position) * scale(vec3(0.05f * log(light.radius + 1)))));

			tempSphere->render(shaderProgram);
		}
	}
}

void Noxoscope::renderQuad() const {
	glBindVertexArray(quadVao.handle);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}

void Noxoscope::renderGui() {
	using namespace ImGui;

	SetNextWindowPos(ImVec2(0, 0), ImGuiSetCond_FirstUseEver);
	SetNextWindowSize(ImVec2(500, 255), ImGuiSetCond_FirstUseEver);
	Begin("Frame Statistics", nullptr, ImGuiWindowFlags_ShowBorders);
	PushFont(monoFont);
	Text("%s", cachedStatisticsWindowText.c_str());
	PopFont();

	End();

	SetNextWindowPos(ImVec2(0, 255), ImGuiSetCond_FirstUseEver);
	SetNextWindowSize(ImVec2(500, 350), ImGuiSetCond_FirstUseEver);
	SetNextWindowCollapsed(true, ImGuiSetCond_FirstUseEver);

	Begin("Config", nullptr, ImGuiWindowFlags_ShowBorders);

	auto flags = SDL_GetWindowFlags(mainwindow);
	bool fullscreen = (flags & SDL_WINDOW_FULLSCREEN) != 0;
	if (Checkbox("Fullscreen", &fullscreen)) {
		toggleFullscreen();
	}

	bool vsync = SDL_GL_GetSwapInterval() != 0;
	if (Checkbox("VSync", &vsync)) {
		toggleVSync();
	}

	Checkbox("Framerate cap", &frameCap);
	InputFloat("Cap framerate", &targetFramerate);
	Checkbox("Framerate cap sleep", &frameCapSleep);

	if (SliderFloat("Internal scale", &internalResolutionScale, 0.0f, 5.0f)) {
		onResize();
	}
	Checkbox("SSR", &ssr);
	Checkbox("SSAO", &ssao);
	Checkbox("Fallback render", &fallbackRender);

	bool showGuiTemp = this->showGui;
	if (Checkbox("Show GUI", &showGuiTemp)) {
		toggleGui();
	}

	Checkbox("Live shader reload", &liveShaderReload);
	if (Button("Reload shaders")) {
		reloadShaders();
	}

	Checkbox("Show light spheres", &debugRenderLightSpheres);
	Checkbox("Show full volume light spheres", &debugSpheresFullSize);
	Checkbox("Light stencil debug render", &stencilDebugRender);

	Checkbox("Show debug bar", &showDebugBar);
	Checkbox("Show default GUI windows", &extraImguiDebug);

	if (Button("Add light##top")) {
		addLightAtPlayer();
	}

	ColorEditMode(ImGuiColorEditMode_HSV);

	int lightnum = 1;

	auto i = begin(lights);
	while (i != end(lights)) {
		auto& light = *i;
		Text("%s", fmt::format("Light {}", lightnum).c_str());
		ColorEdit3(fmt::format("Light color##l{}", lightnum).c_str(), value_ptr(light.color));
		SliderFloat(fmt::format("Light radius##l{}", lightnum).c_str(), &light.radius, 0, 100);
		DragFloat3(fmt::format("Light position##l{}", lightnum).c_str(), value_ptr(light.position), 0.05f);
		if (Button(fmt::format("Remove light##l{}", lightnum).c_str())) {
			i = lights.erase(i);
		} else {
			++i;
		}
		lightnum++;
	}

	if (Button("Add light##end")) {
		addLightAtPlayer();
	}

	End();

	if (extraImguiDebug) {
		ShowTestWindow();
		ShowMetricsWindow();
		ShowUserGuide();
		ShowStyleEditor();
	}

	Render();
}

void GLAPIENTRY onDebugEvent(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
	const char* sourceStr;
	const char* typeStr;
	const char* severityStr;

	const auto NVIDIA_ARRAY_BUFFER_MESSAGE = 131185;
	// Lower severity of shader "being recompiled based on GL state"
	const auto MESA_SHADER_RECOMPILED = 131218;
	switch (id) {
	case NVIDIA_ARRAY_BUFFER_MESSAGE: // ignore array buffer message
		return;
	case MESA_SHADER_RECOMPILED:
		severity = GL_DEBUG_SEVERITY_LOW_ARB;
		break;
	}

	switch (source) {
	case GL_DEBUG_SOURCE_API_ARB: sourceStr = "API";
		break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM_ARB: sourceStr = "WINDOW_SYSTEM";
		break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER_ARB: sourceStr = "SHADER_COMPILER";
		break;
	case GL_DEBUG_SOURCE_THIRD_PARTY_ARB: sourceStr = "THIRD_PARTY";
		break;
	case GL_DEBUG_SOURCE_APPLICATION_ARB: sourceStr = "APPLICATION";
		break;
	case GL_DEBUG_SOURCE_OTHER_ARB: sourceStr = "OTHER";
		break;
	default: sourceStr = "UNKNOWN";
		break;
	}

	switch (type) {
	case GL_DEBUG_TYPE_ERROR_ARB: typeStr = "ERROR";
		break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_ARB: typeStr = "DEPRECATED_BEHAVIOR";
		break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB: typeStr = "UNDEFINED_BEHAVIOR";
		break;
	case GL_DEBUG_TYPE_PORTABILITY_ARB: typeStr = "PORTABILITY";
		break;
	case GL_DEBUG_TYPE_PERFORMANCE_ARB: typeStr = "PERFORMANCE";
		break;
	case GL_DEBUG_TYPE_OTHER_ARB: typeStr = "OTHER";
		break;
	default: typeStr = "UNKNOWN";
		break;
	}

	switch (severity) {
	case GL_DEBUG_SEVERITY_HIGH_ARB: severityStr = "HIGH";
		break;
	case GL_DEBUG_SEVERITY_MEDIUM_ARB: severityStr = "MEDIUM";
		break;
	case GL_DEBUG_SEVERITY_LOW_ARB: severityStr = "LOW";
		break;
	default: severityStr = "UNKNOWN";
		break;
	}

	if (severity == GL_DEBUG_SEVERITY_MEDIUM_ARB || severity == GL_DEBUG_SEVERITY_HIGH_ARB) {
		errorLog("OpenGL error callback, source: {}, type: {}, id: {}, severity: {}, message:\n{}\n",
			sourceStr, typeStr, id, severityStr, message);
	} else {
		log("OpenGL error callback, source: {}, type: {}, id: {}, severity: {}, message:\n{}\n", sourceStr, typeStr, id, severityStr, message);
	}
}
