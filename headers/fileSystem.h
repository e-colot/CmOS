#pragma once

void addFile(const char* filePath, unsigned char ID);
void loadFile(unsigned char ID, unsigned char* mem, size_t len);
void removeFile(unsigned char ID);
size_t getFileSize(unsigned char ID);
