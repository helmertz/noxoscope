//===----------------------------------------------------------------------===//
//
// This file is part of the Noxoscope project
//
// Copyright (c) 2016 Niklas Helmertz
//
//===----------------------------------------------------------------------===//

#ifndef Model_H
#define Model_H

#include <string>
#include <vector>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <assimp/scene.h>

#include "ShaderProgram.h"
#include "Mesh.h"

struct ModelProps {
	GLint magFilter;
	GLint minFilter;
	float texRepeatFactor;
	ModelProps();
	ModelProps(GLint magFilter, GLint minFilter, float texRepeatFactor);
};

class Model {
public:
	Model(const char* path, std::vector<MeshTexture>& loadedTextures);
	Model(const char* path, ModelProps props, std::vector<MeshTexture>& loadedTextures);
	void render(const ShaderProgram& shader);
	void setDiffuseColor(glm::vec3 tvec3);
	void setSpecular(float x);
	void setReflectiveness(float x);
	std::vector<Mesh> meshes;
	std::string directory;
	std::string path;
private:
	ModelProps modelProps;
	std::vector<MeshTexture>& loadedTextures;
	void loadModel(std::string path);
	void processNode(aiNode* node, const aiScene* scene);
	Mesh processMesh(aiNode* node, aiMesh* mesh, const aiScene* scene) const;
	void loadMaterialTextures(aiMaterial* mat, aiTextureType type, MeshTexture* texture) const;
};

GLuint loadTexture(const std::string& relPath, const std::string& dir, ModelProps modelProps);

#endif // Model_H
