#include <stdio.h>
#include <string.h>
#include "../os_memory_API/os_memory_API.h"

int main(int argc, char const *argv[]) {
    printf("=== PRUEBA DE GESTIÓN DE ARCHIVOS ===\n");
    
    // Verificar que se pasó un archivo de memoria como argumento
    if (argc != 2) {
        printf("Uso: %s <archivo_memoria.bin>\n", argv[0]);
        return 1;
    }
    
    // Montar la memoria
    printf("\n1. Montando memoria desde: %s\n", argv[1]);
    mount_memory((char *)argv[1]);
    clear_all_processes(); // Limpiar procesos preexistentes
    
    // Crear un proceso para trabajar con archivos
    printf("\n2. Creando proceso para trabajar con archivos:\n");
    int process_id = 10;
    int result = start_process(process_id, "file_manager");
    printf("   Proceso %d 'file_manager' creado: %s\n", process_id, result == 0 ? "Éxito" : "Fallo");
    
    if (result) {
        printf("Error: No se pudo crear el proceso. Terminando prueba.\n");
        return 1;
    }
    
    // Verificar slots de tabla de archivos
    printf("\n3. Verificando slots de tabla de archivos:\n");
    int file_slots = file_table_slots(process_id);
    printf("   File-table slots disponibles para proceso %d: %d\n", process_id, file_slots);
    
    // Listar archivos inicial (debería estar vacío)
    printf("\n4. Archivos iniciales del proceso:\n");
    list_files(process_id);
    
    // Crear y escribir varios archivos
    printf("\n5. Creando y escribiendo archivos:\n");
    
    // Archivo 1: log.txt
    printf("   5.1. Creando archivo 'log.txt':\n");
    osmFile* file1 = open_file(process_id, "log.txt", 'w');
    if (file1) {
        char data1[] = "Este es un archivo de log del sistema.";
        file1->size = strlen(data1) + 1;
        int write_result1 = write_file(file1, data1);
        printf("       Escritura de log.txt: %s\n", write_result1 ? "Éxito" : "Fallo");
        close_file(file1);
    } else {
        printf("       Error: No se pudo abrir log.txt para escritura\n");
    }
    
    // Archivo 2: config.ini
    printf("   5.2. Creando archivo 'config.ini':\n");
    osmFile* file2 = open_file(process_id, "config.ini", 'w');
    if (file2) {
        char data2[] = "[settings]\nresolution=1920x1080\nvolume=75";
        file2->size = strlen(data2) + 1;
        int write_result2 = write_file(file2, data2);
        printf("       Escritura de config.ini: %s\n", write_result2 ? "Éxito" : "Fallo");
        close_file(file2);
    } else {
        printf("       Error: No se pudo abrir config.ini para escritura\n");
    }
    
    // Archivo 3: datos.csv
    printf("   5.3. Creando archivo 'datos.csv':\n");
    osmFile* file3 = open_file(process_id, "datos.csv", 'w');
    if (file3) {
        char data3[] = "nombre,edad,ciudad\nJuan,25,Santiago\nMaria,30,Valparaiso";
        file3->size = strlen(data3) + 1;
        int write_result3 = write_file(file3, data3);
        printf("       Escritura de datos.csv: %s\n", write_result3 ? "Éxito" : "Fallo");
        close_file(file3);
    } else {
        printf("       Error: No se pudo abrir datos.csv para escritura\n");
    }
    
    // Listar archivos después de crear
    printf("\n6. Archivos después de crear:\n");
    list_files(process_id);
    
    // Verificar slots de tabla de archivos después de crear
    printf("\n7. File-table slots después de crear archivos: %d\n", file_table_slots(process_id));
    
    // Leer archivos
    printf("\n8. Leyendo archivos creados:\n");
    
    // Leer log.txt
    printf("   8.1. Leyendo 'log.txt':\n");
    osmFile* read_file1 = open_file(process_id, "log.txt", 'r');
    if (read_file1) {
        char buffer1[256] = {0};
        int read_result1 = read_file(read_file1, buffer1);
        printf("       Lectura: %s\n", read_result1 ? "Éxito" : "Fallo");
        printf("       Contenido: %s\n", buffer1);
        close_file(read_file1);
    } else {
        printf("       Error: No se pudo abrir log.txt para lectura\n");
    }
    
    // Leer config.ini
    printf("   8.2. Leyendo 'config.ini':\n");
    osmFile* read_file2 = open_file(process_id, "config.ini", 'r');
    if (read_file2) {
        char buffer2[256] = {0};
        int read_result2 = read_file(read_file2, buffer2);
        printf("       Lectura: %s\n", read_result2 ? "Éxito" : "Fallo");
        printf("       Contenido: %s\n", buffer2);
        close_file(read_file2);
    } else {
        printf("       Error: No se pudo abrir config.ini para lectura\n");
    }
    
    // Eliminar un archivo
    printf("\n9. Eliminando archivo 'config.ini':\n");
    delete_file(process_id, "config.ini");
    printf("   Archivo eliminado\n");
    
    // Listar archivos después de eliminar
    printf("\n10. Archivos después de eliminar config.ini:\n");
    list_files(process_id);
    
    // Verificar estado del bitmap
    printf("\n11. Estado del bitmap de frames:\n");
    frame_bitmap_status();
    
    // Finalizar proceso
    printf("\n12. Finalizando proceso:\n");
    int finish_result = finish_process(process_id);
    printf("    Proceso finalizado: %s\n", finish_result == 0 ? "Éxito" : "Fallo");    
    // Estado final
    printf("\n13. Estado final:\n");
    list_processes();
    frame_bitmap_status();
    
    return 0;
}