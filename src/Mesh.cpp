//===----------------------------------------------------------------------===//
//
// This file is part of the Noxoscope project
//
// Copyright (c) 2016 Niklas Helmertz
//
//===----------------------------------------------------------------------===//

#include "Mesh.h"

#include <tuple>

#include <glm/gtc/type_ptr.hpp>

#include "Constants.h"

Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices, MeshTexture diffuseTexture, MeshTexture specularTexture, MeshTexture normalTexture, glm::vec4 color, float specular)
	: vertices{vertices},
	  indices{indices},
	  diffuseTexture(diffuseTexture),
	  specularTexture(specularTexture),
	  normalTexture(normalTexture),
	  color{color},
	  specular{specular} {
	this->setupMesh();
}

void Mesh::setupMesh() {
	glGenVertexArrays(1, &this->vertexArray);
	glGenBuffers(1, &this->vertexBuffer);
	glGenBuffers(1, &this->elemBuffer);

	glBindVertexArray(this->vertexArray);
	glBindBuffer(GL_ARRAY_BUFFER, this->vertexBuffer);

	glBufferData(GL_ARRAY_BUFFER, this->vertices.size() * sizeof(Vertex), &this->vertices[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->elemBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->indices.size() * sizeof(GLuint), &this->indices[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), nullptr);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<GLvoid*>(offsetof(Vertex, normal)));

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<GLvoid*>(offsetof(Vertex, tangent)));

	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<GLvoid*>(offsetof(Vertex, bitangent)));

	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<GLvoid*>(offsetof(Vertex, texCoords)));

	glBindVertexArray(0);
}

void Mesh::render(const ShaderProgram& shader) {
	using namespace glm;
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);

	GLenum texCount = 0;

	auto modelTex = {
		std::make_tuple(&diffuseTexture, UNIFORM_TEXTURE_DIFFUSE, UNIFORM_HAS_DIFFUSE_TEXTURE),
		std::make_tuple(&specularTexture, UNIFORM_TEXTURE_SPECULAR, UNIFORM_HAS_SPECULAR_TEXTURE),
		std::make_tuple(&normalTexture, UNIFORM_TEXTURE_NORMAL, UNIFORM_HAS_NORMAL_TEXTURE)
	};

	for (auto& texInfo : modelTex) {
		auto& tex = std::get<0>(texInfo);
		auto texUniformName = std::get<1>(texInfo);
		auto hasTexName = std::get<2>(texInfo);

		bool hasTex = tex->glObject != nullptr;
		glActiveTexture(GL_TEXTURE0 + texCount);

		if (hasTex) {
			glUniform1i(shader[texUniformName], texCount);
			glBindTexture(GL_TEXTURE_2D, tex->glObject->handle);
		}

		glUniform1i(shader[hasTexName], hasTex);
		texCount++;
	}

	glUniform4fv(shader[UNIFORM_COLOR_DIFFUSE], 1, value_ptr(color));
	glUniform1f(shader[UNIFORM_SPECULAR], specular);
	glUniform1f(shader[UNIFORM_REFLECTIVENESS], reflectiveness);

	glBindVertexArray(this->vertexArray);
	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(this->indices.size()), GL_UNSIGNED_INT, nullptr);
	glBindVertexArray(0);
}
