#include <stdio.h>	// FILE, fopen, fclose, etc.
#include <stdlib.h> // malloc, calloc, free, etc
#include <string.h> //para strcmp
#include <stdbool.h> // bool, true, false
#include <stdint.h> // uint16_t, uint8_t, etc.
#include "os_memory_API.h"
#include "os_memory_constants.h"
#include "os_memory_utils.h"

// Variable global para la ruta de la memoria
char* memory_path;

// Funciones generales ----------------------------------------------------------------------------
void mount_memory(char* memory_path_param) {
    // Liberar memoria anterior si existe
    if (memory_path != NULL) {
        free(memory_path);
    }
    
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

void list_processes() {
    FILE* file = fopen(memory_path, "rb+");
    PCBEntry pcb;
    for (int i=0; i < PCB_ENTRIES; i++) {
        fseek(file, PCB_OFFSET + i * PCB_ENTRY_SIZE, SEEK_SET);
        fread(&pcb, sizeof(PCBEntry), 1, file);
        if (pcb.estado == 0x01) {
            printf("%d %s\n", pcb.id, pcb.nombre);
        }
    }
    fclose(file);
}

int processes_slots() {
    // retorna la cantidad de entradas libres de PCBs
    FILE* file = fopen(memory_path, "rb+");
    int free_slots = 0;
    PCBEntry pcb;
    for (int i=0; i < PCB_ENTRIES; i++) {
        fseek(file, PCB_OFFSET + i * PCB_ENTRY_SIZE, SEEK_SET);
        fread(&pcb, sizeof(PCBEntry), 1, file);
        if (pcb.estado == 0x00) {
            free_slots++;
        }
    }
    fclose(file);
    return free_slots;
}

void list_files(int process_id) {
    // imprime los archivos de un proceso
    FILE* file = fopen(memory_path, "rb+");
    int pcb_index = -1;
    PCBEntry pcb;
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
        return;
    }

    for (int j=0; j < FILE_TABLE_ENTRIES; j++) {
        int entry_offset = PCB_OFFSET + pcb_index * PCB_ENTRY_SIZE + 16 + j * 24; // 16 bytes de estado, nombre e id
        fseek(file, entry_offset, SEEK_SET);
        uint8_t valid;
        char name[15];
        uint64_t size;
        uint32_t virtual_addr;
        fread(&valid, 1, 1, file);
        fread(name, 1, 14, file);
        name[14] = '\0';
        fread(&size, 5, 1, file);
        fread(&virtual_addr, 4, 1, file);
        
        if (valid == 0x01) {
            int vpn = virtual_addr / PAGE_SIZE;
            printf("%X %lu %X %s\n", vpn, size, virtual_addr, name);
        }
    }
    fclose(file);
}

void frame_bitmap_status() {
    // imprime cantidad de frames usados y libres
    FILE* file = fopen(memory_path, "rb+");
    fseek(file, BITMAP_OFFSET, SEEK_SET);
    uint8_t bitmap[BITMAP_SECTION_SIZE];
    fread(bitmap, 1, BITMAP_SECTION_SIZE, file);
    int usados = 0;
    int libres = 0;
    for (int i=0; i < TOTAL_FRAMES; i++) {
        int byte_index = i / 8;
        int bit_index = i % 8;
        if (bitmap[byte_index] & (1 << bit_index)) {
            usados++;
        } else {
            libres++;
        }
    }
    printf("USADOS: %d LIBRES: %d\n", usados, libres);
    fclose(file);
}

// funciones procesos ----------------------------------------------------------------------------

int start_process(int process_id, char* process_name) {
    // Abro el archivo de memoria
    FILE* file = fopen(memory_path, "rb+");
    if (!file) {
        printf("Error al abrir archivo de memoria\n");
        return -1;
    }
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
    int pcb_index = -1;
    PCBEntry pcb;
    // Buscar el PCB del proceso
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
        return -1; // Proceso no encontrado
    }

    // Marcar archivos del proceso como inválidos
    for (int j = 0; j < FILE_TABLE_ENTRIES; j++) {
        int entry_offset = PCB_OFFSET + pcb_index * PCB_ENTRY_SIZE + 16 + j * 24; // 16 bytes de estado, nombre e id
        fseek(file, entry_offset, SEEK_SET);
        uint8_t valid;
        fread(&valid, 1, 1, file);
        if (valid == 0x01) {
            valid = 0x00; // Marcar como inválido
            fseek(file, entry_offset, SEEK_SET);
            fwrite(&valid, 1, 1, file);
        }
    }
    // Marcar el PCB como libre
    fseek(file, PCB_OFFSET + pcb_index * PCB_ENTRY_SIZE, SEEK_SET);
    pcb.estado = 0x00;
    fwrite(&pcb, sizeof(PCBEntry), 1, file);

    // Liberar frames e IPT para el proceso eliminado
    for (int pfn = 0; pfn < TOTAL_FRAMES; pfn++) {
        long entry_offset = IPT_OFFSET + pfn * 3;
        fseek(file, entry_offset, SEEK_SET);
        uint8_t bytes[3];
        fread(bytes, 1, 3, file);

        int valid = (bytes[0] & 0x80) >> 7;
        int pid_entry = ((bytes[0] & 0x3F) << 4) | ((bytes[1] & 0xF0) >> 4);

        if (valid && pid_entry == process_id) {
            // Marcar frame como libre en bitmap
            int byte_index = pfn / 8;
            int bit_index = pfn % 8;
            fseek(file, BITMAP_OFFSET, SEEK_SET);
            uint8_t bitmap[BITMAP_SECTION_SIZE];
            fread(bitmap, 1, BITMAP_SECTION_SIZE, file);
            bitmap[byte_index] &= ~(1 << bit_index);
            fseek(file, BITMAP_OFFSET, SEEK_SET);
            fwrite(bitmap, 1, BITMAP_SECTION_SIZE, file);

            // Marcar entrada IPT como inválida
            bytes[0] &= 0x7F; // valid = 0
            fseek(file, entry_offset, SEEK_SET);
            fwrite(bytes, 1, 3, file);
        }
    }

    fclose(file);
    return 0; // Caso Error
}

int clear_all_processes() {
    FILE* file = fopen(memory_path, "rb+");
    int terminados = 0;
    PCBEntry pcb;

    for (int i=0; i < PCB_ENTRIES; i++) {
        fseek(file, PCB_OFFSET + i * PCB_ENTRY_SIZE, SEEK_SET);
        fread(&pcb, sizeof(PCBEntry), 1, file);

        if (pcb.estado == 0x01) {
            for (int j = 0; j < FILE_TABLE_ENTRIES; j++) {
                int entry_offset = PCB_OFFSET + i * PCB_ENTRY_SIZE + 16 + j * 24; // 16 bytes de estado, nombre e id
                fseek(file, entry_offset, SEEK_SET);
                uint8_t valid;
                fread(&valid, 1, 1, file);
                if (valid == 0x01) {
                    valid = 0x00; // Marcar como inválido
                    fseek(file, entry_offset, SEEK_SET);
                    fwrite(&valid, 1, 1, file);
                }
            }
            fseek(file, PCB_OFFSET + i * PCB_ENTRY_SIZE, SEEK_SET);
            pcb.estado = 0x00; // Marcar como libre
            fwrite(&pcb, sizeof(PCBEntry), 1, file);
            for (int pfn = 0; pfn < TOTAL_FRAMES; pfn++) {
                long entry_offset = IPT_OFFSET + pfn * 3;
                fseek(file, entry_offset, SEEK_SET);
                uint8_t bytes[3];
                fread(bytes, 1, 3, file);

                int valid = (bytes[0] & 0x80) >> 7;
                int pid_entry = ((bytes[0] & 0x3F) << 4) | ((bytes[1] & 0xF0) >> 4);

                if (valid && pid_entry == pcb.id) {
                    int byte_index = pfn / 8;
                    int bit_index = pfn % 8;
                    fseek(file, BITMAP_OFFSET, SEEK_SET);
                    uint8_t bitmap[BITMAP_SECTION_SIZE];
                    fread(bitmap, 1, BITMAP_SECTION_SIZE, file);
                    bitmap[byte_index] &= ~(1 << bit_index);
                    fseek(file, BITMAP_OFFSET, SEEK_SET);
                    fwrite(bitmap, 1, BITMAP_SECTION_SIZE, file);

                    bytes[0] &= 0x7F; // valid = 0
                    fseek(file, entry_offset, SEEK_SET);
                    fwrite(bytes, 1, 3, file);
                }
            }
            terminados++;
        }
    }
    fclose(file);
    return terminados;
}

int file_table_slots(int process_id) {
    FILE* file = fopen(memory_path, "rb+");
    int pcb_index = -1;
    int free_slots = 0;
    // Buscar el PCB del proceso
    PCBEntry pcb;
    for (int i = 0; i < PCB_ENTRIES; i++) {
        fseek(file, PCB_OFFSET + i * PCB_ENTRY_SIZE, SEEK_SET);
        fread(&pcb, sizeof(PCBEntry), 1, file);
        if (pcb.estado == 0x01 && pcb.id == process_id) {
            pcb_index = i;
            break;
        }
    }
    if (pcb_index == -1) {
        fclose(file);
        return -1; // Proceso no encontrado
    }

    // Contar espacios libres en la tabla de archivos
    for (int j = 0; j < FILE_TABLE_ENTRIES; j++) {
        int entry_offset = PCB_OFFSET + pcb_index * PCB_ENTRY_SIZE + 16 + j * 24; // 16 bytes de estado, nombre e id
        fseek(file, entry_offset, SEEK_SET);
        uint8_t valid;
        fread(&valid, 1, 1, file);
        if (valid == 0x00) {
            free_slots++;
        }
    }
    fclose(file);
    return free_slots;
}

// funciones archivos ----------------------------------------------------------------------------

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
    if (mode == 'r') {
        for (int j=0; j < FILE_TABLE_ENTRIES; j++) {
            int entry_offset = PCB_OFFSET + pcb_index * PCB_ENTRY_SIZE + 16 + j * 24; // 16 bytes de estado, nombre e id
            fseek(file, entry_offset, SEEK_SET);
            uint8_t valid;
            char name[15];
            fread(&valid, 1, 1, file);
            fread(name, 1, 14, file);
            name[14] = '\0';
            
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
        }
    }
    
    if (mode == 'w') {
        // Primero verificar si ya existe
        for (int j = 0; j < FILE_TABLE_ENTRIES; j++) {
            int entry_offset = PCB_OFFSET + pcb_index * PCB_ENTRY_SIZE + 16 + j * 24;
            fseek(file, entry_offset, SEEK_SET);
            uint8_t valid;
            char name[15];
            fread(&valid, 1, 1, file);
            fread(name, 1, 14, file);
            name[14] = '\0';
            if (valid == 0x01 && strncmp(name, file_name, 14) == 0) {
                fclose(file);
                return NULL; // Ya existe
            }
        }
        // Buscar espacio libre para crear
        for (int j = 0; j < FILE_TABLE_ENTRIES; j++) {
            int entry_offset = PCB_OFFSET + pcb_index * PCB_ENTRY_SIZE + 16 + j * 24;
            fseek(file, entry_offset, SEEK_SET);
            uint8_t valid;
            fread(&valid, 1, 1, file);
            if (valid == 0x00) {
                osmFile* of = malloc(sizeof(osmFile));
                of->valid = 0x01;
                strncpy(of->name, file_name, 14);
                of->name[14] = '\0';
                of->size = 0;
                of->virtual_addr = buscar_espacio_libre_virtual(process_id); // Asignar dirección virtual adecuada
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
        fclose(file);
        return NULL; // No hay espacio
    }
    fclose(file);
    return NULL; // No hay espacio en la tabla de archivos
}

int read_file(osmFile* file_desc, char* dest) {
    FILE* file = fopen(memory_path, "rb+");
    uint32_t vaddr = file_desc->virtual_addr;
    uint64_t size = file_desc->size;
    int pid = file_desc->process_id;
    size_t total_leidos = 0;

    while (total_leidos < size) {
        uint32_t vpn = (vaddr >> 15) & 0xFFF; // 12 bits de VPN
        uint32_t offset = vaddr % 0x7FFF; // 15 bits de offset
        uint16_t pfn = buscar_pfn_en_IPT(pid, vpn);

        if (pfn < 0) break;

        long paddr_real = (pfn << 15) | offset;
        long paddr_abs = PCB_SECTION_SIZE + IPT_SECTION_SIZE + BITMAP_SECTION_SIZE + paddr_real;

        fseek(file, paddr_abs, SEEK_SET);

        size_t bytes_pagina = PAGE_SIZE - offset;
        size_t bytes_restantes = size - total_leidos;
        size_t bytes_a_leer = (bytes_restantes < bytes_pagina) ? bytes_restantes : bytes_pagina;

        fread(dest + total_leidos, 1, bytes_a_leer, file);
        total_leidos += bytes_a_leer;
        vaddr += bytes_a_leer;
    }

    fclose(file);
    return (int)total_leidos;

}

int write_file(osmFile* file_desc, char* src) {
    FILE* file = fopen(memory_path, "rb+");
    uint32_t vaddr = file_desc->virtual_addr;
    uint64_t size = file_desc->size;
    printf("size %lu\n", size);
    int pid = file_desc->process_id;
    size_t total_escritos = 0;

    while (total_escritos < size) {
        // Verificar espacio virtual contiguo libre
        int pcb_index = -1;
        PCBEntry pcb;
        for (int i = 0; i < PCB_ENTRIES; i++) {
            fseek(file, PCB_OFFSET + i * PCB_ENTRY_SIZE, SEEK_SET);
            fread(&pcb, sizeof(PCBEntry), 1, file);
            if (pcb.estado == 0x01 && pcb.id == pid) {
                pcb_index = i;
                break;
            }
        }
        printf("pcb_index=%d\n", pcb_index);
        int solapado = 0;
        for (int j = 0; j < FILE_TABLE_ENTRIES; j++) {
            int entry_offset = PCB_OFFSET + pcb_index * PCB_ENTRY_SIZE + 16 + j * 24;
            fseek(file, entry_offset, SEEK_SET);
            uint8_t valid;
            char name[15];
            fread(&valid, 1, 1, file);
            fread(name, 1, 14, file);
            name[14] = '\0';
            uint64_t other_size;
            uint32_t other_vaddr;
            fread(&other_size, 5, 1, file);
            fread(&other_vaddr, 4, 1, file);
            if (valid == 0x01 && strncmp(name, file_desc->name, 14) != 0) {
                uint32_t start = other_vaddr;
                uint32_t end = other_vaddr + (uint32_t)other_size;
                uint32_t this_start = vaddr;
                uint32_t this_end = vaddr + (size - total_escritos);
                if (!(this_end <= start || this_start >= end)) {
                    // Hay solapamiento
                    printf("Solapamiento detectado: otro archivo [%s] ocupa vaddr %u-%u\n", name, start, end);
                    solapado = 1;
                    // Limitar la escritura hasta el inicio del archivo siguiente
                    size_t max_escribir = start > this_start ? start - this_start : 0;
                    printf("max_escribir=%zu\n", max_escribir);
                    if (max_escribir == 0) {
                        fclose(file);
                        return (int)total_escritos;
                    }
                    size = total_escritos + max_escribir;
                    break;
                }
            }
        }

        printf("write_file: total_escritos=%zu, size=%lu, vaddr=%u\n", total_escritos, size, vaddr);
        printf("hasta acá\n");
        uint32_t vpn = (vaddr >> 15) & 0xFFF; // 12 bits de VPN
        uint32_t offset = vaddr % PAGE_SIZE; // 15 bits de offset
        int pfn = buscar_pfn_en_IPT(pid, vpn);
        printf("salto\n");

        // Si no existe la página, asignar un frame libre
        if (pfn < 0) {
            // Buscar frame libre en bitmap
            fseek(file, BITMAP_OFFSET, SEEK_SET);
            uint8_t bitmap[BITMAP_SECTION_SIZE];
            fread(bitmap, 1, BITMAP_SECTION_SIZE, file);
            int frame_libre = -1;
            for (int i = 0; i < TOTAL_FRAMES; i++) {
                int byte_index = i / 8;
                int bit_index = i % 8;
                if (!(bitmap[byte_index] & (1 << bit_index))) {
                    frame_libre = i;
                    // Marcar como usado
                    bitmap[byte_index] |= (1 << bit_index);
                    break;
                }
            }
            if (frame_libre == -1) {
                printf("write_file: No hay frames libres, abortando\n");
                fclose(file);
                return -1; // No hay frames libres
            }
            // Actualizar bitmap en archivo
            fseek(file, BITMAP_OFFSET, SEEK_SET);
            fwrite(bitmap, 1, BITMAP_SECTION_SIZE, file);

            // Actualizar IPT
            long entry_offset = IPT_OFFSET + frame_libre * 3;
            fseek(file, entry_offset, SEEK_SET);
            uint8_t bytes[3] = {0};
            bytes[0] = 0x80 | ((pid >> 4) & 0x3F); // valid=1, pid parte alta
            bytes[1] = ((pid & 0x0F) << 4) | ((vpn >> 9) & 0x0F); // pid parte baja, vpn parte alta
            bytes[2] = vpn & 0x1FF; // vpn parte baja
            fwrite(bytes, 1, 3, file);
            pfn = frame_libre;
        }

        long paddr_real = ((uint32_t)pfn << 15) | offset;
        long paddr_abs = DATA_OFFSET + paddr_real;
        fseek(file, paddr_abs, SEEK_SET);

        size_t bytes_pagina = PAGE_SIZE - offset;
        size_t bytes_restantes = size - total_escritos;
        size_t bytes_a_escribir = (bytes_restantes < bytes_pagina) ? bytes_restantes : bytes_pagina;

        printf("write_file: escribiendo %zu bytes en pfn=%d, vpn=%u, offset=%u, paddr_abs=%ld\n", bytes_a_escribir, pfn, vpn, offset, paddr_abs);
        fwrite(src + total_escritos, 1, bytes_a_escribir, file);
        total_escritos += bytes_a_escribir;
        vaddr += bytes_a_escribir;
    }

    // Buscar el PCB del proceso
    int pcb_index = -1;
    PCBEntry pcb;
    for (int i = 0; i < PCB_ENTRIES; i++) {
        fseek(file, PCB_OFFSET + i * PCB_ENTRY_SIZE, SEEK_SET);
        fread(&pcb, sizeof(PCBEntry), 1, file);
        if (pcb.estado == 0x01 && pcb.id == pid) {
            pcb_index = i;
            break;
        }
    }
    if (pcb_index != -1) {
        // Buscar la entrada de archivo
        for (int j = 0; j < FILE_TABLE_ENTRIES; j++) {
            int entry_offset = PCB_OFFSET + pcb_index * PCB_ENTRY_SIZE + 16 + j * 24;
            fseek(file, entry_offset, SEEK_SET);
            uint8_t valid;
            char name[15];
            fread(&valid, 1, 1, file);
            fread(name, 1, 14, file);
            name[14] = '\0';
            if (valid == 0x01 && strncmp(name, file_desc->name, 14) == 0) {
                // Actualizar el tamaño
                fseek(file, entry_offset + 1 + 14, SEEK_SET); // Salta valid y name
                fwrite(&total_escritos, 5, 1, file); // Escribe el nuevo tamaño
                break;
            }
        }
    }

    fclose(file);
    return (int)total_escritos;
}

void delete_file(int process_id, char* file_name) {
    FILE* file = fopen(memory_path, "rb+");
    int pcb_index = -1;
    PCBEntry pcb;
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
        return; // Proceso no encontrado
    }

    for (int j=0; j < FILE_TABLE_ENTRIES; j++) {
        int entry_offset = PCB_OFFSET + pcb_index * PCB_ENTRY_SIZE + 16 + j * 24;
        fseek(file, entry_offset, SEEK_SET);
        uint8_t valid;
        char name[15];
        fread(&valid, 1, 1, file);
        fread(name, 1, 14, file);
        name[14] = '\0';
        
        if (valid == 0x01 && strncmp(name, file_name, 14) == 0) {
            valid = 0x00; // Marcar como inválido
            uint64_t size = 0;
            uint32_t vaddr = 0;
            fseek(file, entry_offset, SEEK_SET);
            fwrite(&valid, 1, 1, file);
            fwrite(name, 1, 14, file); // Mantener el nombre por si se revisa
            fwrite(&size, 5, 1, file);
            fwrite(&vaddr, 4, 1, file);
            // Falta liberar frames asociados al archivo
            break;
        }
    }
    fclose(file);
}

void close_file(osmFile* file_desc) {
    free(file_desc);
}