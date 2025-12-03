#include<stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {

	
	//verificia que se hayan ingresados los parametroa necesarios
    if (argc < 3){ 
	return write(2, "Uso: cifra-xor <archivo> <clave>\n", 34), 1;
    }	


    char *archivo = argv[1], *clave = argv[2];
    int clave_len = strlen(clave);
    if (!clave_len){
    
    	return write(2, "Error: clave vacia\n", 19), 1;
    }
	

    int pipefd[2];
	

 	// inidce 0 extermo de lectria 
	/**
	 *indice 1 expremo de esctirutrua 
	 *se crea el pipeline. 
	 * */
	if (pipe(pipefd) < 0){
	    return write(2, "Error: pipe\n", 12), 1;
    
	}

	//creacion del proceso, verificando que en este caso el proceso creado sea el hijo 

    if (fork() == 0) {
        // HIJO: Lee pipe, cifra, guarda en temporal
	//cierra el extermo de de escritura porque el hijon solo necesita leer del pipe del padre 
	//
	//importante porque el pipe nunca detecta EOF porque siempre habra alguein con el extremo abierto 
	//
	//
	//si hijo no cierra la el pipe de escritura entonces nunca se marcara EOF puesto ques i el padre lo cierra entonces si el hijo no, inidcara que aun hay algo por leer lo cual no es, porque esta abiertp el descritpr de escitrutra del propio hijo 
	//
        close(pipefd[1]);

	//preparando el archivo temporal 
        char tmp[256];
	//se crea el archivo temproal con terminacion .tmp
        snprintf(tmp, 256, "%s.tmp", archivo);

	//trata de abir prero si no existe el archivo entonces lo crea si existe trinca su valor a 0 bytes 
        int fd = open(tmp, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	//si ocurre un error cierra antes de salir 
        if (fd < 0) return close(pipefd[0]), 1;
        

        char buf[4096];
        int k = 0;
        int n;
	//bivle de lectria del pipe
	//va a intentar leer 4KB del descriptor
        while ((n = read(pipefd[0], buf, 4096)) > 0) {
            for (int i = 0; i < n; i++){
	    	//se aplica la operacion XOR ciclica con la clave 
	    	buf[i] ^= clave[k++ % clave_len];
	    }
	    //escirbe en el archivo tempeoira 
	    write(fd, buf, n);
        }
	//cerrados los 2 descriptiores
        close(fd);
        close(pipefd[0]);
        
        // Renombrar temporal -> original
        rename(tmp, archivo);
        return 0;
    }
    
    // PADRE: Lee archivo, envía a pipe
    // cierra el descirptior de lectura porque el padre va a escrbir datos para el hijo 
    close(pipefd[0]);
    //abre el archivo orginal pero en solo lectrua 
    int fd = open(archivo, O_RDONLY);
    //verifiando que no falle si el caso entonces cierra 
    if (fd < 0) return close(pipefd[1]), 1;
    
    //el apdre va a escribir en el pipeline para que el hijo lo reciba 
    char buf[4096];
    int n;
    while ((n = read(fd, buf, 4096)) > 0) write(pipefd[1], buf, n);
    //se cierran lso descritores, en este punto ya cerro lectrua y estruiturua 
    close(fd);
    close(pipefd[1]);
    //espera aue el hijo termina, solo esos nos importa 
    wait(NULL);
    return write(1, "Listo\n", 6), 0;
}
