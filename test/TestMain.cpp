//===----------------------------------------------------------------------===//
//
// This file is part of the Noxoscope project
//
// Copyright (c) 2016 Niklas Helmertz
//
//===----------------------------------------------------------------------===//

#define CATCH_CONFIG_RUNNER
#include "TestShared.h"

#include <Logging.h>

int main(int argc, char* argv[]) {
	int result = Catch::Session().run(argc, argv);
	return result;
}
