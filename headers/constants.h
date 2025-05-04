#pragma once

#define RAM_SIZE 256
#define DISK_SIZE (8 * 1024)
#define PAGE_SIZE 16
#define FILE_ALLOCATION 1 // 0 = bitmap + FAT, 1 = Contiguous Allocation
#define MAX_PROCESSES 16

#define BITMAP_SIZE ((DISK_SIZE + (8 * PAGE_SIZE) - 1) / (8 * PAGE_SIZE))  // number of bytes in the bitmap, ceiled
#define BITMAP_PAGES ((BITMAP_SIZE + PAGE_SIZE - 1) / PAGE_SIZE)          // number of pages in the bitmap, ceiled

#define FAT_START BITMAP_PAGES


#if (DISK_SIZE / PAGE_SIZE) <= 256 // 2^8
    #define ADDRESSING_BYTES 1
#elif (DISK_SIZE / PAGE_SIZE) <= 65536 // 2^16
    #define ADDRESSING_BYTES 2
#elif (DISK_SIZE / PAGE_SIZE) <= 16777216 // 2^24
    #define ADDRESSING_BYTES 3
#else
    #define ADDRESSING_BYTES 4
#endif


// TESTS PARAMETERS
#define TEST_FILE_NBR 50
#define TEST_FILE_SIZE 150
