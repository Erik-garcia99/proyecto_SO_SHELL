
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include<fcntl.h>



//macros 
#define MAX_INPUT 1024 //maxima letras admitidos
#define MAX_ARGS 64 //numero maximo de argumetnos
#define MAX_PIPE 10


//funciones ]

/***
*
*@brief genera los tokesn para el proceso de comandos 
*@params line apuntador de la linea actual de entrada 
*@retun toekens -> entrada separa en tokens
*
*/
char **parse_input(char *line);



int procesar_linea_comando(char *line);



/**
 * si la entrada es solo un comando entonces se llamara a esta funcion que solo tendra la tarea 
 * de ejecutar un comando simple sin pipeline, tanto dels sitema como los propros y los scripts 
 * 
 * 
 */
int procesar_comando(char **args);





/**
 * 
 * @brief 
 * 
 * 
 */
int ejecuta_comandos_sistema(char **args);




//comandos 



//script

int fantasma_rm(char **args);
int borrar_rastro(char **args);



int main() {
	//buffer para guardar lo que escribe el usuarios 
    char input[MAX_INPUT];
    char **args; //arreglo de argumentos que se le pasaran al comadno que se llamara 
    int status = 1; // 1 para continuar, 0 para salir

    printf("===shell===\n");
    printf("Escribe 'exit' para salir\n\n");

    while (status) {
		char cwd[1024];
		//obetenemos el directorio actual,
		//esta funcion asgina en el buffer cwd el directorio acutla en donde se enceuntra 
		if(getcwd(cwd,sizeof(cwd)) != NULL){
	
			printf("%s >> ", cwd);
		}
		else{
			//promt de respaldo en caso que falle getcwd y no puedira obtener el directorio
			//posiblemente causas de permisos. 
			printf("+++> ");
		}

		fflush(stdout); //limpiamos el buffer para asegurar uqe se muestre el promt

		//leer la entrad:

		/**
		 * comprobamos si es que hay error o encuentra el final sale 
		 * 
		 */
		if (fgets(input, MAX_INPUT, stdin) == NULL) {
            		break; // Error o EOF
        	}

        // una vez obtenido lo que el usuario ingreso, pasamos a separar por tokens
		args = parse_input(input);

		/**
		 * una vez teneindo los tokens en hora de saber que es lo que quiere hacer el usuario?
		 * 
		 * 
		 */

       		status = procesar_comando(args);
		
       		
        	free(args);

		//modificacion para funcionar con pipeline y redireccionamiento 
		//status = procesar_linea_comadno(input);
    }
    return 0;
}


/**
 * genrear los tokens para la proceso de comandos
 * 
 * su funciones es tomar lo que ingreso el usurio y regresar un arreglo de cadenas donde cada elementos 
 * es un token
 * 
 * 
 */


char **parse_input(char *line) {
	//indica el indice en dodne se ingresaran los tokens en la lista. 
    int position = 0;
	/**
	 * arreglo de cadenas. 
	 * se reserva memora para 64 apuntadores a char. 
	 * 
	 */
    char **tokens = malloc(MAX_ARGS * sizeof(char*));

	//apuntador para guarda cada token que se extrae
    char *token;

	//verificar que se haya asignado correctamente la memoria 
    if (!tokens) {
        fprintf(stderr, "Error de asignación de memoria\n");
        exit(EXIT_FAILURE);
    }

	/**
	 * !!importante -> token va a modificar el parametro "line"
	 */
    token = strtok(line, " \t\n\r|");
    
	while (token != NULL) {
		//lo que se hace es recorrer 
        tokens[position] = token; //guarda el punetro de es asubcadena. 
        position++;//aumentamos la posicion

        if (position >= MAX_ARGS) {
            break;
        }
		
        token = strtok(NULL, " \t\n\r");
    }
	//la cadena de token lo terminamos con NULL porqeu las funciones que trabajan con los procesos esperan que 
    tokens[position] = NULL;
    return tokens;
}



int procesar_linea_comando(char *line){

	//eliminamos el salto de linea, si se ingresa el salto de linea dejamos que el usuario continue 
	//usando la shell 

	line[strcspn(line, "\n")] =0; //lo terminamos como nulo 
	

	if(strlen(line)==0){
		return 1; //si no ingreso nada muy posiblemente es un error o le dio enter sin querer
	}

	//verificamos si hay pieline 

	char *pipe = strchr(line, '|');

	return 1;
}




int procesar_comando(char **args){

	if(args[0] == NULL){
	
		return 1; //comandos vacio
	}

	//si el primero parametro es "exit" el usuario ha elgido salir retorna 0 
	//que le indicara a la shell que es hora de termienal 
	//salir del sistema 
	if(strcmp(args[0], "exit") == 0){
	
		printf("saliendo del sistema\n");
		return 0;
	}
	

	//el comando cd 
	if(strcmp(args[0], "cd") == 0){

		if(args[1] == NULL){

			fprintf(stderr, "cd necetia un argumentos\n");
		}
		else{

			if(chdir(args[1]) != 0){
				perror("cd");
			}
		}
		return 1;
	}
	
	
	//por ahora solo procesa el comando de borrar rastro 
	
	if(strcmp(args[0], "borrar_rastro") == 0){
		return borrar_rastro(args);
	}


	if(strcmp(args[0], "fantasma-rm") == 0){
		return fantasma_rm(args);
	}


	//en caso de no ser un comando local especial o uno de los scripts/comandos 
	return ejecuta_comandos_sistema(args);
}




int ejecuta_comandos_sistema(char **args){


	pid_t pid;
	int status;

	//no hay comando, no se ejcuta nada esto puede pasar cunado el usuario ingresa enter
	if(args[0] == NULL){
		return 0;
	}

	//creamos proceso hijo, recordando que fork crea un proceso exacto al padre 
	
	pid = fork();

	//en el caso que el proceo creado sea el hijo entonces es cunado se puede crear un proceso 
	//del comando que el usurio solicito
	if(pid == 0){
	
		if(execvp(args[0], args)==-1){
			perror("Error ejecuando comadno");
		}

		exit(EXIT_FAILURE);
	}
	else if(pid<0){
		//en este caso ocurrio un error al crear el proceso hijo. 
		perror("error en fork");
	
	}
	else{
		//proceso padre 
		/**
		 * estete va a esperar a que el hijo con el pid dado cambie de estado, 
		 * 
		 * 
		 */
		do{

			waitpid(pid, &status, 0);	

		}while(!WIFEXITED(status) && !WIFSIGNALED(status));
	}

	return 1;

}

int procesar_redirecciones(char **args){




}





//scripts 


int borrar_rastro(char **args){

	pid_t pid;
	int status; 



	//creamos el proceso hijo/padre y cachamos su id de proceso 
	pid = fork();

	if(pid == 0){
	
		//proceso hijo 
		//construimos el comando, la ruta en donde esta ej ejecutable, el archivo que sera borrado
		//el arreglo debe terminar con NULL porque asi es necesario para execvp
		char *new_args[]={"./borrar_rastro/borrar_rastro", args[1], NULL}; 

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
		
			waitpid(pid, &status, 0);
		}while(!WIFEXITED(status) && !WIFSIGNALED(status));
	}


	return 1;
}



int fantasma_rm(char **args){

	pid_t pid; 
	int status; 

	pid = fork();

	if(pid == 0){
	
		//proceso hijo 
		//
		char *new_args[MAX_ARGS]; //arreglo que sera el que le pasaremos a la funcio excecvp para crear el proceso 

		//copiar arguemntos adicionales
		if(args[1] != NULL){
			//aqui solo le estamos pasando los demas archivos que seran ocultados en el sistrema 
			int i=1;
			new_args[0]="./fantasma-rm/fantasma-rm";
			while(args[i] != NULL && MAX_ARGS -1){
				new_args[i] = args[i];
				i++;
			
			}

			new_args[i]=NULL;
		}

		//se crea el proceso de fantasma 
		if(execvp(new_args[0], new_args)==-1){
		
			perror("error ejecutando fantasma-rm");
		}

		exit(EXIT_FAILURE);
	}
	else if(pid < 0){
		perror("error en fork");
		
	}
	else{

		//proceso padre

		do{
			//esperando que el hijo termine.
			waitpid(pid, &status, 0);

		}while(!WIFEXITED(status) && !WIFSIGNALED(status));

	}
	return 1;

}




//%%%%%%%%%%%%%%%%5comadnos%%%%%%%%%%%%%%%%%%%%55

