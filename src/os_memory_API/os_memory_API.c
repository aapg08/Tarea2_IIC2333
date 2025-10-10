#include <stdio.h>	// FILE, fopen, fclose, etc.
#include <stdlib.h> // malloc, calloc, free, etc
#include <string.h> //para strcmp
#include <stdbool.h> // bool, true, false
#include <stdint.h> // uint16_t, uint8_t, etc.
#include "os_memory_API.h"

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

// Variable global para la ruta de la memoria
char* memory_path;

// Funciones generales
void mount_memory(char* memory_path_param) {
    // Liberar memoria anterior si existe
    if (memory_path != NULL) {
        free(memory_path);
    }
    
    // Validar que el parámetro no sea NULL
    // if (memory_path_param == NULL) {
    //     printf("Error: La ruta de memoria no puede ser NULL\n");
    //     return;
    // }
    
    // // Verificar que el archivo existe
    // FILE* file = fopen(memory_path_param, "rb");
    // if (file == NULL) {
    //     printf("Error: No se puede abrir el archivo de memoria: %s\n", memory_path_param);
    //     return;
    // }
    
    // // Verificar el tamaño del archivo
    // fseek(file, 0, SEEK_END);
    // long file_size = ftell(file);
    // long expected_size = PCB_SECTION_SIZE + IPT_SECTION_SIZE + BITMAP_SECTION_SIZE + DATA_SECTION_SIZE;
    
    // if (file_size != expected_size) {
    //     printf("Error: El archivo de memoria tiene un tamaño incorrecto. Esperado: %ld, Actual: %ld\n", 
    //            expected_size, file_size);
    //     fclose(file);
    //     return;
    // }
    
    // fclose(file);
    
    // Asignar memoria para almacenar la ruta
    memory_path = malloc(strlen(memory_path_param) + 1);
    if (memory_path == NULL) {
        printf("Error: No se pudo asignar memoria para la ruta\n");
        return;
    }
    
    // Copiar la ruta
    strcpy(memory_path, memory_path_param);
    
    printf("Memoria montada exitosamente: %s\n", memory_path);
}

// // funciones procesos

int start_process(int process_id, char* process_name) {
    // Abro el archivo de memoria
    FILE* file = fopen(memory_path, "rb+");
    PCBEntry nuevo_pcb;

    // Inicializar el nuevo PCB
    nuevo_pcb.estado = 0x01;  // 0x01 existe
    strncpy(nuevo_pcb.nombre, process_name, 14);
    nuevo_pcb.nombre[14] = '\0';
    nuevo_pcb.id = process_id;
    memset(nuevo_pcb.tabla_archivos, 0x00, 240);  // Inicializar tabla de archivos

    // Agregar el PCB a la tabla de PCBs en la memoria montada
    for (int i=0; i<PCB_ENTRIES; i++) {
        // fseek(FILE *stream, long offset, int origin); 
        // origin: SEEK_SET = desde el inicio del archivo
        fseek(file, PCB_OFFSET + i * PCB_ENTRY_SIZE, SEEK_SET);
        uint8_t estado;
        // Guarda en estado el byte leído
        fread(&estado, sizeof(uint8_t), 1, file);

        if (estado == 0x00) {
            // Encontró un espacio libre
            fseek(file, PCB_OFFSET + i * PCB_ENTRY_SIZE, SEEK_SET);
            fwrite(&nuevo_pcb, sizeof(PCBEntry), 1, file);
            fclose(file);
            return 0; // Caso Éxito
        }
    }

    fclose(file);
    return -1; // Caso Error: No hay espacio para más procesos
}

int finish_process(int process_id) {
    FILE* file = fopen(memory_path, "rb+");
    // Buscar el PCB del proceso
    for (int i=0; i < PCB_ENTRIES; i++) {
        fseek(file, PCB_OFFSET + i * PCB_ENTRY_SIZE, SEEK_SET);
        PCBEntry pcb;
        fread(&pcb, sizeof(PCBEntry), 1, file);

        if (pcb.estado == 0x01 && pcb.id == process_id) {
            pcb.estado = 0x00; // Marcar como libre
            fseek(file, PCB_OFFSET + i * PCB_ENTRY_SIZE, SEEK_SET);
            fwrite(&pcb, sizeof(PCBEntry), 1, file);
            fclose(file);
            return 0; // Caso Éxito
        }
    }
    // Falta liberar frames y archivos asociados al proceso

    fclose(file);
    return -1; // Caso Error
}

int clear_all_processes() {
    FILE* file = fopen(memory_path, "rb+");
    int terminados = 0;
    PCBEntry pcb;

    for (int i=0; i < PCB_ENTRIES; i++) {
        fseek(file, PCB_OFFSET + i * PCB_ENTRY_SIZE, SEEK_SET);
        fread(&pcb, sizeof(PCBEntry), 1, file);

        if (pcb.estado == 0x01) {
            pcb.estado = 0x00; // Marcar como libre
            fseek(file, PCB_OFFSET + i * PCB_ENTRY_SIZE, SEEK_SET);
            fwrite(&pcb, sizeof(PCBEntry), 1, file);
            terminados++;
        }
    }
    // Falta liberar frames y archivos asociados a los procesos

    fclose(file);
    return terminados;
}

// // funciones archivos

osmFile* open_file(int process_id, char* file_name, char mode) {
    FILE* file = fopen(memory_path, "rb+");
    // Buscar el PCB del proceso
    PCBEntry pcb;
    int pcb_index = -1;
    for (int i=0; i < PCB_ENTRIES; i++) {
        fseek(file, PCB_OFFSET + i * PCB_ENTRY_SIZE, SEEK_SET);
        fread(&pcb, sizeof(PCBEntry), 1, file);
        if (pcb.estado == 0x01 && pcb.id == process_id) {
            pcb_index = i;
            break;
        }
    }
    if (pcb_index == -1) {
        fclose(file);
        return NULL; // Proceso no encontrado
    }

    // Recorremos la tabla de archivos del PCB
    for (int j=0; j < FILE_TABLE_ENTRIES; j++) {
        int entry_offset = PCB_OFFSET + pcb_index * PCB_ENTRY_SIZE + 16 + j * 24; // 16 bytes de estado, nombre e id
        fseek(file, entry_offset, SEEK_SET);
        uint8_t valid;
        char name[15];
        fread(&valid, 1, 1, file);
        fread(name, 1, 14, file);
        name[14] = '\0';
        if (mode == 'r') {
            if (valid == 0x01 && strncmp(name, file_name, 14) == 0) {
                // Archivo encontrado
                osmFile* of = malloc(sizeof(osmFile));
                of->valid = valid;
                strncpy(of->name, name, 15);
                fread(&(of->size), 5, 1, file);
                fread(&(of->virtual_addr), 4, 1, file);
                of->process_id = process_id;
                of->mode = mode;
                fclose(file);
                return of;
            }
        } else if (mode == 'w') {
            if (valid == 0x01 && strncmp(name, file_name, 14) == 0) {
                // Archivo ya existe
                fclose(file);
                return NULL;
            }
        }

        if (mode == 'w') {
            // Buscar un espacio libre para crear el archivo
            for (int j=0; j < FILE_TABLE_ENTRIES; j++) {
                int entry_offset = PCB_OFFSET + pcb_index * PCB_ENTRY_SIZE + 16 + j * 24;
                fseek(file, entry_offset, SEEK_SET);
                uint8_t valid;
                fread(&valid, 1, 1, file);
                if (valid == 0x00) {
                    // Espacio libre encontrado, crear osmFile
                    osmFile* of = malloc(sizeof(osmFile));
                    of->valid = 0x01;
                    strncpy(of->name, file_name, 14);
                    of->name[14] = '\0';
                    of->size = 0;
                    of->virtual_addr = 0; // Asignar dirección virtual adecuada
                    of->process_id = process_id;
                    of->mode = mode;

                    // Escribir en la tabla de archivos
                    fseek(file, entry_offset, SEEK_SET);
                    fwrite(&(of->valid), 1, 1, file);
                    fwrite(of->name, 1, 14, file);
                    fwrite(&(of->size), 5, 1, file);
                    fwrite(&(of->virtual_addr), 4, 1, file);

                    fclose(file);
                    return of;
                }
            }
        }
        fclose(file);
        return NULL; // No hay espacio en la tabla de archivos
    }
    
    


}