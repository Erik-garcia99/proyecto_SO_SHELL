#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DB_FILE ".integrity.db"
#define MAX_LINE 2048

// Función simple de hash (DJB2) para evitar dependencias complejas como OpenSSL
// Suma byte por byte para crear una firma única del archivo.
unsigned long hash_file(const char *filename) {
    FILE *f = fopen(filename, "rb");
    if (!f) return 0;

    unsigned long hash = 5381;
    int c;

    while ((c = fgetc(f)) != EOF) {
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }

    fclose(f);
    return hash;
}

// Busca el hash previo de un archivo en la base de datos
unsigned long get_previous_hash(const char *target_file) {
    FILE *db = fopen(DB_FILE, "r");
    if (!db) return 0;

    char line[MAX_LINE];
    char name[1024];
    unsigned long stored_hash;
    unsigned long found_hash = 0;

    while (fgets(line, sizeof(line), db)) {
        // Formato esperado: NOMBRE_ARCHIVO HASH
        sscanf(line, "%s %lu", name, &stored_hash);
        if (strcmp(name, target_file) == 0) {
            found_hash = stored_hash;
            break;
        }
    }
    fclose(db);
    return found_hash;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Uso: verificador-integridad <archivo1> <archivo2> ...\n");
        return 1;
    }

    // Abrimos la DB antigua para leer y creamos una temporal para el nuevo estado
    // Esto cumple con el requisito de "Cada ejecución genera un archivo de sumas" 
    FILE *new_db = fopen(DB_FILE ".tmp", "w");
    if (!new_db) {
        perror("Error creando base de datos temporal");
        return 1;
    }

    // Si existe la DB anterior, la mantenemos para comparar, si no, se asume primera ejecución.
    FILE *old_db_check = fopen(DB_FILE, "r");
    int first_run = (old_db_check == NULL);
    if (old_db_check) fclose(old_db_check);

    printf("=== Verificador de Integridad ===\n");
    if(first_run) printf("[INFO] Primera ejecución: Creando base de datos de integridad.\n");

    int alertas = 0;

    // Procesar cada archivo pasado como argumento
    for (int i = 1; i < argc; i++) {
        char *filename = argv[i];
        unsigned long current_hash = hash_file(filename);

        if (current_hash == 0) {
            printf("[-] %s: Error al leer archivo (o no existe).\n", filename);
            continue;
        }

        // Guardamos el nuevo estado en la DB temporal
        fprintf(new_db, "%s %lu\n", filename, current_hash);

        if (!first_run) {
            unsigned long prev_hash = get_previous_hash(filename);
            
            if (prev_hash == 0) {
                printf("[+] %s: NUEVO archivo registrado.\n", filename);
            } else if (current_hash != prev_hash) {
                printf("[!] %s: ALERTA DE INTEGRIDAD - El archivo fue modificado.\n", filename);
                alertas++;
            } else {
                printf("[OK] %s: Verificado correctamente.\n", filename);
            }
        } else {
            printf("[+] %s: Registrado en DB.\n", filename);
        }
    }

    fclose(new_db);

    // Actualizamos la base de datos real con la temporal
    remove(DB_FILE);
    rename(DB_FILE ".tmp", DB_FILE);

    return alertas > 0 ? 1 : 0;
}
