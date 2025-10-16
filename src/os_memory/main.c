#include <stdio.h>
#include <string.h>
#include "../os_memory_API/os_memory_API.h"

int main(int argc, char const *argv[]) {
    // Verificar que se pasó un archivo de memoria como argumento
    if (argc != 2) {
        printf("Uso: %s <archivo_memoria.bin>\n", argv[0]);
        return 1;
    }

    // Montar la memoria
    mount_memory((char *)argv[1]); // Monta memoria

    // Crear procesos
    start_process(10, "ventas"); // Crea proceso 10
    start_process(20, "reportes"); // Crea proceso 20

    // Listar procesos
    list_processes(); // Lista procesos

    // Mostrar slots libres de PCB
    int pcb_libres = processes_slots(); // PCB slots libres
    printf("PCB free slots: %d\n", pcb_libres);

    // Mostrar slots libres en la tabla de archivos
    int slots_arch = file_table_slots(10); // Slots archivo libres
    printf("File-table free slots (pid=10): %d\n", slots_arch);

    // Operaciones con archivos
    osmFile* f = open_file(10, "log.txt", 'w'); // Abrir/crear archivo (w)
    if (f) {
        write_file(f, "log_local.txt"); // Escribir desde local
        close_file(f); // Cierra descriptor
    }

    f = open_file(10, "log.txt", 'r'); // Reabrir lectura
    if (f) {
        read_file(f, "log_copia.txt"); // Copiar a local
        close_file(f); // Cierra descriptor
    }

    list_files(10); // Listar archivos (hex)

    // Estado del bitmap de marcos
    frame_bitmap_status(); // Bitmap: usados/libres

    // Eliminar archivo y finalizar proceso
    delete_file(10, "log.txt"); // Eliminar archivo p10
    finish_process(10); // Terminar proceso 10

    // Cerrar todos los procesos
    int cerrados = clear_all_processes(); // Cerrar todos
    printf("Procesos cerrados por clear_all_processes(): %d\n", cerrados);

    return 0;
}