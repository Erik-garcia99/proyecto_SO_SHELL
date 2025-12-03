/*
  cifra-xor.c
  Cifra/descifra con XOR usando pipeline interno (fork + pipe).
  Compilar: gcc -o cifra-xor cifra-xor.c
  Uso: ./cifra-xor archivo.txt clave123
*/
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    if (argc < 3) return write(2, "Uso: ./cifra-xor <archivo> <clave>\n", 36), 1;
    
    char *archivo = argv[1], *clave = argv[2];
    int clave_len = strlen(clave);
    if (!clave_len) return write(2, "Error: clave vacia\n", 19), 1;
    
    int pipefd[2];
    if (pipe(pipefd) < 0) return write(2, "Error: pipe\n", 12), 1;
    
    if (fork() == 0) {
        // HIJO: Lee pipe, cifra, guarda en temporal
        close(pipefd[1]);
        char tmp[256];
        snprintf(tmp, 256, "%s.tmp", archivo);
        int fd = open(tmp, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd < 0) return close(pipefd[0]), 1;
        
        char buf[4096];
        int k = 0;
        ssize_t n;
        while ((n = read(pipefd[0], buf, 4096)) > 0) {
            for (int i = 0; i < n; i++) buf[i] ^= clave[k++ % clave_len];
            write(fd, buf, n);
        }
        close(fd);
        close(pipefd[0]);
        
        // Renombrar temporal -> original
        rename(tmp, archivo);
        return 0;
    }
    
    // PADRE: Lee archivo, envía a pipe
    close(pipefd[0]);
    int fd = open(archivo, O_RDONLY);
    if (fd < 0) return close(pipefd[1]), 1;
    
    char buf[4096];
    ssize_t n;
    while ((n = read(fd, buf, 4096)) > 0) write(pipefd[1], buf, n);
    
    close(fd);
    close(pipefd[1]);
    wait(NULL);
    return write(1, "Listo\n", 6), 0;
}
