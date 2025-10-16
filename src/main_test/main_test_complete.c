#include <stdio.h>
#include <string.h>
#include "../os_memory_API/os_memory_API.h"

int main(int argc, char const *argv[]) {
    printf("=== PRUEBA COMPLETA INTEGRADA ===\n");
    
    // Verificar que se pasó un archivo de memoria como argumento
    if (argc != 2) {
        printf("Uso: %s <archivo_memoria.bin>\n", argv[0]);
        return 1;
    }
    
    // Montar la memoria
    printf("\n1. Montando memoria desde: %s\n", argv[1]);
    mount_memory((char *)argv[1]);
    clear_all_processes(); // Limpiar procesos preexistentes
    
    // Estado inicial completo
    printf("\n2. Estado inicial completo:\n");
    printf("   Procesos existentes:\n");
    list_processes();
    printf("   PCB slots disponibles: %d\n", processes_slots());
    printf("   Estado del bitmap:\n");
    frame_bitmap_status();
    
    // Escenario: Sistema multiusuario con múltiples procesos y archivos
    printf("\n3. Simulando sistema multiusuario:\n");
    
    // Usuario 1: Sistema operativo (PID 1)
    printf("   3.1. Creando proceso del sistema (PID 1):\n");
    start_process(1, "kernel");
    
    // Usuario 2: Navegador web (PID 2)
    printf("   3.2. Creando navegador web (PID 100):\n");
    start_process(2, "firefox");
    
    // Usuario 3: Editor de texto (PID 3)
    printf("   3.3. Creando editor de texto (PID 200):\n");
    start_process(3, "notepad");
    
    // Usuario 4: Juego (PID 4)
    printf("   3.4. Creando juego (PID 4):\n");
    start_process(4, "tetris");
    
    printf("\n4. Estado después de crear procesos:\n");
    list_processes();
    printf("   PCB slots disponibles: %d\n", processes_slots());
    
    // Cada proceso crea archivos
    printf("\n5. Cada proceso crea sus archivos:\n");
    
    // Proceso kernel crea archivos de sistema
    printf("   5.1. Kernel crea archivos de sistema:\n");
    osmFile* system_log = open_file(1, "system.log", 'w');
    if (system_log) {
        char sys_data[] = "[KERNEL] Sistema iniciado correctamente";
        system_log->size = strlen(sys_data) + 1;
        write_file(system_log, sys_data);
        close_file(system_log);
        printf("       system.log creado\n");
    }
    
    osmFile* boot_cfg = open_file(1, "boot.cfg", 'w');
    if (boot_cfg) {
        char boot_data[] = "timeout=5\ndefault=ubuntu";
        boot_cfg->size = strlen(boot_data) + 1;
        write_file(boot_cfg, boot_data);
        close_file(boot_cfg);
        printf("       boot.cfg creado\n");
    }
    
    // Proceso firefox crea archivos web
    printf("   5.2. Firefox crea archivos web:\n");
    osmFile* history = open_file(2, "history.db", 'w');
    if (history) {
        char hist_data[] = "google.com,youtube.com,github.com";
        history->size = strlen(hist_data) + 1;
        write_file(history, hist_data);
        close_file(history);
        printf("       history.db creado\n");
    }
    
    osmFile* bookmarks = open_file(2, "bookmarks.json", 'w');
    if (bookmarks) {
        char book_data[] = "{\"favorites\":[\"stackoverflow.com\",\"reddit.com\"]}";
        bookmarks->size = strlen(book_data) + 1;
        write_file(bookmarks, book_data);
        close_file(bookmarks);
        printf("       bookmarks.json creado\n");
    }
    
    // Proceso notepad crea documentos
    printf("   5.3. Notepad crea documentos:\n");
    osmFile* document = open_file(3, "documento.txt", 'w');
    if (document) {
        char doc_data[] = "Este es mi documento importante.";
        document->size = strlen(doc_data) + 1;
        write_file(document, doc_data);
        close_file(document);
        printf("       documento.txt creado\n");
    }
    
    // Proceso tetris crea save game
    printf("   5.4. Tetris crea save game:\n");
    osmFile* savegame = open_file(4, "savegame.dat", 'w');
    if (savegame) {
        char save_data[] = "level=5,score=12450,lines=23";
        savegame->size = strlen(save_data) + 1;
        write_file(savegame, save_data);
        close_file(savegame);
        printf("       savegame.dat creado\n");
    }
    
    // Mostrar archivos de cada proceso
    printf("\n6. Archivos de cada proceso:\n");
    printf("   6.1. Archivos del kernel (PID 1):\n");
    list_files(1);
    printf("   6.2. Archivos de firefox (PID 2):\n");
    list_files(2);
    printf("   6.3. Archivos de notepad (PID 3):\n");
    list_files(3);
    printf("   6.4. Archivos de tetris (PID 4):\n");
    list_files(4);
    
    // Estado de memoria después de crear archivos
    printf("\n7. Estado de memoria después de crear archivos:\n");
    frame_bitmap_status();
    
    // Simular que el usuario cierra firefox
    printf("\n8. Usuario cierra firefox (PID 100):\n");
    finish_process(100);
    printf("   Firefox cerrado\n");
    
    // Verificar que los archivos de firefox se eliminaron
    printf("\n9. Estado después de cerrar firefox:\n");
    list_processes();
    printf("   Estado de memoria:\n");
    frame_bitmap_status();
    
    // El usuario lee un documento en notepad
    printf("\n10. Usuario lee documento en notepad:\n");
    osmFile* read_doc = open_file(3, "documento.txt", 'r');
    if (read_doc) {
        char buffer[256] = {0};
        read_file(read_doc, buffer);
        printf("    Contenido leído: %s\n", buffer);
        close_file(read_doc);
    }
    
    // El usuario actualiza el save game en tetris
    printf("\n11. Usuario actualiza save game en tetris:\n");
    delete_file(4, "savegame.dat");  // Elimina el save anterior
    osmFile* new_save = open_file(4, "savegame.dat", 'w');
    if (new_save) {
        char new_save_data[] = "level=7,score=28930,lines=45";
        new_save->size = strlen(new_save_data) + 1;
        write_file(new_save, new_save_data);
        close_file(new_save);
        printf("    Save game actualizado\n");
    }
    
    // Estado final antes de limpiar
    printf("\n12. Estado final antes de limpiar:\n");
    list_processes();
    frame_bitmap_status();
    
    // Apagar sistema (cerrar todos los procesos)
    printf("\n13. Apagando sistema (cerrando todos los procesos):\n");
    int closed = clear_all_processes();
    printf("    Procesos cerrados: %d\n", closed);
    
    // Estado completamente limpio
    printf("\n14. Estado final limpio:\n");
    list_processes();
    frame_bitmap_status();
    
    return 0;
}