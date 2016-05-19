//===----------------------------------------------------------------------===//
//
// This file is part of the Noxoscope project
//
// Copyright (c) 2016 Niklas Helmertz
//
//===----------------------------------------------------------------------===//

#include <string>
#include <vector>
#include <algorithm>

#include <GL/glew.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <stb_image.h>
#include <glm/glm.hpp>

#include "Model.h"
#include "ShaderProgram.h"
#include "Logging.h"
#include "FileTools.h"

Model::Model(const char* path, std::vector<MeshTexture>& loadedTextures) : Model(path, ModelProps(), loadedTextures) {}

Model::Model(const char* path, ModelProps modelProps, std::vector<MeshTexture>& loadedTextures)
	: modelProps(modelProps),
	  loadedTextures(loadedTextures) {
	this->loadModel(path);
}

ModelProps::ModelProps() : magFilter(GL_LINEAR), minFilter(GL_LINEAR_MIPMAP_LINEAR), texRepeatFactor(1) {}

ModelProps::ModelProps(GLint magFilter, GLint minFilter, float texRepeatFactor) : magFilter(magFilter), minFilter(minFilter), texRepeatFactor(texRepeatFactor) {}

void Model::loadModel(std::string path) {
	Assimp::Importer import;
	auto scene = import.ReadFile(path,
		aiProcess_GenNormals |
		//aiProcess_GenSmoothNormals |
		aiProcess_CalcTangentSpace |
		aiProcess_Triangulate |
		//aiProcess_OptimizeMeshes |
		//aiProcessPreset_TargetRealtime_MaxQuality |
		//aiProcess_PreTransformVertices |
		//aiProcess_JoinIdenticalVertices |
		//aiProcess_SortByPType |
		0
	);

	if (!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		fatalError("Assimp error while loading \"{}\":\n{}", path.c_str(), import.GetErrorString());
		return;
	}
	this->directory = path.substr(0, path.find_last_of('/'));

	this->processNode(scene->mRootNode, scene);
}

void Model::processNode(aiNode* node, const aiScene* scene) {
	for (GLuint i = 0; i < node->mNumMeshes; i++) {
		auto mesh = scene->mMeshes[node->mMeshes[i]];
		this->meshes.push_back(this->processMesh(node, mesh, scene));
	}
	for (GLuint i = 0; i < node->mNumChildren; i++) {
		this->processNode(node->mChildren[i], scene);
	}
}

Mesh Model::processMesh(aiNode* node, aiMesh* aiMesh, const aiScene* scene) const {
	using namespace glm;

	std::vector<Vertex> vertices;
	std::vector<GLuint> indices;
	MeshTexture diffuseTex;
	MeshTexture specTex;
	MeshTexture normalTex;

	auto toGlm = [](aiVector3D& aiVec) {
		vec3 v;
		v.x = aiVec.x;
		v.y = aiVec.y;
		v.z = aiVec.z;
		return v;
	};

	for (GLuint i = 0; i < aiMesh->mNumVertices; i++) {

		Vertex vertex;
		vertex.position = toGlm(aiMesh->mVertices[i]);
		vertex.normal = vec3(0);
		if (aiMesh->HasNormals()) {
			vertex.normal = toGlm(aiMesh->mNormals[i]);
		}

		vertex.tangent = vec3(0);
		vertex.bitangent = vec3(0);
		if (aiMesh->HasTangentsAndBitangents()) {
			vertex.tangent = toGlm(aiMesh->mTangents[i]);
			vertex.bitangent = toGlm(aiMesh->mBitangents[i]);
		}

		if (aiMesh->mTextureCoords[0]) {
			vec2 vec;
			vec.x = aiMesh->mTextureCoords[0][i].x;
			vec.y = aiMesh->mTextureCoords[0][i].y;
			vertex.texCoords = vec;
		} else {
			vertex.texCoords = vec2(0.0f, 0.0f);
		}
		vertices.push_back(vertex);
	}

	for (GLuint i = 0; i < aiMesh->mNumFaces; i++) {
		auto face = aiMesh->mFaces[i];
		for (GLuint j = 0; j < face.mNumIndices; j++) {
			indices.push_back(face.mIndices[j]);
		}
	}

	vec4 color;
	float specular = 0.2f;

	aiMaterial* mat = scene->mMaterials[aiMesh->mMaterialIndex];

	aiColor4D aiColor;
	mat->Get(AI_MATKEY_COLOR_DIFFUSE, aiColor);
	color = vec4(aiColor.r, aiColor.g, aiColor.b, aiColor.a);

	float opac;
	mat->Get(AI_MATKEY_OPACITY, opac);
	color.a *= opac;

	aiColor4D aiSpec;
	if (mat->Get(AI_MATKEY_COLOR_SPECULAR, aiSpec)) {
		specular = length(vec3(aiSpec.r, aiSpec.g, aiSpec.b));
	}

	if (mat->Get(AI_MATKEY_SHININESS, aiSpec)) {
		specular = length(vec3(aiSpec.r, aiSpec.g, aiSpec.b));
	}

	loadMaterialTextures(mat, aiTextureType_DIFFUSE, &diffuseTex);
	loadMaterialTextures(mat, aiTextureType_SPECULAR, &specTex);
	loadMaterialTextures(mat, aiTextureType_NORMALS, &normalTex);
	if (normalTex.glObject == nullptr) {
		loadMaterialTextures(mat, aiTextureType_HEIGHT, &normalTex);
	}

	return {vertices, indices, diffuseTex, specTex, normalTex, color, specular};
}

void Model::loadMaterialTextures(aiMaterial* mat, aiTextureType type, MeshTexture* texture) const {
	auto typeCount = mat->GetTextureCount(type);
	if (typeCount < 1) {
		*texture = {aiString(""), nullptr};
		return;
	}

	aiString path;
	// Just pick the first one in case of several matches
	mat->GetTexture(type, 0, &path);

	// Check if already loaded
	for (GLuint j = 0; j < loadedTextures.size(); j++) {
		if (loadedTextures[j].path == path) {
			*texture = loadedTextures[j];
			return;
		}
	}

	debug("Loading: {}", path.C_Str());
	auto loadRes = loadTexture(path.C_Str(), this->directory.c_str(), modelProps);
	texture->glObject = std::make_shared<GLTexture>(loadRes);
	texture->path = path;
	loadedTextures.push_back(*texture);
}

GLuint loadTexture(const std::string& relPath, const std::string& dir, ModelProps modelProps) {
	int w, h, n;

	auto fixedRelPath = relPath;
	auto fixedDir = dir;
	fixOSPath(fixedRelPath);
	fixOSPath(fixedDir);

	std::string fullPath = fmt::format("{}{}{}", fixedDir, PATH_SEP, fixedRelPath);

	stbi_set_flip_vertically_on_load(true);
	auto data = stbi_load(fullPath.c_str(), &w, &h, &n, 0);
	if (data == nullptr) {
		errorLog("Failed loading texture {}", relPath.c_str());
		return 0;
	}

	GLuint textureID;
	glGenTextures(1, &textureID);

	glBindTexture(GL_TEXTURE_2D, textureID);

	GLenum loadFormat;
	GLint internalFormat;
	switch (n) {
	case 1:
		loadFormat = GL_LUMINANCE;
		internalFormat = GL_LUMINANCE;
		break;
	case 2:
		loadFormat = GL_LUMINANCE_ALPHA;
		internalFormat = GL_LUMINANCE_ALPHA;
		break;
	case 3:
		internalFormat = GL_RGB;
		loadFormat = GL_RGB;
		break;
	case 4:
		internalFormat = GL_RGBA;
		loadFormat = GL_RGBA;
		break;
	default:
		errorLog("Unrecognized number of channels per pixel ({}) in {}", n, relPath.c_str());
		return 0;
	}

	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, w, h, 0, loadFormat, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, modelProps.magFilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, modelProps.minFilter);
	if (GLEW_EXT_texture_filter_anisotropic) {
		float aniso;
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &aniso);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso);
	}
	return textureID;
}

void Model::setDiffuseColor(glm::vec3 color) {
	for (auto& m : this->meshes) {
		m.color = glm::vec4(color, 1.0f);
	}
}

void Model::setSpecular(float specular) {
	for (auto& m : this->meshes) {
		m.specular = specular;
	}
}

void Model::setReflectiveness(float reflectiveness) {
	for (auto& m : this->meshes) {
		m.reflectiveness = reflectiveness;
	}
}

void Model::render(const ShaderProgram& shader) {
	glUniform1f(glGetUniformLocation(shader.handle, "texRepeatFactor"), modelProps.texRepeatFactor);

	for (GLuint i = 0; i < this->meshes.size(); i++) {
		this->meshes[i].render(shader);
	}
}
