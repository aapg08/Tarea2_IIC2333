#ifndef OS_MEMORY_CONSTANTS_H
#define OS_MEMORY_CONSTANTS_H

#include <stdint.h>

// Constantes de la estructura de memoria
#define PCB_SECTION_SIZE (8 * 1024)        // 8 KB
#define IPT_SECTION_SIZE (192 * 1024)      // 192 KB  
#define BITMAP_SECTION_SIZE (8 * 1024)     // 8 KB
#define DATA_SECTION_SIZE (2LL * 1024 * 1024 * 1024) // 2 GB

#define FRAME_SIZE (32 * 1024)             // 32 KB por frame
#define TOTAL_FRAMES 65536                 // 2^16 frames
#define PAGE_SIZE (32 * 1024)              // 32 KB por página
#define VIRTUAL_SPACE_SIZE (128 * 1024 * 1024) // 128 MB por proceso

#define PCB_ENTRIES 32                     // 32 procesos máximo
#define PCB_ENTRY_SIZE 256                 // 256 bytes por PCB
#define FILE_TABLE_ENTRIES 10              // 10 archivos por proceso

// Offsets de las secciones en el archivo
#define PCB_OFFSET 0
#define IPT_OFFSET (PCB_OFFSET + PCB_SECTION_SIZE)
#define BITMAP_OFFSET (IPT_OFFSET + IPT_SECTION_SIZE)
#define DATA_OFFSET (BITMAP_OFFSET + BITMAP_SECTION_SIZE)

#endif // OS_MEMORY_CONSTANTS_H