/*
garcia chavez erik  01275863
Armando Tepale Chocolat  1280222
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




int main(int argc, char *argv[]) {

    //comprueba que el comando haya recibido 2 parametrso 
    if (argc < 2) {
        write(1, "Uso: ./borrar-rastro <archivo>\n", 32);
        return 1;
    }
    
    const char *archivo = argv[1];

    struct stat info;
    

    if (stat(archivo, &info) < 0) {
        write(2, "Error: archivo no encontrado\n", 29);
        return 1;
    }
    if (!S_ISREG(info.st_mode)) {

        write(2, "Error: no es un archivo normal\n", 31);
        return 1;
    }
    
    
    int fd = open(archivo, O_WRONLY);
    if (fd < 0) {
        write(2, "Error: no se puede abrir\n", 25);
        return 1;
    }
    
    int random = open("/dev/urandom", O_RDONLY);

    
    if (random < 0) {
        write(2, "Error: sistema de aleatoriedad\n", 31);
        close(fd);
        return 1;
    }
    
    char buffer[4096]; 
    int tamaño = info.st_size; 
    
    write(1, "Borrando archivo...\n", 20);
    
    // Hacer 3 pasadas de sobrescritura

    for (int vuelta = 1; vuelta <= 3; vuelta++) {
        lseek(fd, 0, SEEK_SET);
        int restante = tamaño;
        
       /**
        * cunado termine de llenar el archivo con contenido random
        * es cunado sale del ciclo while y forza la escritura 
        * en el disco en el cual se daran 7 vueltas 
        */
        while (restante > 0) {
            int bloque = (restante > 4096) ? 4096 : restante;
            read(random, buffer, bloque);
            write(fd, buffer, bloque);
            restante -= bloque;
        }
       
        fsync(fd);  
    }
    
    //cerramos ambos archivos 
    close(random);
    close(fd);
    
    if (unlink(archivo) != 0) {
        write(2, "Error: no se pudo borrar\n", 25);
        return 1;
    }
    
    write(1, "Archivo borrado con exito\n", 26);
    return 0;
}

