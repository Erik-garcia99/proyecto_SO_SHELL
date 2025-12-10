/*
garcia chavez erik  01275863
Armando Tepale Chocolat  1280222
Sistemas operativos 2025-2
proyecto <adromeda_shell>
ingenieria en computacion 
UABC
*/

#include<stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {

	
	
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

    //al crear el proceso el hijo cerrara su lado de escritura porque el
    /**
     * el hijo se encargara de la lectrua, lee los datos del archivo que 
     * el padre tiene acceso, por lo que el hijo lee los datos del padre 
     * aplica la operacion xor y guarda los datos dentro de un archivo temproal
     * 
     */

     pid_t pid = fork();
    if (pid == 0) {
        
        close(pipefd[1]);
        char tmp[256];
        snprintf(tmp, 256, "%s.tmp", archivo);

        int fd = open(tmp, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd < 0) return close(pipefd[0]), 1;
        

        char buf[4096];
        int k = 0;
        int n;
        while ((n = read(pipefd[0], buf, 4096)) > 0) {
            for (int i = 0; i < n; i++){
	    	    buf[i] ^= clave[k++ % clave_len];
	        }
        }
        write(fd, buf, n);
        
        close(fd);
        close(pipefd[0]);
        //sustituye el archivo original por el archivo temproal 
        rename(tmp, archivo);
        return 0;
    }
    
    else if(pid < 0){
        perror("error en la creacion de fork");
        return 1;
    }
    else{
        close(pipefd[0]);
        int fd = open(archivo, O_RDONLY);
        //verifiando que no falle si el caso entonces cierra 
        if (fd < 0) return close(pipefd[1]), 1;
        
        //el apdre va a escribir en el pipeline para que el hijo lo reciba 
        char buf[4096];
        int n;
        while ((n = read(fd, buf, 4096)) > 0){

            write(pipefd[1], buf, n);
        }  
        close(fd);
        close(pipefd[1]);
        //espera aue el hijo termina, solo esos nos importa 
        wait(NULL);
        return write(1, "Listo\n", 6), 0;
    }
    return 1;
}

