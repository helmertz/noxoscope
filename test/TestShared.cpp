//===----------------------------------------------------------------------===//
//
// This file is part of the Noxoscope project
//
// Copyright (c) 2016 Niklas Helmertz
//
//===----------------------------------------------------------------------===//

#include "TestShared.h"

bool approxEqual(float a, float b) {
	return a == Approx(b);
}

float floor0(float x) {
	return x >= 0 ? floorf(x) : ceilf(x);
}

float floatInRange(float a, float b) {
	return *rc::gen::inRange(int(floor0(a * 100.0f)), int(floor0(b * 100.0f))) / 100.0f;
}
