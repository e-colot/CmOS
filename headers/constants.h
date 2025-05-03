#pragma once

#define RAM_SIZE 256
#define DISK_SIZE (512 * 1024)
#define PAGE_SIZE 64
#define MAX_PROCESSES 16

#define BITMAP_SIZE (DISK_SIZE/(8*PAGE_SIZE))  // number of bytes in the bitmap
#define BITMAP_PAGES (BITMAP_SIZE/PAGE_SIZE)

#define FAT_START BITMAP_PAGES


#if (DISK_SIZE / PAGE_SIZE) <= 256
    #define ADDRESSING_BYTES 1
#elif (DISK_SIZE / PAGE_SIZE) <= 65536
    #define ADDRESSING_BYTES 2
#elif (DISK_SIZE / PAGE_SIZE) <= 16777216
    #define ADDRESSING_BYTES 3
#else
    #define ADDRESSING_BYTES 4
#endif


