//===----------------------------------------------------------------------===//
//
// This file is part of the Noxoscope project
//
// Copyright (c) 2016 Niklas Helmertz
//
//===----------------------------------------------------------------------===//
//
// Declaration of the Noxoscope class.
//
//===----------------------------------------------------------------------===//

#ifndef Noxoscope_H
#define Noxoscope_H

#include <string>
#include <vector>
#include <chrono>

#include <SDL.h>
#include <glm/glm.hpp>

#include "Model.h"
#include "ShaderProgram.h"
#include "Entity.h"
#include "Light.h"
#include "GLObject.h"

/// Top-level class for the program.
///
/// This handles the game loop timing, top-level rendering calls and logic
/// updates. Many parts may be refactored out at a later time.
class Noxoscope {
public:
	Noxoscope(SDL_Window* window, SDL_GLContext context);
	~Noxoscope();

	// Forbid copy construction
	Noxoscope(const Noxoscope&) = delete;
	Noxoscope& operator=(const Noxoscope&) = delete;

	/// Start running the Noxoscope rendering by calling this.
	///
	/// The function will not return until the program has been exited.
	///
	/// \param mainwindow A handle to the SDL window to be used.
	///
	/// \param maincontext The SDL GL context to be used for Noxoscope.
	static void loadAndRun(SDL_Window* mainWindow, SDL_GLContext mainContext);

private:
	void onSecondPassed();
	void reloadBuffers();
	void loadModels();
	void initialize();
	void reshape(int width, int height);
	void getSize(int* w, int* h) const;
	void onResize();
	void enableDesktopFullscreen() const;
	void toggleGui();
	void toggleFullscreen() const;
	void onKeyPress(SDL_Keysym keysym);
	void update(float fDiff);
	void renderObjects(const ShaderProgram& shaderProgram);
	void ssrRender();
	void deferredRender();
	void lightBufferRender();
	void render();
	void addLightAtPlayer();
	void renderGui();
	void run();
	void reloadShaders();
	void renderQuad() const;
	void forwardRender();
	void ssaoRender();
	void setupImgui();
	Model& addModel(const char* path);
	Model& addModel(const char* path, ModelProps props);
	Model& addModelCopy(const Model& model);
	Entity& addEntity(Model& model, glm::mat4 modelMatrix);
	static void toggleVSync();
	static void toggleMouseTrap();

	// External dependencies
	SDL_Window* mainwindow;
	SDL_GLContext maincontext;

	// Simulation state
	bool quit = false;

	// Main data members
	std::vector<Entity> entities;
	std::vector<Model> models;
	std::vector<PointLight> lights;
	std::vector<MeshTexture> loadedTextures;
	size_t entityCount = 0;
	size_t modelCount = 0;
	Entity* rotModel = nullptr;
	PointLight* mainLight = nullptr;
	Model* tempSphere = nullptr;
	PointLight* redLight = nullptr;
	PointLight* blueLight = nullptr;
	glm::vec3 cameraPosition = glm::vec3(1.03f, 0.4f, 0);
	glm::vec3 cameraDirection = normalize(glm::vec3(0, 0.33f, 0) - cameraPosition);

	// Rendering data
	float near = 0.2f;
	float far = 10000.0f;
	float aspect = 1;
	int height = 0;
	int width = 0;
	int internalWidth = 0;
	int internalHeight = 0;
	int ssaoWidth = 0;
	int ssaoHeight = 0;
	int ssrWidth = 0;
	int ssrHeight = 0;
	ShaderProgram mainForwardShader;
	ShaderProgram gBufferShader;
	ShaderProgram renderTextureShader;
	ShaderProgram lightCombineShader;
	ShaderProgram ssrShader;
	ShaderProgram ssaoShader;
	ShaderProgram lightShader;
	ShaderProgram blurShader;
	ShaderProgram simpleShader;
	GLFramebuffer gBuffer;
	GLFramebuffer finalFbo1;
	GLFramebuffer finalFbo2;
	GLFramebuffer ssrFbo;
	GLFramebuffer ssaoBlurFbo;
	GLFramebuffer lightFbo;
	GLFramebuffer ssaoFbo;
	GLFramebuffer* currentFinalFbo = nullptr;
	GLFramebuffer* lastFinalFbo = nullptr;
	GLTexture gBufferDepth;
	GLTexture gBufferNormal;
	GLTexture gBufferDiffuse;
	GLTexture gBufferSpecular;
	GLTexture gBufferNormalMappedNormal;
	GLTexture finalTexture1;
	GLTexture finalTexture2;
	GLTexture ssaoBuffer;
	GLTexture noiseTexture;
	GLTexture ssaoBlurTexture;
	GLTexture ssrTexture;
	GLTexture lightTexture;
	GLTexture* currentFinalTexture = nullptr;
	GLTexture* lastFinalTexture = nullptr;
	GLRenderBuffer sharedDepthStencil;
	GLRenderBuffer rboDepth;
	GLVertexArray quadVao;
	GLBuffer quadVbo;
	std::vector<glm::vec3> ssaoKernel;
	std::vector<glm::vec3> ssaoNoise;
	glm::mat4 lastProjectionMatrix;
	glm::mat4 lastViewMatrix;
	glm::mat4 viewMatrix;
	glm::mat4 projectionMatrix;
	glm::mat4 inverseProjection;

	// GUI data
	struct ImFont* monoFont = nullptr;

	// Render settings
	bool showGui = true;
	bool extraImguiDebug = false;
	bool debugRenderLightSpheres = true;
	bool debugSpheresFullSize = false;
	bool showDebugBar = true;
	bool frameCap = false;
	bool frameCapSleep = false;
	bool fallbackRender = false;
	float targetFramerate = 60.0f;
	float ssaoResolutionScale = 1.0f;
	float ssrResolutionScale = 1.0f;
	float internalResolutionScale = 1.0f;
	bool ssr = false;
	bool ssao = false;
	bool liveShaderReload = true;
	bool stencilDebugRender = false;

	// Rendering statistics
	int numFrames = 0;
	float frameTimeAccumulator = 0.0f;
	float deviationAccumulator = 0.0f;
	float lastDeviation = 0.0f;
	float maxDeviation = 0.0f;
	float lastFrametime = 0.0f;
	float lastOutFrametime = 0.0f;
	float lastOutFramerate = 0.0f;
	float lastOutSD = 0.0f;
	float lastOutMaxSD = 0.0f;
	float lastOutSDPerTime = 0.0f;
	float lastOutMaxSDPerTime = 0.0f;
	std::chrono::high_resolution_clock::time_point startTime;
	std::chrono::high_resolution_clock::time_point lastRender;
	std::chrono::high_resolution_clock::time_point now;
	std::string cachedStatisticsWindowText;
};

void GLAPIENTRY onDebugEvent(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam);

#endif // Noxoscope_H
