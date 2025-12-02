
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




int procesar_comando(char **args);
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
		//obetenemos el directorio actual, esto se hace para orientar al usuario en donde esta 
		//sin este solo se veria >>> y el usuario no sabria en que directorio esta y puede ejeucar un 
		//comando que pueda modificar o eliminar archivos importantes por lo que con este 
		//con el comando "getcwd()" se obtiene el directorio /home/user/dir/ >>>

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
		 * va a guardar en el buffer del comadno, la entrada del usuario hasta 
		 * que se encuentre el salto de linea 
		 * 
		 */
        if (fgets(input, MAX_INPUT, stdin) == NULL) {
            break; // Error o EOF
        }

        // una vez obtenido lo que el usuario ingreso, pasamos a separar por tokens 
		/**
		 * el comando, para saber que comando o script quiere ejeuctar, parametros y archivos con los que va a 
		 * trabajar.
		 * una vez serparado el comadno en tokens, tenemos una lista de strings por lo que son los 
		 * parametros que se ingresaon por linea de comand
		 * 
		 * 
		 */
        args = parse_input(input);

		/**
		 * una vez teneindo los tokens en hora de saber que es lo que quiere hacer el usuario?
		 * 
		 * 
		 */

        status = procesar_comando(args);

        // Mostrar información del comando (en lugar de ejecutarlo)
       // print_command_info(args);

        free(args);
    }
    return 0;
}


/**
 * genrear los tokens para la proceso de comandos
 * 
 * su funciones es tomar lo que ingreso el usurio y regresar un arreglo de cadenas donde cada elementos 
 * es un token, estos token estal delimitados pro espacios, tabs, saltos de linea, retorno de corro
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
	 * strok en la primrea pasada devuelve un apuntador de la primera subcadena 
	 * que se encuentre que este delimitado por los caracteres
	 * 
	 * !!importante -> token va a modificar el parametro "line"
	 */
    token = strtok(line, " \t\n\r");
    
	while (token != NULL) {
		//lo que se hace es recorrer 
        tokens[position] = token; //guarda el punetro de es asubcadena. 
        position++;//aumentamos la posicion

        if (position >= MAX_ARGS) {
            break;
        }
		/**
		 * cunado se le pasa un parametro NULL en la sigueinte lectrua se lele una copia almacenada 
		 * del ultimo parametro de strin1 no nulo -> devuelve puntero al sigueinte token e la lista. 
		 */
        token = strtok(NULL, " \t\n\r");
    }
	//la cadena de token lo terminamos con NULL porqeu las funciones que trabajan con los procesos esperan que 
	//la lista termine con NULL 
    tokens[position] = NULL;
    return tokens;
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
	
	
	//por ahora solo procesa el comando de borrar rastro 
	
	if(strcmp(args[0], "borrar_rastro") == 0){
		return borrar_rastro(args);
	}


	if(strcmp(args[0], "fantasma-rm") == 0){
		return fantasma_rm(args);
	}


	//en caso de no ser un comando local especial o uno de los scripts/comandos 
	//propios quiere decir que quiere ejeuctar algun otro comando s

	return ejecuta_comandos_sistema(args);
}




int ejecuta_comandos_sistema(char **args){

	//declaramos variables a utuliza, pid que tomara el id de proceso al ejecutar el comadno fork 
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
	
		//proceso hijo
		//funcion para ejecutar cualquier comando del sistema, creacion de un proceso que no sea igual al padre que lo invoco 
		/**
		 * la funcion execvp remplaza el proceso hijo crfeado con uno nuevo, en este caso es un proceos 
		 * que responde al comando en la primera posicion del argumento args[] el cual se busca en el PATH del sistema
		 * como segundo parametro es toda la lista de argumentos, esto porque si no se pone el comando a ejecutar 
		 * con sus argumentos la funcion falla y regresa -1 que indica el fallo.
		 * 
		 */
		
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
		 * en este caso que el proceso hijo termiena ya sea por exit() o por una senial.
		 * 
		 * el hecho de estar en un ciclo es por si un hijo no ha terminado si no que esta pausado. 
		 * 
		 * 
		 * 
		 * la opcion WUNTRACED hace que waitpid retrono tambien cunado el hijo este dentenido. 
		 * 
		 * en este caso estamso esperando a que el hijo termine por completo 
		 * 
		 */
		do{

			waitpid(pid, &status, 0);
			
			//macros que nos ayudan a examinar el estado en el que se encuentra el hijo 

			/**
			 * lo que esta pasando es que, mientras el hijo no haya terminao normalmente y no haya sido
			 * terminado por senial, osea sigue esperando mientras el proceco este vivo, detenia o en cualuqier 
			 * otro estado menos en terminado 
			 * 
			 * 
			 */
		}while(!WIFEXITED(status) && !WIFSIGNALED(status));
	}

	return 1;

}





//scripts 

/**
 * 
 * el codigo es algo parecido al otro, pero lo quiero mantenre separado por si ocurre 
 * algun error con este script solo sea en esta funcion y solucionarlo mucho mas facil 
 * 
 * 
 */

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

		/**
		 * 
		 * ahora estamos cambinado el proceos creado por el fork al proceso al cual se esta llamando 
		 * con la direccion del primer indice del arreglo, si todo tiene exito se ejecutara el proceos 
		 * 
		 * 
		 * como es que funciona exevp?
		 * 
		 * lo que hace es reemplazar completamente el programa actual por uno nuevo, por lo que las lineas que estan 
		 * despues de esta llamada no se ejecutan. 
		 * 
		 * 
		 */
		if(execvp(new_args[0],new_args) == -1){
		
			perror("error ejecutando borrar_rastro");
		}
		/**
		 * si la ejecucion execvp falla este va a mostrar en pantalla el mensaje de erro y luego 
		 * llama a "exit" con la macro estbalceita para terminar, lo cual cerrara todos los descriptores de archivo.
		 * 
		 * porque si tiene exito nunca se regresa. 
		 */
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

