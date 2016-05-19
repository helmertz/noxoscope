//===----------------------------------------------------------------------===//
//
// This file is part of the Noxoscope project
//
// Copyright (c) 2016 Niklas Helmertz
//
//===----------------------------------------------------------------------===//

#ifndef GeometryMath_H
#define GeometryMath_H

#include <glm/vec3.hpp>

constexpr float PI_F = 3.14159265358979f;

glm::vec3 cartesianToSpherical(glm::vec3 cartesian);
glm::vec3 sphericalToCartesian(float radius, float theta, float phi);
glm::vec3 sphericalToCartesian(glm::vec3 spherical);

#endif // GeometryMath_H
