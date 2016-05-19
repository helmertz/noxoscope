//===----------------------------------------------------------------------===//
//
// This file is part of the Noxoscope project
//
// Copyright (c) 2016 Niklas Helmertz
//
//===----------------------------------------------------------------------===//

#ifndef FileTools_H
#define FileTools_H

#include <string>

#ifdef _MSC_VER
#include <direct.h>
#define MAXPATHLEN 260
#define chdir _chdir
#define getcwd _getcwd
#define stat _stat
#else
#include <unistd.h>
#include <sys/param.h>
#include <sys/stat.h>
#endif

#ifdef _WIN32
static const char PATH_SEP = '\\';
#else
static const char PATH_SEP = '/';
#endif

std::string baseDirRelative(const char* str);
std::string readFile(const char* filePath);
void fixOSPath(std::string& cs);

#endif // FileTools_H
