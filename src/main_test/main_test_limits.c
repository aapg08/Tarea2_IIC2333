#include <stdio.h>
#include <string.h>
#include "../os_memory_API/os_memory_API.h"

int main(int argc, char const *argv[]) {
    printf("=== PRUEBA DE CASOS LÍMITE Y ERRORES ===\n");
    
    // Verificar que se pasó un archivo de memoria como argumento
    if (argc != 2) {
        printf("Uso: %s <archivo_memoria.bin>\n", argv[0]);
        return 1;
    }
    
    // Montar la memoria
    printf("\n1. Montando memoria desde: %s\n", argv[1]);
    mount_memory((char *)argv[1]);

    // Limpiar procesos anteriores
    printf("\nLimpieza de procesos anteriores:\n");
    clear_all_processes();
    
    printf("\n2. Estado inicial:\n");
    list_processes();
    printf("   PCB slots disponibles: %d\n", processes_slots());
    
    // Prueba 1: Intentar operaciones con proceso inexistente
    printf("\n3. PRUEBA: Operaciones con proceso inexistente (PID 70):\n");
    printf("   3.1. Intentando listar archivos de proceso inexistente:\n");
    list_files(70);
    
    printf("   3.2. Intentando obtener file-table slots de proceso inexistente:\n");
    int slots = file_table_slots(70);
    printf("       Resultado: %d\n", slots);
    
    printf("   3.3. Intentando abrir archivo en proceso inexistente:\n");
    osmFile* invalid_file = open_file(70, "test.txt", 'w');
    printf("       Resultado: %s\n", invalid_file ? "Éxito (inesperado)" : "Fallo (esperado)");
    
    // Prueba 2: Crear proceso y probar límites de archivos
    printf("\n4. PRUEBA: Límites de archivos por proceso:\n");
    int test_pid = 50;
    start_process(test_pid, "test_limits");
    
    printf("   4.1. File-table slots inicial: %d\n", file_table_slots(test_pid));
    
    // Intentar crear muchos archivos para agotar la tabla
    printf("   4.2. Creando archivos hasta agotar tabla:\n");
    for (int i = 0; i < 10; i++) {  // Ajusta según el límite de tu implementación
        char filename[20];
        sprintf(filename, "file%d.txt", i);
        
        osmFile* file = open_file(test_pid, filename, 'w');
        if (file) {
            char data[50];
            sprintf(data, "Contenido del archivo %d", i);
            file->size = strlen(data) + 1;
            int write_result = write_file(file, data);
            close_file(file);
            printf("       Archivo %s: %s\n", filename, write_result ? "Creado" : "Fallo en escritura");
        } else {
            printf("       Archivo %s: Fallo al abrir (tabla llena?)\n", filename);
            break;
        }
    }
    
    printf("   4.3. File-table slots después de crear archivos: %d\n", file_table_slots(test_pid));
    printf("   4.4. Archivos creados:\n");
    list_files(test_pid);
    
    // Prueba 3: Intentar operaciones con archivos inexistentes
    printf("\n5. PRUEBA: Operaciones con archivos inexistentes:\n");
    printf("   5.1. Intentando leer archivo inexistente:\n");
    osmFile* nonexistent = open_file(test_pid, "noexiste.txt", 'r');
    printf("       Resultado: %s\n", nonexistent ? "Éxito (inesperado)" : "Fallo (esperado)");
    
    printf("   5.2. Intentando eliminar archivo inexistente:\n");
    delete_file(test_pid, "noexiste.txt");
    printf("       Eliminación intentada, no ejecuta nada\n");
    
    // Prueba 4: Intentar crear procesos con IDs duplicados
    printf("\n6. PRUEBA: Procesos con IDs duplicados:\n");
    printf("   6.1. Creando proceso original (PID 100):\n");
    int result1 = start_process(100, "original");
    printf("       Resultado: %s\n", result1 == 0 ? "Éxito" : "Fallo");
    
    printf("   6.2. Intentando crear proceso con mismo ID (PID 100):\n");
    int result2 = start_process(100, "duplicado");
    printf("       Resultado: %s\n", result2 == 0 ? "Éxito (inesperado)" : "Fallo (esperado)");
    
    // Prueba 5: Intentar finalizar proceso inexistente
    printf("\n7. PRUEBA: Finalizar proceso inexistente:\n");
    int finish_result = finish_process(200);
    printf("   Resultado: %s\n", finish_result == 0 ? "Éxito (inesperado)" : "Fallo (esperado)");
    
    // Prueba 6: Crear archivos con nombres largos o especiales
    printf("\n8. PRUEBA: Nombres de archivos especiales:\n");
    printf("   8.1. Archivo con nombre muy largo:\n");
    osmFile* long_name = open_file(test_pid, "archivo_con_nombre_muy_muy_muy_largo.txt", 'w');
    if (long_name) {
        char data[] = "Archivo con nombre largo";
        long_name->size = strlen(data) + 1;
        write_file(long_name, data);
        close_file(long_name);
        printf("       Éxito\n");
    } else {
        printf("       Fallo (esperado si hay límite de longitud)\n");
    }
    
    printf("   8.2. Archivo con nombre vacío:\n");
    osmFile* empty_name = open_file(test_pid, "", 'w');
    printf("       Resultado: %s\n", empty_name ? "Éxito" : "Fallo");
    if (empty_name) close_file(empty_name);
    
    // Estado final de pruebas
    printf("\n9. Estado final de pruebas:\n");
    list_processes();
    frame_bitmap_status();
    
    // Limpiar todo
    printf("\n10. Limpiando todos los procesos:\n");
    int cleared = clear_all_processes();
    printf("    Procesos cerrados: %d\n", cleared);
    
    printf("\n11. Estado final limpio:\n");
    list_processes();
    frame_bitmap_status();
    
    return 0;
}