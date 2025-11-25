
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        write(1, "Uso: ./borrar-rastro <archivo>\n", 32);
        return 1;
    }
    
    const char *archivo = argv[1];
    struct stat info;
    
    // Verificar que el archivo existe
    if (stat(archivo, &info) < 0) {
        write(2, "Error: archivo no encontrado\n", 29);
        return 1;
    }
    
    // Verificar que es un archivo normal
    if (!S_ISREG(info.st_mode)) {
        write(2, "Error: no es un archivo normal\n", 31);
        return 1;
    }
    
    // Abrir el archivo
    int fd = open(archivo, O_WRONLY);
    if (fd < 0) {
        write(2, "Error: no se puede abrir\n", 25);
        return 1;
    }
    
    // Abrir generador de datos aleatorios
    int random = open("/dev/urandom", O_RDONLY);
    if (random < 0) {
        write(2, "Error: sistema de aleatoriedad\n", 31);
        close(fd);
        return 1;
    }
    
    char buffer[4096];
    off_t tamaño = info.st_size;
    
    write(1, "Borrando archivo...\n", 20);
    
    // Hacer 3 pasadas de sobrescritura
    for (int vuelta = 1; vuelta <= 3; vuelta++) {
        lseek(fd, 0, SEEK_SET);
        off_t restante = tamaño;
        
        while (restante > 0) {
            int bloque = (restante > 4096) ? 4096 : restante;
            read(random, buffer, bloque);
            write(fd, buffer, bloque);
            restante -= bloque;
        }
        
        fsync(fd);  // Asegurar que se escribió al disco
    }
    
    close(random);
    close(fd);
    
    // Borrar el archivo
    if (unlink(archivo) != 0) {
        write(2, "Error: no se pudo borrar\n", 25);
        return 1;
    }
    
    write(1, "Archivo borrado con exito\n", 26);
    return 0;
}
