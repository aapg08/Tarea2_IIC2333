#pragma once
#include <stdint.h>

// Estructura para representar un archivo abierto
typedef struct {
    uint8_t valid;           // 1 byte: 0x01 válido, 0x00 vacío
    char name[15];          // 14 bytes + '\0' para el nombre del archivo
    uint64_t size;          // 5 bytes como uint64_t (40 bits) en little endian
    uint32_t virtual_addr;  // 4 bytes dirección virtual
    int process_id;         // ID del proceso propietario
    uint8_t mode;           // Modo de apertura ('r', 'w')
} osmFile;