#include <stdio.h>
#include <string.h>
#include "../os_memory_API/os_memory_API.h"

int main(int argc, char const *argv[]) {
    printf("=== PRUEBA DE GESTIÓN DE PROCESOS ===\n");
    
    // Verificar que se pasó un archivo de memoria como argumento
    if (argc != 2) {
        printf("Uso: %s <archivo_memoria.bin>\n", argv[0]);
        return 1;
    }
    
    // Montar la memoria
    printf("\n1. Montando memoria desde: %s\n", argv[1]);
    mount_memory((char *)argv[1]);
    
    // Estado inicial
    printf("\n2. Estado inicial:\n");
    list_processes();
    printf("   PCB slots disponibles: %d\n", processes_slots());
    
    // Crear varios procesos
    printf("\n3. Creando procesos:\n");
    int result1 = start_process(10, "navegador");
    printf("   Proceso 10 'navegador' creado: %s\n", result1 == 0 ? "Éxito" : "Fallo");
    
    int result2 = start_process(20, "editor");
    printf("   Proceso 20 'editor' creado: %s\n", result2 == 0 ? "Éxito" : "Fallo");
    
    int result3 = start_process(30, "calculadora");
    printf("   Proceso 30 'calculadora' creado: %s\n", result3 == 0 ? "Éxito" : "Fallo");
    
    // Listar procesos después de crear
    printf("\n4. Procesos después de crear:\n");
    list_processes();
    printf("   PCB slots disponibles: %d\n", processes_slots());
    
    // Intentar crear proceso con ID duplicado
    printf("\n5. Intentando crear proceso con ID duplicado (10):\n");
    int result4 = start_process(10, "duplicado");
    printf("   Resultado: %s\n", result4 == 0 ? "Éxito (inesperado)" : "Fallo (esperado)");
    
    // Finalizar un proceso
    printf("\n6. Finalizando proceso 20:\n");
    int finish_result = finish_process(20);
    printf("   Proceso 20 finalizado: %s\n", finish_result == 0 ? "Éxito" : "Fallo");
    
    // Listar procesos después de finalizar
    printf("\n7. Procesos después de finalizar el 20:\n");
    list_processes();
    printf("   PCB slots disponibles: %d\n", processes_slots());
    
    // Intentar finalizar proceso inexistente
    printf("\n8. Intentando finalizar proceso inexistente (99):\n");
    int finish_result2 = finish_process(99);
    printf("   Resultado: %s\n", finish_result2 == 0 ? "Éxito (inesperado)" : "Fallo (esperado)");
    
    // Limpiar todos los procesos
    printf("\n9. Limpiando todos los procesos:\n");
    int cleared = clear_all_processes();
    printf("   Procesos cerrados: %d\n", cleared);
    
    // Estado final
    printf("\n10. Estado final:\n");
    list_processes();
    printf("    PCB slots disponibles: %d\n", processes_slots());
    
    return 0;
}