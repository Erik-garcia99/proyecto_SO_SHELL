
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

int main(int argc, char *argv[]) {

    //comprueba que el comando haya recibido 2 parametrso 
    if (argc < 2) {
        write(1, "Uso: ./borrar-rastro <archivo>\n", 32);
        return 1;
    }
    
    //se esta trayendo el nombre del archivo 
    const char *archivo = argv[1];

    struct stat info;
    
    // para conocer exista la funcion regresa un 0 de lo contrario regresa un -1
    if (stat(archivo, &info) < 0) {
        write(2, "Error: archivo no encontrado\n", 29);
        return 1;
    }
    if (!S_ISREG(info.st_mode)) {

        write(2, "Error: no es un archivo normal\n", 31);
        return 1;
    }
    
    // abrmos el archivo 
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
    off_t tamaño = info.st_size; 
    
    write(1, "Borrando archivo...\n", 20);
    
    // Hacer 3 pasadas de sobrescritura
    for (int vuelta = 1; vuelta <= 3; vuelta++) {
        //repositiona el puntero del archivo al inicio
        lseek(fd, 0, SEEK_SET);
        //lleva la cuenta de cunatos bytes faltan por escribir 
        off_t restante = tamaño;
        
    
        while (restante > 0) {
           
            int bloque = (restante > 4096) ? 4096 : restante;
          
            read(random, buffer, bloque);
           s en el archivo que se esta haciendo el proceso de limpiar 
            write(fd, buffer, bloque);
            //vamos restando los 4KB al peso total del archivo
            restante -= bloque;
        }
        fsync(fd);  // Asegurar que se escribió al disco
    }
    
    //cerramos ambos archivos 
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

