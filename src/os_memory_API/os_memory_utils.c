#include <stdio.h>   // FILE, fopen, fclose, etc.
#include <stdlib.h>  // malloc, calloc, free, etc
#include <string.h>  // para strcmp
#include <stdbool.h> // bool, true, false
#include <stdint.h>  // uint16_t, uint8_t, etc.
#include "os_memory_utils.h"

int buscar_pfn_en_IPT(int process_id, int vpn) {
    FILE* file = fopen(memory_path, "rb+");
    if (!file) {
        printf("Error: No se pudo abrir el archivo de memoria\n");
        return -1;
    }
    
    fseek(file, IPT_OFFSET, SEEK_SET);
    for (int pfn = 0; pfn < TOTAL_FRAMES; pfn++) {
        long entry_offset = IPT_OFFSET + pfn * 3;
        fseek(file, entry_offset, SEEK_SET);

        uint8_t bytes[3];
        size_t leidos = fread(bytes, 1, 3, file);
        if (leidos != 3) {
            printf("Error: fread no pudo leer 3 bytes en pfn=%u\n", pfn);
            break;
        }

        int valid = (bytes[0] & 0x80) >> 7; // bit más significativo
        int pid_entry = ((bytes[0] & 0x3F) << 4) | ((bytes[1] & 0xF0) >> 4);
        int vpn_entry = ((bytes[1] & 0x0F) << 9) | bytes[2];

        if (valid && pid_entry == process_id && vpn_entry == vpn) {
            fclose(file);
            return pfn; // Encontrado
        }
    }
    fclose(file);
    return -1; // No encontrado
}

uint32_t buscar_espacio_libre_virtual(int process_id) {
    FILE* file = fopen(memory_path, "rb+");
    if (!file) {
        printf("Error: No se pudo abrir el archivo de memoria\n");
        return 0;
    }
    
    int pcb_index = -1;
    PCBEntry pcb;
    // Buscar el PCB del proceso
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
        return 0; // Proceso no encontrado, por defecto 0
    }

    // Guardar los rangos ocupados por archivos
    uint32_t ocupados[FILE_TABLE_ENTRIES][2]; // [inicio, fin]
    int count = 0;
    for (int j = 0; j < FILE_TABLE_ENTRIES; j++) {
        int entry_offset = PCB_OFFSET + pcb_index * PCB_ENTRY_SIZE + 16 + j * 24;
        fseek(file, entry_offset, SEEK_SET);
        uint8_t valid;
        fread(&valid, 1, 1, file);
        if (valid == 0x01) {
            char name[15];
            fread(name, 1, 14, file);
            uint64_t size;
            uint32_t vaddr;
            fread(&size, 5, 1, file);
            fread(&vaddr, 4, 1, file);
            ocupados[count][0] = vaddr;
            ocupados[count][1] = vaddr + (uint32_t)size;
            count++;
        }
    }
    fclose(file);

    // Buscar el primer espacio libre
    uint32_t actual = 0;
    const uint32_t LIMITE = VIRTUAL_SPACE_SIZE;
    while (actual < LIMITE) {
        int ocupado = 0;
        for (int k = 0; k < count; k++) {
            if (actual >= ocupados[k][0] && actual < ocupados[k][1]) {
                ocupado = 1;
                actual = ocupados[k][1]; // Saltar al final del archivo ocupado
                break;
            }
        }
        if (!ocupado) {
            return actual;
        }
    }
    return 0; // Si no hay espacio, retorna 0
}

void debug_print_pcbs(void) {
    FILE* file = fopen(memory_path, "rb+");
    if (!file) {
        printf("Error: No se pudo abrir el archivo de memoria\n");
        return;
    }
    
    PCBEntry pcb;
    printf("---- Estado PCB ----\n");
    for (int i = 0; i < PCB_ENTRIES; i++) {
        fseek(file, PCB_OFFSET + i * PCB_ENTRY_SIZE, SEEK_SET);
        fread(&pcb, sizeof(PCBEntry), 1, file);
        printf("PCB[%d]: estado=%02X, id=%d, nombre=%s\n", i, pcb.estado, pcb.id, pcb.nombre);
        // Opcional: imprimir tabla de archivos
        for (int j = 0; j < FILE_TABLE_ENTRIES; j++) {
            uint8_t valid;
            char name[15];
            uint64_t size;
            uint32_t vaddr;
            int entry_offset = PCB_OFFSET + i * PCB_ENTRY_SIZE + 16 + j * 24;
            fseek(file, entry_offset, SEEK_SET);
            fread(&valid, 1, 1, file);
            fread(name, 1, 14, file);
            name[14] = '\0';
            fread(&size, 5, 1, file);
            fread(&vaddr, 4, 1, file);
            if (valid == 0x01) {
                printf("  Archivo[%d]: %s, size=%lu, vaddr=%u\n", j, name, size, vaddr);
            }
        }
    }
    fclose(file);
    printf("--------------------\n");
}