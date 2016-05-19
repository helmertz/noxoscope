//===----------------------------------------------------------------------===//
//
// This file is part of the Noxoscope project
//
// Copyright (c) 2016 Niklas Helmertz
//
//===----------------------------------------------------------------------===//

#ifndef Light_H
#define Light_H

#include <glm/vec3.hpp>

struct PointLight {
	glm::vec3 position;
	float radius;
	glm::vec3 color;
};

#endif // Light_H
