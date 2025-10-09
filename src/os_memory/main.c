#include <stdio.h>
#include "../os_memory_API/os_memory_API.h"

int main(int argc, char const *argv[]) {
  // Verificar que se pasó un archivo de memoria como argumento
  if (argc != 2) {
    printf("Uso: %s <archivo_memoria.bin>\n", argv[0]);
    return 1;
  }
  
  // Montar la memoria
  mount_memory((char *)argv[1]);
  
  // Aquí puedes agregar más funciones de prueba
  
  return 0;
}