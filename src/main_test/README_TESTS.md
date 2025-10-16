# Archivos de Prueba para el Sistema de Memoria

Este directorio contiene varios archivos main para probar diferentes aspectos del sistema de gestión de memoria.

## Archivos de Prueba

### 1. `main_test_basic.c`
**Propósito**: Prueba las funciones básicas del sistema
- Montar memoria
- Verificar estado inicial
- Mostrar PCB slots disponibles
- Mostrar estado del bitmap de frames

**Uso**: Para verificar que el sistema monta correctamente y muestra el estado inicial.

### 2. `main_test_processes.c`
**Propósito**: Prueba completa de gestión de procesos
- Crear múltiples procesos
- Verificar IDs duplicados
- Finalizar procesos individuales
- Limpiar todos los procesos
- Verificar slots de PCB

**Uso**: Para probar todas las operaciones relacionadas con procesos.

### 3. `main_test_files.c`
**Propósito**: Prueba completa de gestión de archivos
- Crear archivos con diferentes contenidos
- Escribir y leer archivos
- Eliminar archivos
- Verificar tabla de archivos
- Probar diferentes tipos de contenido (texto, CSV, INI)

**Uso**: Para probar todas las operaciones de archivos con un solo proceso.

### 4. `main_test_complete.c`
**Propósito**: Simulación completa de un sistema multiusuario
- Múltiples procesos simultáneos
- Cada proceso con sus propios archivos
- Simulación de uso real (kernel, navegador, editor, juego)
- Actualización de archivos existentes
- Cierre gradual de procesos

**Uso**: Para probar el sistema en un escenario realista con múltiples procesos y archivos.

### 5. `main_test_limits.c`
**Propósito**: Prueba de casos límite y manejo de errores
- Operaciones con procesos inexistentes
- Agotar tabla de archivos
- Archivos inexistentes
- IDs de proceso duplicados
- Nombres de archivo especiales
- Agotar memoria disponible

**Uso**: Para verificar que el sistema maneja correctamente situaciones de error y límites.

## Cómo Compilar y Ejecutar

Para cada archivo de prueba, necesitarás compilarlo junto con tu implementación. Asumiendo que tienes un Makefile:

```bash
# Para main_test_basic.c
make clean
cp src/os_memory/main_test_basic.c src/os_memory/main.c
make
./os_memory memorias/memoria_vacia.bin

# Para main_test_processes.c
make clean
cp src/os_memory/main_test_processes.c src/os_memory/main.c
make
./os_memory memorias/memoria_vacia.bin

# Y así sucesivamente para cada archivo de prueba...
```

## Memorias de Prueba Recomendadas

### Memoria Vacía
Usa una memoria completamente vacía para probar la funcionalidad desde cero.

### Memoria con Datos Existentes
Si tienes archivos de memoria con datos pre-existentes, úsalos especialmente con `main_test_basic.c` para ver cómo el sistema maneja datos existentes.

## Qué Observar en Cada Prueba

### Salida Normal Esperada
- Los procesos se crean exitosamente
- Los archivos se escriben y leen correctamente
- Los PCB slots disminuyen al crear procesos y aumentan al eliminarlos
- El bitmap de frames muestra cambios cuando se crean/eliminan archivos
- Las operaciones inválidas fallan correctamente

### Señales de Problemas
- Procesos que no se crean cuando deberían
- Archivos que no se escriben o leen correctamente
- Contenido corrupto en los archivos
- Memory leaks (PCB slots o frames que no se liberan)
- Crashes o comportamiento inesperado

## Consejos para Debugging

1. **Ejecuta las pruebas en orden**: Empieza con `basic`, luego `processes`, luego `files`, etc.

2. **Compara salidas**: Si una prueba falla, compara su salida con lo que esperarías ver.

3. **Usa diferentes memorias**: Prueba tanto con memorias vacías como con memorias que ya tienen datos.

4. **Verifica la limpieza**: Después de cada prueba, verifica que todos los recursos se liberen correctamente.

5. **Modifica las pruebas**: Siéntete libre de modificar los archivos de prueba para casos específicos que quieras probar.