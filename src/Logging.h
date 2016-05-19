//===----------------------------------------------------------------------===//
//
// This file is part of the Noxoscope project
//
// Copyright (c) 2016 Niklas Helmertz
//
//===----------------------------------------------------------------------===//

#ifndef Logging_H
#define Logging_H

#include <cstdio>
#include <cstdlib>

#include <SDL.h>
#include <fmt/format.h>

#define NOMINMAX
#include <rlutil.h>
#undef near
#undef far

template <typename... Args>
void logInternal(const char* format, const Args&... args) {
	auto FILENAME = "log.txt";
	auto file = std::fopen(FILENAME, "ab+");
	if (file) {
		fmt::print(file, format, args...);
		fmt::print(file, "\n");
	}
}

template <typename... Args>
void debug(const char* format, const Args&... args) {
	setColor(rlutil::WHITE);
	fmt::print(format, args...);
	fmt::print("\n");
	setColor(rlutil::GREY);
}

template <typename... Args>
void warn(const char* format, const Args&... args) {
	setColor(rlutil::YELLOW);
	fmt::print(format, args...);
	fmt::print("\n");
	setColor(rlutil::GREY);
}

template <typename... Args>
void fatalError(const char* format, const Args&... args) {
	setColor(rlutil::LIGHTRED);
	fmt::print(stderr, format, args...);
	fmt::print("\n");
	logInternal(format, args...);
	setColor(rlutil::GREY);
	SDL_Quit();
	std::exit(1);
}

template <typename... Args>
void errorLog(const char* format, const Args&... args) {
	setColor(rlutil::LIGHTRED);
	fmt::print(stderr, format, args...);
	fmt::print("\n");
	logInternal(format, args...);
	setColor(rlutil::GREY);
}

template <typename... Args>
void log(const char* format, const Args&... args) {
	setColor(rlutil::YELLOW);
	fmt::print(format, args...);
	fmt::print("\n");
	logInternal(format, args...);
	setColor(rlutil::GREY);
}

template <typename... Args>
void debugColored(const char* format, int color, const Args&... args) {
	rlutil::setColor(color);
	fmt::print(format, args...);
	fmt::print("\n");
	setColor(rlutil::GREY);
}

#endif // Logging_H
