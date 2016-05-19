//===----------------------------------------------------------------------===//
//
// This file is part of the Noxoscope project
//
// Copyright (c) 2016 Niklas Helmertz
//
//===----------------------------------------------------------------------===//

#ifndef Entity_H
#define Entity_H

#include "Model.h"

class Entity {
public:
	explicit Entity(Model* model, glm::mat4 modelMatrix)
		: model(model), modelMatrix(modelMatrix) {}

	void render(const ShaderProgram& shader);
	Model* model;
	glm::mat4 modelMatrix;
};

#endif // Entity_H
