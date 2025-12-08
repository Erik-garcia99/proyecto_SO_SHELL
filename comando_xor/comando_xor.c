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
	

 	
	if (pipe(pipefd) < 0){
	    return write(2, "Error: pipe\n", 12), 1;
    
	}


    if (fork() == 0) {
        
        close(pipefd[1]);

	//preparando el archivo temporal 
        char tmp[256];
	//se crea el archivo temproal con terminacion .tmp
        snprintf(tmp, 256, "%s.tmp", archivo);

        int fd = open(tmp, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	//si ocurre un error cierra antes de salir 
        if (fd < 0) return close(pipefd[0]), 1;
        

        char buf[4096];
        int k = 0;
        int n;
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
    
    
    close(pipefd[0]);
    //abre el archivo orginal pero en solo lectrua 
    int fd = open(archivo, O_RDONLY);
    //verifiando que no falle si el caso entonces cierra 
    if (fd < 0) return close(pipefd[1]), 1;
    
    //el apdre va a escribir en el pipeline para que el hijo lo reciba 
    char buf[4096];
    int n;
    while ((n = read(fd, buf, 4096)) > 0) write(pipefd[1], buf, n); 
    close(fd);
    close(pipefd[1]);
    //espera aue el hijo termina, solo esos nos importa 
    wait(NULL);
    return write(1, "Listo\n", 6), 0;
}

