//===----------------------------------------------------------------------===//
//
// This file is part of the Noxoscope project
//
// Copyright (c) 2016 Niklas Helmertz
//
//===----------------------------------------------------------------------===//

#include "GeometryMath.h"

#include <glm/gtc/quaternion.hpp>

glm::vec3 cartesianToSpherical(glm::vec3 cartesian) {
	float radius = length(cartesian);
	float inclination = acos(cartesian.y / radius);
	float azimuth = atan2(cartesian.z, cartesian.x);
	return glm::vec3(radius, inclination, azimuth);
}

glm::vec3 sphericalToCartesian(glm::vec3 spherical) {
	return sphericalToCartesian(spherical.x, spherical.y, spherical.z);
}

glm::vec3 sphericalToCartesian(float radius, float inclination, float azimuth) {
	return glm::vec3(
		radius * sin(inclination) * cos(azimuth),
		radius * cos(inclination),
		radius * sin(inclination) * sin(azimuth)
	);
}
