#pragma once
#include <stdio.h>

void diskInit(size_t dimensions);
void diskWrite(size_t pos, unsigned char* data, size_t len);
void diskRead(size_t diskPos, unsigned char* mem, size_t memPos, size_t len);


