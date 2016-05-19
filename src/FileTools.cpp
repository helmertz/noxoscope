//===----------------------------------------------------------------------===//
//
// This file is part of the Noxoscope project
//
// Copyright (c) 2016 Niklas Helmertz
//
//===----------------------------------------------------------------------===//

#include "FileTools.h"

#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>

#include <SDL.h>

std::string baseDirRelative(const char* str) {
	std::stringstream ss;
	ss << SDL_GetBasePath() << PATH_SEP << str;
	return ss.str();
}

std::string readFile(const char* filePath) {
	std::ifstream in(filePath);
	std::string contents((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
	return contents;
}

void fixOSPath(std::string& cs) {
	std::replace(cs.begin(), cs.end(), '/', PATH_SEP);
	std::replace(cs.begin(), cs.end(), '\\', PATH_SEP);
}
