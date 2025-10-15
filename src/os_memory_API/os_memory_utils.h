#ifndef OS_MEMORY_UTILS_H
#define OS_MEMORY_UTILS_H

#include <stdint.h>
#include "os_memory_constants.h"
#include "../osm_File/osm_File.h"

// Variable global para la ruta de la memoria
extern char* memory_path;

// Estructura PCBEntry
typedef struct {
    uint8_t estado;         // 1 byte: 0x01 existe, 0x00 libre
    char nombre[15];        // 14 bytes + '\0' para uso local
    uint8_t id;             // 1 byte
    uint8_t tabla_archivos[240]; // 240 bytes para la tabla de archivos
} PCBEntry;

/*====== FUNCIONES EXTRAS =====*/

/**
 Busca el Page Frame Number (PFN) en la Inverted Page Table (IPT)
 para un proceso y página virtual dados
 */
int buscar_pfn_en_IPT(int process_id, int vpn);

/**
 Busca el primer espacio libre en la memoria virtual del proceso
 */
uint32_t buscar_espacio_libre_virtual(int process_id);

/**
 Función de depuración que imprime el estado de todos los PCBs
 y sus archivos asociados
 */
void debug_print_pcbs(void);

#endif // OS_MEMORY_UTILS_H