
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
    //la funcion stat retorna infromacion sobre un archivo o directroio 
    //a traves de un argumento de salida. 
    /**
     * 
     * el arguemnto en donde se guarda la infromacion 
     * 
     */
    struct stat info;
    
    // para conocer exista la funcion regresa un 0 de lo contrario regresa un -1
    if (stat(archivo, &info) < 0) {
        write(2, "Error: archivo no encontrado\n", 29);
        return 1;
    }
    
    /**
     * miembros que nos podrian importar de la estrucutra 
     * 
     * -> st_mode -  
     * ->st_size - para archivos regulaes el tamanio del archivo en bytes
     * 
     * la macro !S_ISREG es un numero diferente a 0 cunado el archivo es uno regular 
     * que no es un directorio por ejemplo 
     * 
     * entonces se pone negado porque, porque no nos interesa saber si es regular, 
     * nos interesa mas bien si el archivo es otro tipo por ejemplo una carpeta en este caso 
     * si es una carpeta indicara < 0 > por lo que lo negamos para cunado esto paso el reusltado 
     * sea 1 y entonces entre en la condiccion en el otro caso es 0 y no entra por lo que es un archivo regular 
     * 
     */
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
    
    // vamos a tratar de leer bytes del dispositivo espcial de linux que nos genera numeros randoms 
    int random = open("/dev/urandom", O_RDONLY);

    //en caso que falle cerramos el archivo e indicamos el susceso pero en sistemas linux actuales no
    //es tan comun que esto suceda
    if (random < 0) {
        write(2, "Error: sistema de aleatoriedad\n", 31);
        close(fd);
        return 1;
    }
    
    char buffer[4096]; //el buffer en donde se estasn extrayendo y leyedno los datos a escribir y lerr del archivo
    //obtenemos el tamanio del archivo orignial
    //como tenemos un archivo regular nos dara el numero de bytes del archivo
    off_t tamaño = info.st_size; 
    
    write(1, "Borrando archivo...\n", 20);
    
    // Hacer 3 pasadas de sobrescritura
    for (int vuelta = 1; vuelta <= 3; vuelta++) {
        //repositiona el puntero del archivo al inicio
        lseek(fd, 0, SEEK_SET);
        //lleva la cuenta de cunatos bytes faltan por escribir 
        off_t restante = tamaño;
        
        //debe de escirbir todo el archivo por lo que no sabemos cunato es por lo que lo indicamos en un ciclo while
        while (restante > 0) {
            //preguntamos si el restante es mayor a los 4KB a escirbir si es asi, entonces asignamos los 4KB 
            //si no asignamos lo que resta a escribir 
            int bloque = (restante > 4096) ? 4096 : restante;
            //va a lleer el total de bytes que dados por bloque del archivo random, los guardara en el buffer
            read(random, buffer, bloque);
            //estos mismos bytes leidos los escribimos en el archivo que se esta haciendo el proceso de limpiar 
            write(fd, buffer, bloque);
            //vamos restando los 4KB al peso total del archivo
            restante -= bloque;
        }
        //con esta funcion nos aseguramos que se vacie el cahce del kernel al disco fisico sin estos los datos 
        //podrian quedarse en RAM. 
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
