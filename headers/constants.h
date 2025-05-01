#pragma once

#define RAM_SIZE 256
#define DISK_SIZE (4 * 1024) // 4kB
#define PAGE_SIZE 16 // 16 bytes per page
#define MAX_PROCESSES 16

#define BITMAP_SIZE (DISK_SIZE/(8*PAGE_SIZE)) 
#define BITMAP_PAGES (BITMAP_SIZE/PAGE_SIZE)
