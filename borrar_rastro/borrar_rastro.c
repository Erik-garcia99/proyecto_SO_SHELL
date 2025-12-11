/*
Garcia Chavez Erik  01275863
Armando Tepale Chocolatl  1280222
Sistemas operativos 2025-2
proyecto <adromeda_shell>
ingenieria en computacion 
UABC
*/
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>
#include <stdio.h>

int borrar_archivo(const char *archivo) {
    struct stat info;
    
    // Verificar que existe y es archivo regular
    if (stat(archivo, &info) < 0 || !S_ISREG(info.st_mode)) {
        return -1;
    }
    
    int fd = open(archivo, O_WRONLY);
    if (fd < 0) {
        return -1;
    }
    
    // Abrir generador de datos aleatorios
    int random = open("/dev/urandom", O_RDONLY);
    if (random < 0) {
        close(fd);
        return -1;
    }
    
    char buffer[4096];
    int tamaño = info.st_size;
    
    // Sobrescribir 3 veces con datos aleatorios
    for (int vuelta = 1; vuelta <= 3; vuelta++) {
        lseek(fd, 0, SEEK_SET);  // Volver al inicio
        int restante = tamaño;
        
        while (restante > 0) {
            int bloque = (restante > 4096) ? 4096 : restante;
            read(random, buffer, bloque);
            write(fd, buffer, bloque);
            restante -= bloque;
        }
        
        fsync(fd);  // Forzar escritura fisica al disco
    }
    
    close(random);
    close(fd);
    
    // Eliminar archivo del sistema
    if (unlink(archivo) != 0) {
        return -1;
    }
    
    return 0;
}

int borrar_directorio(const char *ruta) {
    DIR *dir = opendir(ruta);
    if (!dir) return -1;
    
    struct dirent *entrada;
    char ruta_completa[1024];
    
    // Procesar cada entrada del directorio
    while ((entrada = readdir(dir)) != NULL) {
        // Ignorar . y ..
        if (strcmp(entrada->d_name, ".") == 0 || strcmp(entrada->d_name, "..") == 0) {
            continue;
        }
        
        snprintf(ruta_completa, sizeof(ruta_completa), "%s/%s", ruta, entrada->d_name);
        
        struct stat info;
        if (stat(ruta_completa, &info) < 0) continue;
        
        // Recursion para subdirectorios
        if (S_ISDIR(info.st_mode)) {
            borrar_directorio(ruta_completa);
        } else if (S_ISREG(info.st_mode)) {
            borrar_archivo(ruta_completa);
        }
    }
    
    closedir(dir);
    rmdir(ruta);  // Eliminar directorio vacio
    
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        write(1, "Uso: ./borrar-rastro <archivos_o_directorios>\n", 47);
        return 1;
    }
    
    write(1, "Iniciando borrado seguro...\n", 28);
    write(1, "Borrando: ", 10);
    
    // Mostrar archivos/directorios a borrar
    for (int i = 1; i < argc; i++) {
        write(1, argv[i], strlen(argv[i]));
        if (i < argc - 1) {
            write(1, ", ", 2);
        }
    }
    write(1, "\n\n", 2);
    
    // Procesar cada argumento
    for (int i = 1; i < argc; i++) {
        struct stat info;
        
        if (stat(argv[i], &info) < 0) {
            write(2, "Error: ", 7);
            write(2, argv[i], strlen(argv[i]));
            write(2, " no encontrado\n", 15);
            continue;
        }
        
        if (S_ISDIR(info.st_mode)) {
            // Mostrar contenido del directorio
            write(1, "Directorio: ", 12);
            write(1, argv[i], strlen(argv[i]));
            write(1, "\n", 1);
            
            DIR *dir = opendir(argv[i]);
            if (dir) {
                struct dirent *entrada;
                write(1, "Contenido: ", 11);
                int primera = 1;
                while ((entrada = readdir(dir)) != NULL) {
                    if (strcmp(entrada->d_name, ".") == 0 || strcmp(entrada->d_name, "..") == 0) {
                        continue;
                    }
                    if (!primera) write(1, ", ", 2);
                    write(1, entrada->d_name, strlen(entrada->d_name));
                    primera = 0;
                }
                closedir(dir);
                write(1, "\n", 1);
            }
            
            borrar_directorio(argv[i]);
        } else if (S_ISREG(info.st_mode)) {
            borrar_archivo(argv[i]);
        }
    }
    
    // Confirmacion final
    write(1, "\nEliminado: ", 12);
    for (int i = 1; i < argc; i++) {
        write(1, argv[i], strlen(argv[i]));
        if (i < argc - 1) {
            write(1, ", ", 2);
        }
    }
    write(1, "\n", 1);
    
    write(1, "Archivos borrados con exito\n", 28);
    
    return 0;
}