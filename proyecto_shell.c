
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include<fcntl.h>



//macros 
#define MAX_INPUT 1024 //maxima letras admitidos
#define MAX_ARGS 64 //numero maximo de argumetnos



//funciones ]

/***
*
*@brief genera los tokesn para el proceso de comandos 
*@params line apuntador de la linea actual de entrada 
*@retun toekens -> entrada separa en tokens
*
*/
char **parse_input(char *line);

/**
 * 
 * @brief 
 * 
 * 
 */
int ejecuta_comandos_sistema(char **args);

int borrar_rastro(char **args);
int procesar_comando(char **args);

int main() {
    char input[MAX_INPUT];
    char **args;
    int status = 1; // 1 para continuar, 0 para salir

    printf("===shell===\n");
    printf("Escribe 'exit' para salir\n\n");

    while (status) {
		char cwd[1024];
		if(getcwd(cwd,sizeof(cwd)) != NULL){
	
		printf("%s >> ", cwd);
		}
		else{
	
			printf(">>> ");
		}

		fflush(stdout);

		//leer la entrad:

	
        if (fgets(input, MAX_INPUT, stdin) == NULL) {
            break; // Error o EOF
        }

        // Parsear entrada
        args = parse_input(input);

        status = procesar_comando(args);

        // Mostrar información del comando (en lugar de ejecutarlo)
       // print_command_info(args);

        free(args);
    }
    return 0;
}


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



int ejecuta_comandos_sistema(char **args){


	pid_t pid;
	int status;

	if(args[0] == NULL){
		return 0;
	}

	pid = fork();

	if(pid == 0){
	
		//proceso hijo
		if(execvp(args[0], args)==-1){
			perror("Error ejecuando comadno");
		}

		exit(EXIT_FAILURE);
	}
	else if(pid<0){
		perror("error en fork");
	
	}
	else{
		//proceso padre 
		//
		do{
			waitpid(pid, &status, WUNTRACED);

		}while(!WIFEXITED(status) && !WIFSIGNALED(status));
	}

	return 1;

}



int borrar_rastro(char **args){

	pid_t pid;
	int status; 

	pid = fork();

	if(pid == 0){
	
		//proceso hijo 
		char *new_args[]= {"./borrar_rastro/borrar_rastro",NULL};
		if(args[1] != NULL){
		
			//hay arguemnetos uqe pasarle al comandos 
			new_args[1]= args[1];
			new_args[2]=NULL;
		}

		if(execvp(new_args[0],new_args) == -1){
		
			perror("error ejecutando borrar_rastro");
		}
		exit(EXIT_FAILURE);
	}
	else if(pid < 0){
	
		perror("error en el fork");
	}
	else{
	
		//proceso padre 
		do{
		
			waitpid(pid, &status, WUNTRACED);
		}while(!WIFEXITED(status) && !WIFSIGNALED(status));
	}


	return 1;
}

int procesar_comando(char **args){

	if(args[0] == NULL){
	
		return 1; //comandos vacio
	}


	//salir del sistema 
	if(strcmp(args[0], "exit") == 0){
	
		printf("saliendo del sistema\n");
		return 0;
	}
	
	
	//por ahora solo procesa el comando de borrar rastro 
	
	if(strcmp(args[0], "borrar_rastro") == 0){
		return borrar_rastro(args);
	}



	return ejecuta_comandos_sistema(args);
}

