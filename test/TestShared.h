//===----------------------------------------------------------------------===//
//
// This file is part of the Noxoscope project
//
// Copyright (c) 2016 Niklas Helmertz
//
//===----------------------------------------------------------------------===//

#ifndef TestShared_H
#define TestShared_H

#include <catch.hpp>
#include <rapidcheck/catch.h>

bool approxEqual(float a, float b);

float floor0(float x);

float floatInRange(float a, float b);

#endif // TestShared_H
