#include <stdio.h>
#include <string.h>
#include "../os_memory_API/os_memory_API.h"

int main(int argc, char const *argv[]) {
    printf("=== PRUEBA DE FUNCIONES BÁSICAS ===\n");
    
    // Verificar que se pasó un archivo de memoria como argumento
    if (argc != 2) {
        printf("Uso: %s <archivo_memoria.bin>\n", argv[0]);
        return 1;
    }
    
    // Montar la memoria
    printf("\n1. Montando memoria desde: %s\n", argv[1]);
    mount_memory((char *)argv[1]);
    
    // Mostrar estado inicial
    printf("\n2. Estado inicial de la memoria:\n");
    printf("   Listando procesos existentes:\n");
    list_processes();
    
    printf("\n   PCB slots disponibles: %d\n", processes_slots());
    
    printf("\n   Estado del bitmap de frames:\n");
    frame_bitmap_status();
    
    return 0;
}