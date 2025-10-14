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
  mount_memory((char *)argv[1]);

  clear_all_processes();
  printf("Antes de start_process\n");
  // debug_print_pcbs();
  start_process(10, "ventas"); // Crea proceso 10
  printf("Antes de start_process\n");
  // debug_print_pcbs();
  start_process(20, "reportes"); // Crea proceso 20
  printf("Antes de list_process\n");
  // debug_print_pcbs();
  list_processes(); // Lista procesos

  printf("Antes de process_slots\n");
  int pcb_libres = processes_slots();
  printf("PCB free slots: %d\n", pcb_libres);

  printf("Antes de file_table_slots\n");
  int slots_arch = file_table_slots(10);
  printf("File-table free slots (pid=10): %d\n", slots_arch);

  printf("Antes de list_files (para ver archivos existentes)\n");
  list_files(10);
  printf("Antes de delete_file (asegurar slot libre para log.txt)\n");
  delete_file(10, "log.txt"); // Eliminar archivo si existe
  printf("Después de delete_file\n");
  printf("Antes de open_file (creación log.txt)\n");
  osmFile* f = open_file(10, "log.txt", 'w'); // Abrir/crear archivo (w)
  if (!f) {
    printf("Error: open_file retornó NULL (creación log.txt)\n");
    return 1;
  }
  printf("open_file (creación log.txt) exitoso\n");
  // Simula escribir datos: crea un buffer con datos
  char datos[100] = "Este es un log de ventas.";
  f->size = 100; // Simula tamaño a escribir
  debug_print_pcbs();
  printf("Antes de write_file\n");
  write_file(f, datos); // Escribe datos
  printf("Después de write_file\n");
  printf("Antes de close_file\n");
  close_file(f);

  printf("Antes de open_file\n");
  f = open_file(10, "log.txt", 'r'); // Reabrir lectura
  if (!f) {
    printf("Error: open_file (lectura) retornó NULL\n");
    return 1;
  }
  char copia[100] = {0};
  printf("Antes de read_file\n");
  read_file(f, copia); // Copia a buffer local
  printf("Contenido leído: %s\n", copia);
  // Verificación automática
  if (strcmp(copia, "Este es un log de ventas.") == 0) {
    printf("[OK] El contenido leído es correcto.\n");
  } else {
    printf("[ERROR] El contenido leído NO coincide con lo esperado.\n");
  }
  printf("Antes de close_file\n");
  close_file(f);
  printf("Antes de list_files\n");
  list_files(10); // Listar archivos
  printf("Antes de frame_bitmap_status\n");
  frame_bitmap_status(); // Bitmap: usados/libres
  printf("Antes de delete_file\n");
  delete_file(10, "log.txt"); // Eliminar archivo p10

  printf("Antes de finish_process\n");
  finish_process(10); // Terminar proceso 10

  printf("Antes de clear_all_processes\n");
  int cerrados = clear_all_processes(); // Cerrar todos
  printf("Procesos cerrados por clear_all_processes(): %d\n", cerrados);

  
  return 0;
}