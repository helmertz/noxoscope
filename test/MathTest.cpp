//===----------------------------------------------------------------------===//
//
// This file is part of the Noxoscope project
//
// Copyright (c) 2016 Niklas Helmertz
//
//===----------------------------------------------------------------------===//

#include "TestShared.h"

#include <Logging.h>
#include <GeometryMath.h>

TEST_CASE("Example test") {
	REQUIRE(1 + 1 == 2);
}

TEST_CASE("Test converting spherical and back") {
	using namespace glm;
	rc::prop("", []() {
		float radius = floatInRange(0.01f, 100.0f);
		float inclination = floatInRange(0.0f, PI_F);
		float azimuth = floatInRange(0.0f, 2.0f * PI_F);

		vec3 cartesian = sphericalToCartesian(radius, inclination, azimuth);
		vec3 sphericalRes = cartesianToSpherical(cartesian);

		float resRadius = sphericalRes.x;
		float resInclination = sphericalRes.y;
		float resAzimuth = fmod(sphericalRes.z + 2.0f * PI_F, 2.0f * PI_F); // Convert from [-pi,pi] to [0,2*pi]

		RC_ASSERT(approxEqual(radius, resRadius));
		RC_ASSERT(approxEqual(inclination, resInclination));

		// If inclination is either straight up or down, any azimuth would be valid
		if (!approxEqual(inclination, 0.0f) && !approxEqual(inclination, PI_F)) {
			RC_ASSERT(approxEqual(azimuth, resAzimuth));
		}
	});
}
