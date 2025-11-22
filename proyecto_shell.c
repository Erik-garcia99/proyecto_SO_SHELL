
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_INPUT 1024 //maxima letras admitidos
#define MAX_ARGS 64 //numero maximo de argumetnos

/**
 * genrear los tokens para la proceso de comandos
 */
char **parse_input(char *line) {
    int position = 0;
    char **tokens = malloc(MAX_ARGS * sizeof(char*));
    char *token;

    if (!tokens) {
        fprintf(stderr, "Error de asignación de memoria\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, " \t\n\r");
    while (token != NULL) {
        tokens[position] = token;
        position++;

        if (position >= MAX_ARGS) {
            break;
        }

        token = strtok(NULL, " \t\n\r");
    }
    tokens[position] = NULL;
    return tokens;
}

/**
 * infromacion de lo que se ingreso, por ahora para concoer que eralmente se esta ingresando correctamante 
 */
void print_command_info(char **args) {
    if (args[0] == NULL) {
        return; // No hay comando
    }

    printf("\n--- informacion del comando ---\n");
    printf("Comando principal: %s\n", args[0]);
    
    printf("argumentos: ");
    for (int i = 1; args[i] != NULL; i++) {
        printf("[%s] ", args[i]);
    }
    printf("\ntotal de argumentos: ");
    
    int arg_count = 0;
    for (int i = 1; args[i] != NULL; i++) {
        arg_count++;
    }
    printf("%d\n", arg_count);
    printf("-------------------------------\n\n");
}


int main() {
    char input[MAX_INPUT];
    char **args;
    int status = 1; // 1 para continuar, 0 para salir

    printf("===shell===\n");
    printf("Escribe 'exit' para salir\n\n");

    while (status) {
        // Mostrar prompt
        printf(">>> ");
        fflush(stdout);

        // Leer entrada
        if (fgets(input, MAX_INPUT, stdin) == NULL) {
            break; // Error o EOF
        }

        // Parsear entrada
        args = parse_input(input);

        // Verificar si hay comando
        if (args[0] == NULL) {
            continue; // Línea vacía
        }

        // Comprobar comando exit
        if (strcmp(args[0], "exit") == 0) {
            printf("Saliendo del shell...\n");
            status = 0;
            free(args);
            continue;
        }

        // Mostrar información del comando (en lugar de ejecutarlo)
        print_command_info(args);

        free(args);
    }
    return 0;
}


