#include <fcntl.h>      
#include <unistd.h>     
#include <stdlib.h>  
#include <string.h>    
#include <stdio.h>   
#include <sys/stat.h> 
#include <sys/types.h>  
#include <errno.h>

#define BUFFER_SIZE 4096
//creacion de una caprtea donde se guardara la db de vetificador 
#define verificador_padre "/.andromeda_shell/" 
#define verificador_final ".verificador/"
#define TAM_RUTA_BASE 2048
#define TAM_RUTA_FINAL 2100
char RUTA_DB[TAM_RUTA_BASE];
char RUTA_DB_TMP[TAM_RUTA_FINAL];


void print_msg(const char *msg);
int leer_linea(int fd, char *buffer, int max_len);
unsigned long hash_file(const char *filename);
unsigned long obetner_hash_previo(const char *target_file);
void crear_directorio_verificador();


int main(int argc, char *argv[]) {
    //verifica que se le hayan ingresao correcto lso parametro 
    if (argc < 2) {
        char *msg = "Uso: verificador-integridad <archivo1> <archivo2> ...\n";
        write(STDERR_FILENO, msg, strlen(msg));
        return 1;
    }

    crear_directorio_verificador();

    //Crear/Truncar archivo temporal con permisos 0644 (rw-r--r--) del estado actual de los archivos 
    int fd_new = open(RUTA_DB_TMP, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd_new < 0) {
        char *err = "Error creando base de datos temporal\n";
        write(STDERR_FILENO, err, strlen(err));
        return 1;
    }

    //verifica si existe la basde de datos, solo verifica mas no hace nada con ella 
    int fd_old = open(RUTA_DB, O_RDONLY);
    int first_run = (fd_old < 0);
    if (fd_old >= 0) close(fd_old);


    int alertas = 0;
    char out_buf[1024]; //buffer para formatear mensajes de salida

    for (int i = 1; i < argc; i++) {
        char *filename = argv[i];
        //calcula el hash actial
        unsigned long current_hash = hash_file(filename);
        //control de errores si no existe no el archivo o no se puede ller
        if (current_hash == 0) {
            sprintf(out_buf, "%s: Error al leer archivo.\n", filename);
            print_msg(out_buf);
            continue;
        }

        //formatea y guARDA EN LA BASE DE DATOs temporal 
        int len = sprintf(out_buf, "%s %lu\n", filename, current_hash);
        write(fd_new, out_buf, len);

        if (!first_run) {
            //si no es la primera vez que que se ingresa a la db el archivo buscamos su hash del pasado 
            unsigned long prev_hash = obetner_hash_previo(filename);
            

            if (prev_hash == 0) {
                sprintf(out_buf, "%s: nuevo archivo registrado.\n", filename);
                print_msg(out_buf);
                //compara el hash calculado con el previo en la DB antrior para verificar si es diferentes 
            } else if (current_hash != prev_hash) {
                sprintf(out_buf, "%s: ARCHIVO MODIFICADO!.\n", filename);
                print_msg(out_buf);
                alertas++;
            } else {
                //no ha camibado el archivo 
                sprintf(out_buf, "%s: Verificado.\n", filename);
                print_msg(out_buf);
            }
        } else {
            //si es la priemra vez no hay con que comparar 
            sprintf(out_buf, "%s: Registrado.\n", filename);
            print_msg(out_buf);
        }
    }
    //se cierra el archivo temporal 
    close(fd_new);

    unlink(RUTA_DB); //borramos la DB vieja 
    rename(RUTA_DB_TMP, RUTA_DB);//renombranos la temperotal para que se la oficial 

    return alertas > 0 ? 1 : 0;
}


void crear_directorio_verificador(){
    
    char path_F[TAM_RUTA_BASE];
    
    char exito[]="se ha creado el directorio con exito!";
    char padre[]=".andromeda_shell/ creado o existente\n";
    
    //construir ruta al padre: /home/user/.andromeda_shell/
    snprintf(path_F, sizeof(path_F), "%s%s", getenv("HOME"), verificador_padre);
    
    //intentar crear padre
    if(mkdir(path_F, 0700) == 0){
        write(1, exito, strlen(exito));
        write(1, "\n", 1);
    }
    else if (errno == EEXIST){
        // Ya existe, no pasa nada, continuamos
    }
    else{
        perror("Error al crear la carpeta padre");
    }

    //construir ruta final: /home/user/.andromeda_shell/.verificador/
    //path_F ya tiene la ruta padre, le concatenamos al hijo
    strcat(path_F, verificador_final);

    // Intentar crear carpeta final
    if(mkdir(path_F, 0700) == 0){
        write(1, exito, strlen(exito));
        write(1, verificador_final, strlen(verificador_final));
        write(1, "\n", 1);
    }
    else if(errno == EEXIST){
        // Ya existe, está bien.
    }
    else{
        perror("Error al crear el directorio verificador");
    }

    snprintf(RUTA_DB, sizeof(RUTA_DB), "%s.integrity.db", path_F);
    snprintf(RUTA_DB_TMP, sizeof(RUTA_DB_TMP), "%s.integrity.db.tmp", path_F);
}

//escribe en pantalla 
void print_msg(const char *msg) {
    write(STDOUT_FILENO, msg, strlen(msg));
}

//vamos a leer linea por byte por byte del archivo 

int leer_linea(int fd, char *buffer, int max_len) {
    int i = 0;
    char c;
    while (i < max_len - 1) {

        int n = read(fd, &c, 1);
        if (n <= 0) {
            if (i == 0) return 0;
            break;
        }
        //verificando si hay salto de linea cortar 
        if (c == '\n') break;
        //guaramos el byte en el buffer 
        buffer[i++] = c;
    }
    //se agrega un caracter nulo al final para que el string sea valido 
    buffer[i] = '\0';
    return 1;
}

//funcion HASH del algoritmos djb2 
//
unsigned long hash_file(const char *filename) {
    int fd = open(filename, O_RDONLY);
    if (fd < 0) return 0;

    unsigned long hash = 5381;
    char buffer[BUFFER_SIZE];
    int bytes_read;

    //
    while ((bytes_read = read(fd, buffer, BUFFER_SIZE)) > 0) {
        
        for (int i = 0; i < bytes_read; i++) {
            hash = ((hash << 5) + hash) + buffer[i];
        }
    }

    close(fd); //cerramos el descritops 
    return hash; //regresamos el hash creado 
}


unsigned long obetner_hash_previo(const char *target_file) {
    // va a abrir la rura en donde se guardan los hash 
    int fd = open(RUTA_DB, O_RDONLY);
    if (fd < 0) return 0;

    //va a estar letedno 1kbye por 1kbyte 
    char line[1024];
    char name[512];
    unsigned long found_hash = 0; //bandera que indica que se encontro la base de datos. 

    while (leer_linea(fd, line, sizeof(line))) {
        char *ptr_hash = strrchr(line, ' ');
        //solo entrara aqui hasta que se haya leido el nombre completo 
        if (ptr_hash) {
            *ptr_hash = '\0';//separamos el nombre del hash
            ptr_hash++;       //se avanza al numero 
            if (strcmp(line, target_file) == 0) {
                found_hash = strtoul(ptr_hash, NULL, 10);
                break;
            }
        }
    }
    close(fd);
    return found_hash;
}

