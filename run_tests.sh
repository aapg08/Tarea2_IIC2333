#!/bin/bash

# Script para ejecutar las diferentes pruebas del sistema de memoria
# Uso: ./run_tests.sh [test_name] [memory_file]

TESTS_DIR="src/os_memory"
ORIGINAL_MAIN="$TESTS_DIR/main.c"
BACKUP_MAIN="$TESTS_DIR/main_original.c"

# Función para mostrar ayuda
show_help() {
    echo "Uso: $0 [test_name] [memory_file]"
    echo ""
    echo "Tests disponibles:"
    echo "  basic     - Prueba funciones básicas"
    echo "  processes - Prueba gestión de procesos"
    echo "  files     - Prueba gestión de archivos"
    echo "  complete  - Prueba completa integrada"
    echo "  limits    - Prueba casos límite y errores"
    echo "  all       - Ejecuta todas las pruebas"
    echo ""
    echo "Ejemplo:"
    echo "  $0 basic memorias/memoria_vacia.bin"
    echo "  $0 all memorias/memoria_vacia.bin"
}

# Función para hacer backup del main original
backup_original() {
    if [ -f "$ORIGINAL_MAIN" ] && [ ! -f "$BACKUP_MAIN" ]; then
        echo "Haciendo backup del main original..."
        cp "$ORIGINAL_MAIN" "$BACKUP_MAIN"
    fi
}

# Función para restaurar el main original
restore_original() {
    if [ -f "$BACKUP_MAIN" ]; then
        echo "Restaurando main original..."
        cp "$BACKUP_MAIN" "$ORIGINAL_MAIN"
    fi
}

# Función para ejecutar una prueba específica
run_test() {
    local test_name=$1
    local memory_file=$2
    local test_file="$TESTS_DIR/main_test_${test_name}.c"
    
    if [ ! -f "$test_file" ]; then
        echo "Error: Archivo de prueba $test_file no encontrado"
        return 1
    fi
    
    echo "=========================================="
    echo "Ejecutando prueba: $test_name"
    echo "Archivo de memoria: $memory_file"
    echo "=========================================="
    
    # Copiar el archivo de prueba como main.c
    cp "$test_file" "$ORIGINAL_MAIN"
    
    # Compilar
    echo "Compilando..."
    make clean > /dev/null 2>&1
    if make > /dev/null 2>&1; then
        echo "Compilación exitosa"
        echo ""
        
        # Ejecutar
        if [ -f "./os_memory" ]; then
            ./os_memory "$memory_file"
            echo ""
            echo "Prueba $test_name completada"
        else
            echo "Error: Ejecutable os_memory no encontrado"
        fi
    else
        echo "Error en la compilación"
        make  # Mostrar errores de compilación
    fi
    
    echo ""
    echo "Presiona Enter para continuar..."
    read
}

# Verificar argumentos
if [ $# -lt 2 ]; then
    show_help
    exit 1
fi

TEST_NAME=$1
MEMORY_FILE=$2

# Verificar que el archivo de memoria existe
if [ ! -f "$MEMORY_FILE" ]; then
    echo "Error: Archivo de memoria $MEMORY_FILE no encontrado"
    exit 1
fi

# Hacer backup del main original
backup_original

# Ejecutar pruebas según el parámetro
case $TEST_NAME in
    "basic")
        run_test "basic" "$MEMORY_FILE"
        ;;
    "processes")
        run_test "processes" "$MEMORY_FILE"
        ;;
    "files")
        run_test "files" "$MEMORY_FILE"
        ;;
    "complete")
        run_test "complete" "$MEMORY_FILE"
        ;;
    "limits")
        run_test "limits" "$MEMORY_FILE"
        ;;
    "all")
        echo "Ejecutando todas las pruebas..."
        run_test "basic" "$MEMORY_FILE"
        run_test "processes" "$MEMORY_FILE"
        run_test "files" "$MEMORY_FILE"
        run_test "complete" "$MEMORY_FILE"
        run_test "limits" "$MEMORY_FILE"
        ;;
    *)
        echo "Error: Prueba desconocida '$TEST_NAME'"
        show_help
        exit 1
        ;;
esac

# Restaurar el main original
restore_original

echo "Todas las pruebas completadas. Main original restaurado."