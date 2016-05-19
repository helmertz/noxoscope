//===----------------------------------------------------------------------===//
//
// This file is part of the Noxoscope project
//
// Copyright (c) 2016 Niklas Helmertz
//
//===----------------------------------------------------------------------===//

#include "Entity.h"

#include <glm/gtc/type_ptr.hpp>

#include "Constants.h"

void Entity::render(const ShaderProgram& shader) {
	glUniformMatrix4fv(shader[UNIFORM_MODEL_MATRIX], 1, GL_FALSE, value_ptr(modelMatrix));
	this->model->render(shader);
}
