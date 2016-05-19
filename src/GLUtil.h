//===----------------------------------------------------------------------===//
//
// This file is part of the Noxoscope project
//
// Copyright (c) 2016 Niklas Helmertz
//
//===----------------------------------------------------------------------===//

#ifndef GLUtil_H
#define GLUtil_H

#include <tuple>

#include "ShaderProgram.h"

void attachTextures(const ShaderProgram& shader, const std::initializer_list<std::tuple<GLuint, const char*>>& textureTuples);
void checkFboStatus();

#endif // GLUtil_H
