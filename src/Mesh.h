//===----------------------------------------------------------------------===//
//
// This file is part of the Noxoscope project
//
// Copyright (c) 2016 Niklas Helmertz
//
//===----------------------------------------------------------------------===//

#ifndef Mesh_H
#define Mesh_H

#include <vector>
#include <memory>

#include <GL/glew.h>
#include <assimp/Importer.hpp>
#include <glm/glm.hpp>

#include "ShaderProgram.h"
#include "GLObject.h"

struct Vertex {
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec3 tangent;
	glm::vec3 bitangent;
	glm::vec2 texCoords;
};

struct MeshTexture {
	aiString path;
	std::shared_ptr<GLTexture> glObject;
};

class Mesh {
public:
	std::vector<Vertex> vertices;
	std::vector<GLuint> indices;

	MeshTexture diffuseTexture;
	MeshTexture specularTexture;
	MeshTexture normalTexture;

	glm::vec4 color;
	float specular;
	float reflectiveness = 0;
	Mesh(const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices, MeshTexture diffuseTex, MeshTexture specTex, MeshTexture normalTexture, glm::vec4 color, float specular);
	void render(const ShaderProgram& shader);
private:
	GLuint vertexArray, vertexBuffer, elemBuffer;
	void setupMesh();
};

#endif // Mesh_H
