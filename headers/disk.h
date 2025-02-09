#pragma once
#include <stdio.h>

void diskInit(size_t dimensions);
void diskWrite(size_t pos, char* data, size_t len);
void diskRead(size_t diskPos, char* mem, size_t memPos, size_t len);

void storeProgram(const char* filePath, size_t pos);

