
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


//******************funciones principales


/***
*
*@brief genera los tokesn para el proceso de comandos 
*@params line apuntador de la linea actual de entrada 
*@retun toekens -> entrada separa en tokens
*
*/
char **parse_input(char *line);

int procesar_linea_comando(char *line);

//ejecutar piepline 
int ejecutar_pipeline(char **comandos, int num_comandos);

int ejecuta_comandos_sistema(char **args);

//************************************************


//*************comandos internos y scritps 

int procesar_comando(char **args);




//comandos 

int cifrado_xor(char **args);

//script

int fantasma_rm(char **args);
int borrar_rastro(char **args);

//***************funciones auxiliar para dividir pipeline







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
		//args = parse_input(input);


		/**
		 * una vez teneindo los tokens en hora de saber que es lo que quiere hacer el usuario?
		 * 
		 * 
		 */

       		//status = procesar_comando(args);
		
       		
        	//free(args);

		//modificacion para funcionar con pipeline y redireccionamiento 
		status = procesar_linea_comadno(input);

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
    token = strtok(line, " \t\n\r|");
    
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



int procesar_linea_comando(char *line){

	//eliminamos el salto de linea, si se ingresa el salto de linea dejamos que el usuario continue 
	//usando la shell 

	line[strcspn(line, "\n")] =0; //lo terminamos como nulo 
	

	if(strlen(line)==0){
		return 1; //si no ingreso nada muy posiblemente es un error o le dio enter sin querer
	}

	//verificamos si hay pieline 

	char *pipe_pos = strchr(line, '|');
	

	if(pipe_pos = NULL){
	
	
		//comando simpre
		char **args = parse_input(line);
		int status = procesar_comando(args);


		//lberar memoria 
		//
		int i=0; 
		while(args[i]!=NULL){
		
			free(args[i]);
			i++;
		}
		free(args);
	}else{
		//hay pipe line, al menos 1 pero hay un limite
		char *comandos[MAX_PIPES];
		int num_comandos = 0; 

		char *token = strtok(line, "|");

		while(toke !=NULL && num_comandos < MAX_PIPES){
		
			//eliminar espacios extras
			//
			while(*token=' ') token++;

			char *end = token + strlen(token)-1;

			while(end >  token && *end ==' '){
			
				*end = '\0';
				end--;
			}

		}


		comandos[num_comandos] = strdup(token);
		num_comandos++;
		toke = strtok(NULL, "|");

		if(num_comandos < 2){
			
			fprintf(stderr, "error: pipeline invalido\n");

			for(int i=0; i< num_comandos; i++){
			
				free(comandos[i]);
			}

			return 1;
			
		}

		//parsear cada comandos 
		//
		char **comandos_args = malloc(num_comandos *sizeof(char**));

		for(int i=0; i< num_comados;i++){
		
			comandos_args[i]=parse_input(comandos[i]);
			free(comandos[i]);
		}

		//ejecutar pipelines
		//
		int resultado = ejecutar_pipeline(comandos_args, num_comandos);
	

		//liberear memoria 
		//
		for(int i=0; i< num_comandos; i++){
		
			for(int j=0; i<comandos_args[i][j]; i++){
			
				free(comandos_args[i][j]);
			}

			free(comandos_args[i]);
		
		}

		free(comandos_args);
		return resultado;

	}	

	return 1;
}



int ejecutar_pipeline(char **comandos, int num_comandos){


	int pipes[num_comandos-1][2];

	pid_t pids[num_comandos];
	int i;

	//crear pipes 
	//
	for(i=0; i< num_comandos-1;i++){
	
		if(pipe(pipes[i])==-1){
		
			perror("error creando pipe");
			return 1;
		}
	
	}


	//crear procesos para cada comandos 
	//
	for(i=0; i< num_comadnos; i++){
	
		pids[i] = fork();

		if(pids[i] == 0){
			//proceso hijo 
			//
			//conectar pipes 
			//
			if(i >0){
			
				//redirigir entrada desde pipe anteriorr
				dup2(pipes[i-1][0], STDIN_FILEO);
			}

			if(i< num_comandos -1){
				//redirigir salida al pipe actual
				dup2(pipe[i][1], STDOUT_FILEO);
			}

			for(int j=0; i<num_comandos-1;j++){
			
				close(pipe[j][0]);
				close(pipes[j][1]);
			
			}

			//ejejcutar comando 
			//
			retrn ejecutar_comandos_simpre(comandos[i]);
		
		
		}else if(pids[i]<0){
		
			perror("error en fork");
			return 1;
		}

	
	}

	//cerrandos todos los pipes en el proceso padre 
	//
	for(i=0; i< num_comandos-1; i++){
	
		close(pipes[i][0]);
		close(pipes[i][0]);
	}

	//esperar a que termine 
	//
	for(i=0; i< num_comandos; i++){
		waitpid(pipes[i],NULL,0);
	}
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
	
	
	//por ahora solo procesa el comando de borrar rastro 
	
	if(strcmp(args[0], "borrar_rastro") == 0){
		return borrar_rastro(args);
	}


	if(strcmp(args[0], "fantasma-rm") == 0){
		return fantasma_rm(args);
	}


	if(strcmp(args[0], "comando_xor") == 0){

		return cifrado_xor(args);
	}


	//en caso de no ser un comando local especial o uno de los scripts/comandos 
	//propios quiere decir que quiere ejeuctar algun otro comando s

	return ejecuta_comandos_simple(args);
}



//este va a pasar a ser -> procesar_comando_simple
int ejecuta_comandos_simple(char **args){



	if(args[0]==NULL){
	
		return 1;
	}

	//verificar si hay redireccionaminetos 
	//
	char *input_file =NULL;
	char *output_file = NULL;
	int append_mode = 0;
	int background = 0;

	//filtrar redirecciones 
	//
	char *filtrados[MAX_ARGS];
	int filtrado_idx=0;

	for(int i=0; args[i] !=NULL && filtrado_idx < MAX_ARGS -1 ;i++){
	
		if(strcmp(args[i], "<")==0){
		
			//redireccion de entrada 
			//
			if(args[i+1] ==NULL){
			
				fprintf(stderr, "ERROR: se necesita un archivo de entrada\n");
				return 1;
			}
			input_file = args[i+1];
			i++;//saltar el archivo
		}

		else if(strcmp(args[i], ">") == 0){
		
			//redireccionamoento de salida 
			//
			if(args[i+1] == NULL){
				fprintf(stderr, "ERROR: se necesita un archivo de salida\n");
				return 1;
			
			}

			output_file = args[i+1];
			append_mode = 0;
			i++;
		}
		else if(strcmp(args[i],">>")==0){
		
			if(args[i+1] ==NULL){
			
				fprintf(stderr, "ERROR: se necesita un archivo de salida\n");
				return 1;
				
			}

			output_file =args[i+1];
			append_mode =1;
			i++;
		
		}
		else if(strcmp(args[i],"&")==0){
			//ejecucar backgroudn
			background=1;
		}
		else{
			filtrado[filtrado_idx++]= args[i];
		}
	
	}
	filtrado[filtrado_idx]=NULL;
	

	//si no hay cambio despues de filtrar 
	//
	if(filtrado[0]==NULL){
	
		return 1;
	}

	//ejecutar comando interno o del sistema 
	//
	if(strcmp(filtrado[0], "fantasma-rm")==0){
		return fantasma-rm(args);				
				
	}

	else if(strcmp(filtrado[0], "borrar_rastro")==0){
	
		return borrar_rastro(args);
	}
	else{
	
	
		pid_t pid;
		int status

		pid=fork();

		if(pid==0){
		
			//proceso hijo 
			//
			if(input_file !=NULL){
			
				int fd_in = open(input_file, O_RDONLY);
				if(fd_int < 0){
				
					perror("errro al abrir el archivo de entrada");
					exit(EXIT_FAILURE);
				}
				dup2(fd_in, STDIN_FILENO);
				close(fd_in);
			
			}

			//redireccioamineto de salida 
			//
			if(output_file !=NULL){
				int flags = O_WRONLY | O_CREAT;
				if(append_mode){
					flags |= O_APPEND;
				
				}
				else{
				
					flags |=O_TRUNC;
				}

				in fd_out = open(outpu_file,flags, 0644);
				if(fd_out < 0){
				
					perror("Error: creando el archivo de salida\n");
					exit(EXIT_FAILURE);
				}
				dup2(fd_out,STDOUT_FILENO);
				close(fd_out);
			
			}

			//ejecutar comando 
			//
			if(execvp(filtrado[0], filtrado)==-1){
			
				perror("ERROR al ejejcutar el comando");
			}
			exit(EXIT_FAILURE);
		}
		else if(pid<0){
		
			perror("erro en fork");
			return 1;
		}

		else{
		
			//proceos padre
			//
			if(!background){
			
				do{
				
					waitpid(pid, &status, 0);

				}while(!WIFEXITED(status) && !WIFSIGNALED(status));
			}else{
			
				printf("[%d] ejecutando en background\n", pid);
			}
		}
	}




	return 1;

}

int procesar_redirecciones_en_comando(char **args, char **input_file, char **output_file, int *append) {
    *input_file = NULL;
    *output_file = NULL;
    *append = 0;
    
    // Buscar redirecciones
    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], "<") == 0 && args[i + 1] != NULL) {
            *input_file = args[i + 1];
            args[i] = NULL; // Marcar para ignorar
            i++; // Saltar el archivo
        }
        else if (strcmp(args[i], ">") == 0 && args[i + 1] != NULL) {
            *output_file = args[i + 1];
            *append = 0;
            args[i] = NULL;
            i++;
        }
        else if (strcmp(args[i], ">>") == 0 && args[i + 1] != NULL) {
            *output_file = args[i + 1];
            *append = 1;
            args[i] = NULL;
            i++;
        }
    }
    
    return 0;
}





//scripts 

/**
 * 
 * el codigo es algo parecido al otro, pero lo quiero mantenre separado por si ocurre 
 * algun error con este script solo sea en esta funcion y solucionarlo mucho mas facil 
 * 
 * 
 */
int borrar_rastro(char **args) {
    pid_t pid;
    int status;
    
    char *input_file = NULL;
    char *output_file = NULL;
    int append = 0;
    
    // Hacer copia de args
    char *args_copia[MAX_ARGS];
    int i;
    for (i = 0; args[i] != NULL && i < MAX_ARGS - 1; i++) {
        args_copia[i] = args[i];
    }
    args_copia[i] = NULL;
    
    // Procesar redirecciones
    procesar_redirecciones_en_comando(args_copia, &input_file, &output_file, &append);
    
    pid = fork();
    
    if (pid == 0) {
        // Proceso hijo
        // Aplicar redirecciones
        if (input_file != NULL) {
            int fd_in = open(input_file, O_RDONLY);
            if (fd_in < 0) {
                perror("Error abriendo archivo de entrada");
                exit(EXIT_FAILURE);
            }
            dup2(fd_in, STDIN_FILENO);
            close(fd_in);
        }
        
        if (output_file != NULL) {
            int flags = O_WRONLY | O_CREAT;
            if (append) {
                flags |= O_APPEND;
            } else {
                flags |= O_TRUNC;
            }
            
            int fd_out = open(output_file, flags, 0644);
            if (fd_out < 0) {
                perror("Error creando archivo de salida");
                exit(EXIT_FAILURE);
            }
            dup2(fd_out, STDOUT_FILENO);
            close(fd_out);
        }
        
        // Construir argumentos para borrar_rastro
        char *new_args[MAX_ARGS];
        new_args[0] = "./borrar_rastro/borrar_rastro";
        int j = 1;
        
        for (int k = 1; args_copia[k] != NULL && j < MAX_ARGS - 1; k++) {
            if (args_copia[k] != NULL && strlen(args_copia[k]) > 0) {
                new_args[j++] = args_copia[k];
            }
        }
        new_args[j] = NULL;
        
        // Ejecutar borrar_rastro
        execvp(new_args[0], new_args);
        perror("Error ejecutando borrar_rastro");
        exit(EXIT_FAILURE);
    }
    else if (pid < 0) {
        perror("Error en fork");
        return 1;
    }
    else {
        // Proceso padre
        do {
            waitpid(pid, &status, 0);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }
    
    return 1;
}

int fantasma_rm(char **args) {
    pid_t pid;
    int status;
    
    char *input_file = NULL;
    char *output_file = NULL;
    int append = 0;
    
    // Hacer copia de args para no modificar el original
    char *args_copia[MAX_ARGS];
    int i;
    for (i = 0; args[i] != NULL && i < MAX_ARGS - 1; i++) {
        args_copia[i] = args[i];
    }
    args_copia[i] = NULL;
    
    // Procesar redirecciones
    procesar_redirecciones_en_comando(args_copia, &input_file, &output_file, &append);
    
    pid = fork();
    
    if (pid == 0) {
        // Proceso hijo
        // Aplicar redirecciones
        if (input_file != NULL) {
            int fd_in = open(input_file, O_RDONLY);
            if (fd_in < 0) {
                perror("Error abriendo archivo de entrada");
                exit(EXIT_FAILURE);
            }
            dup2(fd_in, STDIN_FILENO);
            close(fd_in);
        }
        
        if (output_file != NULL) {
            int flags = O_WRONLY | O_CREAT;
            if (append) {
                flags |= O_APPEND;
            } else {
                flags |= O_TRUNC;
            }
            
            int fd_out = open(output_file, flags, 0644);
            if (fd_out < 0) {
                perror("Error creando archivo de salida");
                exit(EXIT_FAILURE);
            }
            dup2(fd_out, STDOUT_FILENO);
            close(fd_out);
        }
        
        // Construir argumentos para fantasma-rm (sin redirecciones)
        char *new_args[MAX_ARGS];
        new_args[0] = "./fantasma-rm/fantasma-rm";
        int j = 1;
        
        // Copiar solo los argumentos que no son redirecciones
        for (int k = 1; args_copia[k] != NULL && j < MAX_ARGS - 1; k++) {
            if (args_copia[k] != NULL && strlen(args_copia[k]) > 0) {
                new_args[j++] = args_copia[k];
            }
        }
        new_args[j] = NULL;
        
        // Ejecutar fantasma-rm
        execvp(new_args[0], new_args);
        perror("Error ejecutando fantasma-rm");
        exit(EXIT_FAILURE);
    }
    else if (pid < 0) {
        perror("Error en fork");
        return 1;
    }
    else {
        // Proceso padre
        do {
            waitpid(pid, &status, 0);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }
    
    return 1;
}


//%%%%%%%%%%%%%%%%5comadnos%%%%%%%%%%%%%%%%%%%%55

int cifrado_xor(char **args){


	pid_t pid;
	int status;


	pid = fork();


	if(pid == 0){
	
		char *new_args[4];

		new_args[0]="./comando_xor/comando_xor";
		new_args[1]= args[1]; //archiov
		new_args[2]=args[2]; //clave de cifrado
		new_args[3]=NULL;


		if(execvp(new_args[0], new_args) == -1){
			fprintf(stderr, "error al buscar el directorio .cifrado_xor/cifrado_xor\n");

		
		}

		exit(EXIT_FAILURE);
	
	}
	else if(pid < 0){
	
		perror("error en fork");
	}
	else{
	
		//proceso padre 
		//
		do{
		
			
			waitpid(pid, &status, 0);
		}while(!WIFEXITED(status) && !WIFSIGNALED(status));
	}
	
	return 1;
}
