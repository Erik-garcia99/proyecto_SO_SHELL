
//soluciones de errores de variables mal escritas, tipograficas, de sintaxis etc..



/*
funcion dup2 hace que un descriptor de archivo, como pantalla o el tclado apunte a otro lugar. 

int dup2(int fd1, int fd2); ->> copia la conexion del enchufe fd1 al enchife fd2
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

// Macros 
#define MAX_INPUT 1024
#define MAX_ARGS 64
#define MAX_PIPES 10  

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

int ejecutar_pipeline(char ***comandos, int num_comandos);  // CORREGIDO: char ***

int ejecuta_comandos_simple(char **args);

//************************************************
//*************comandos internos y scripts 
int procesar_comando(char **args);
int procesar_redirecciones_en_comando(char **args, char **input_file, char **output_file, int *append);

// comandos 
int comando_xor(char **args);
int verificador_integridad(char **args);


/**
 * 
 * @brief 
 * 
 * 
 */
int ejecuta_comandos_sistema(char **args);




//comandos 

int cifrado_xor(char **args);

//script

int fantasma_rm(char **args);
int borrar_rastro(char **args);

int bit_log(char **args);

// script
int fantasma_rm(char **args);
int borrar_rastro(char **args);

//en la funcion main se toma el dato en crudo del usuarios 
>>>>>>> Erik
int main() {

	//buffer para guardar lo que ingresa el usaurio, todo lo que e usuario esbrime en la linea de comando, mas no guarda token o otra cosa garda crudo 
    char input[MAX_INPUT];

	//estatus para verificar si seguir o salir de la shell 
    int status = 1;

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

        fflush(stdout);
		//capturamos la entrada y lo guardamos en el buffer
        if (fgets(input, MAX_INPUT, stdin) == NULL) {
            break;
        }

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
    token = strtok(line, " \t\n\r");
    
    while (token != NULL) {
        //guarda el punetro de es asubcadena. 
        //lo que hace es guardar uan copia identica e independiete de una cadena de texto en una nueva ubicacion de memoria. 
        //recordar librerar la memoria puesto que strdup hace uso de malloc internamnete 
        tokens[position] = strdup(token); 
        position++;

        if (position >= MAX_ARGS) {
            break;
        }
		
        token = strtok(NULL, " \t\n\r");
    }
	//la cadena de token lo terminamos con NULL porqeu las funciones que trabajan con los procesos esperan que 
    tokens[position] = NULL;
    return tokens;
}


int procesar_linea_comando(char *line) {

	//va a buscar el salto de linea y lo reemplazara con el caracter nulo 
    line[strcspn(line, "\n")] = 0;
	

    //si la longitud de la linea al elminar el salto de linea es 0 quiere decir que muy probablemnete solo se le dio enter por lo que regresa para que pueda ingresar algo mas el usuario. 
	if(strlen(line) == 0) {
        return 1;
    }

    // verificar si hay pipeline
	//la funcion regresa la direccion de la primera aparicion del caracter mostrado. 
    char *pipe_pos = strchr(line, '|');
    
    if(pipe_pos == NULL) {
		//  
        // comando simple
		//genera los tokens de la solicitud del usuario
        char **args = parse_input(line);
		//dentro de una lista verifica si es un comando interno del sistema o un comando externo personalizado o creado por el usuario, dependiendo si el comando se realizo con exito o no regresa si se sale o sigue el programa. 

        //aqui ya regreso la lista de tokens que solicito el usuario estos son comandos que no hacen uso del pipeline 
        int status = procesar_comando(args);

        //liberamos la memoria 
        int i = 0;
        while(args[i] != NULL) {
            free(args[i]); //eliminando los espacios creados pro strdup
            i++;
        }
        free(args); //elimina la lsita que guardaba donde estaban las palabras 
        return status;
    } else {
        
        //hay pipelines por lo que hay comandos a ejecutar 
        char *comandos[MAX_PIPES];

        int num_comandos = 0;



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


	if(strcmp(args[0], "comando_xor") == 0){
	
		return cifrado_xor(args);
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

        //ejecutar pipeline
		//llamar a la funcion prnicpal que crea los pipes, hace fork y dup2
        int resultado = ejecutar_pipeline(comandos_args, num_comandos);

        //liberamos la memoria 
        for(int i = 0; i < num_comandos; i++) {
            for(int j = 0; comandos_args[i][j] != NULL; j++) {
                free(comandos_args[i][j]);
            }
            free(comandos_args[i]);
        }
        free(comandos_args);
        
        return resultado;
    }
}

int procesar_redirecciones(char **args){




}





//scripts 



int ejecutar_pipeline(char ***comandos, int num_comandos) {

	/**
	 * si tenemos N comandos necesitamos N-1 posiciones de tuberias para conectarlos 
	 * 
	 * cada tuberia tiene 2 extremos [0] lectura [1] lectrua. 
	 */
    int pipes[num_comandos - 1][2];

	//arreglo donde guardaremos los IDS de los procesos creados 
	//esto para que el padre pueda esperar a todos al final 
    pid_t pids[num_comandos];
    int i;

    // creamos los pipelines, necesario crearlos antes puedo que si se crea dentro de un proceso hijo los hermanos no podran compartir los pipelines 
    for(i = 0; i < num_comandos - 1; i++) {
		//la funciones pipe llena el arreglo con 2 descriptores de archivo. 
        if(pipe(pipes[i]) == -1) {
            perror("error creando pipe");
            return 1;
        }
    }

    //dependineo de cuantos comandos creamos ese total de procesos 
    for(i = 0; i < num_comandos; i++) {
		//creamos un proceso el cual es el mismo a este que lo esta llamando 
        pids[i] = fork();

        if(pids[i] == 0) {
            //proceso hijo
            //conectar pipes
			//si no SOY el primer comando necesito leer del anterior 
            if(i > 0) {
            
				//la funcion dup2 toma el extremo de la lectra del pipe anterior y lo conecta a la entrada entandar de este proceso. 
                dup2(pipes[i-1][0], STDIN_FILENO);
            }
			//configurar la lsaida, si no SOY el ultimo comando, necesito escribir hacua el sigueinte 
            if(i < num_comandos - 1) {
                //la funcion dup2 toma el extremos de escritura del pipe acutla 
				//lo conecta en la salida estandar 
				//asi que o que imprimra el comando ira hacia el pipe
                dup2(pipes[i][1], STDOUT_FILENO);
            }

            //ya una vez que se apico la funcion dup2 ya tenemos todas nuestras conexiones listas, debemos cerrar todos los descriptores heredados para evadir figas y bloqueos 
            for(int j = 0; j < num_comandos - 1; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }

            //se ejecuta el comando real 
            /**
             * lo que esta pasando es que ahora ya se puede tener la conexion 
             * entre los comandos entre uno escirbe al otro mas adelante y el mas adelante lee lo que escrbiio el que esta mas atras 
             * 
             *
             */
            ejecuta_comandos_simple(comandos[i]);
            /**
			 * si execvp falla o la funcion regresa, terminamos el hijo con exito para no delar proceso zombes duplicados de la shell
			 * 
			 */
			exit(EXIT_SUCCESS);  
        } else if(pids[i] < 0) {

			//marcamos error si no se pudo crear un proceos 
            perror("error en fork");
            return 1;
        }
    }

    //cerramos los pipes del padre, puesto que el padre tiene copias abierta de los pipes que cerraron los hjos 
	//si el padre no cierra los extremos de escritura los hjos que estan leyendo se quedaran esperando datos eternmanete porque nunca recibiran la senial EOF 
    for(i = 0; i < num_comandos - 1; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);  
    }

    //esper a que todos los hijos terminen 
    for(i = 0; i < num_comandos; i++) {
        waitpid(pids[i], NULL, 0); 
    }
    
    return 1;
}

/**
 * 
 * esta funcion esta verificando que comando se esta ejecutando si es un comando prorpio del sistema o uno de los comandos que son personales 
 */
int procesar_comando(char **args) {
    if(args[0] == NULL) {
        return 1;
    }

    if(strcmp(args[0], "exit") == 0) {
        printf("saliendo del sistema\n");
        return 0;
    }
    
    if(strcmp(args[0], "borrar_rastro") == 0) {
        return borrar_rastro(args);
    }

    if(strcmp(args[0], "fantasma-rm") == 0) {
        return fantasma_rm(args);
    }

	//comandos 
    if(strcmp(args[0], "comando_xor") == 0) {
        return comando_xor(args);
    }

	if(strcmp(args[0], "verificador-integridad") == 0) {
        return verificador_integridad(args);
    }

    if(strcmp(args[0], "bit-log") == 0) {
        return bit_log(args);
    }

    return ejecuta_comandos_simple(args);
}

//procesar comandos que no tengan pipelines

int ejecuta_comandos_simple(char **args) {

	//regresa si no hay arguemenso a procesr
    if(args[0] == NULL) {
        return 1;
    }

    //variables de apoyo por si hay redireccionamineto 
	//para redireccionamineto de entrada <
    char *input_file = NULL;
	//para redireccionamineto de salida > o >>
    char *output_file = NULL;
	//para el modo de salida si es >> se agrega al final si es > se trunca y se agrega desde el inicio 
    int append_mode = 0;
	//si se euncetra & para ejeuctar en 2do plano 
    int background = 0;

    //arreglo para guardar solo el comandos y sus opciones 
    char *filtrados[MAX_ARGS];
    int filtrado_idx = 0;


	//recorrimeos todos los arguementos que escribio el usuario 
    for(int i = 0; args[i] != NULL && filtrado_idx < MAX_ARGS - 1; i++) {
		//esta verificando si hay un redireccionamineto de entrada 
        if(strcmp(args[i], "<") == 0) {
            //auemntamos en 1 e index para verificar que se haya ingresado un archivo de entrada para el comando , si no es asi se indica el errors
            if(args[i+1] == NULL) {
                fprintf(stderr, "ERROR: se necesita un archivo de entrada\n");
                return 1;
            }

            input_file = args[i+1];//guardamos el nombre del archivo
            i++;//saltamos al sigueinte indice porque el nombre del archivo no es parte del comandos 

			//es redireccionamineto de slaida?,
        } else if(strcmp(args[i], ">") == 0) {
            //se verifica de igual froma que haga uso un archivo de salida 
            if(args[i+1] == NULL) {
                fprintf(stderr, "ERROR: se necesita un archivo de salida\n");
                return 1;
            }
			//guardamos el nombre del archivo 
            output_file = args[i+1];
            append_mode = 0; // como el redireccionamineto es ">" indica que quiere trucar el archivo borrar su contenido si existe o crearlo
            i++;//saltamos el nombre del archivo 

			//redireccionamineo de salida que agrega al final del archivo
        } else if(strcmp(args[i], ">>") == 0) {
            if(args[i+1] == NULL) {
                fprintf(stderr, "ERROR: se necesita un archivo de salida\n");
                return 1;
            }
            output_file = args[i+1];
            append_mode = 1;//el modo sera append que agrega al final del archivo o lo crea si no existe 
            i++;

			//verificamos si es un proceso qeu se ejeutara en segundo plano 
        } else if(strcmp(args[i], "&") == 0) {
            
            background = 1;
        } else {

			//si no es un smbolo especial, es parte del comandos 
			//lo agregamos a la lista limpa filtrados 
            filtrados[filtrado_idx++] = args[i];
        }
    }
	//terminamos la lista limpa con NULL para que excevp sepa donde acaba 
    filtrados[filtrado_idx] = NULL;

    //si no hay comandos despues de fltrar regresa.
    if(filtrados[0] == NULL) {
        return 1;
    }

    //ejecutar comandos internos 
    if(strcmp(filtrados[0], "fantasma-rm") == 0) {
        return fantasma_rm(args);
    } 
    else if(strcmp(filtrados[0], "borrar_rastro") == 0) {
        return borrar_rastro(args);
    } 
    else if(strcmp(filtrados[0], "comando_xor") == 0) {
        return comando_xor(args);
    } 
    else if(strcmp(filtrados[0], "verificador-integridad") == 0){
		return verificador_integridad(args);
	}
    else if(strcmp(filtrados[0], "bit-log") == 0){
        return bit_log(args);
    }
    else {
        //ejecucion de comandos del sistem 
        pid_t pid;
        int status;

        pid = fork();

        if(pid == 0) {
            // Proceso hijo
            if(input_file != NULL) {
				//congirutar entrada si hibo '<'
				//abrirms el archivo de solo lectrua 
                int fd_in = open(input_file, O_RDONLY);
                if(fd_in < 0) {

                    perror("error al abrir el archivo de entrada");
                    exit(EXIT_FAILURE);
                }
				//cnecta ek archivo a la entrada estanda r
                dup2(fd_in, STDIN_FILENO);
				//cerrar el descriptor original 
                close(fd_in);
            }

            //redireccionamineto de salida 
            if(output_file != NULL) {
				//establecemos las banderas base la cual serai de escritrua si existe o crear si no existe
                int flags = O_WRONLY | O_CREAT;
				//dependiendo del modo que se eligio ">" o '>>' se agrega el modo, el modo que trunca o el que agrega al final en el archivo 
                if(append_mode) {
                    flags |= O_APPEND;
                } else {
                    flags |= O_TRUNC;
                }

				//abrimos el archivo con las banderas indicafas y asinamos los permisos de lectura/escritura para el usuario y de lectrua para los otros 
                int fd_out = open(output_file, flags, 0644);
                if(fd_out < 0) {
                    perror("Error creando el archivo de salida");
                    exit(EXIT_FAILURE);
                }
				//conecamos la salida estandar con el archivo
                dup2(fd_out, STDOUT_FILENO);
				//cerramos el descriptor 
                close(fd_out);
            }

            //la funcion execvp busca el programa en el PATH y lo ejecuta con los argumentos fltrados 
            if(execvp(filtrados[0], filtrados) == -1) {
                perror("ERROR al ejecutar el comando");
            }
			//llega aqui solo si excevp falla 
            exit(EXIT_FAILURE);
        } else if(pid < 0) {
			//indica un error en el fork
            perror("error en fork");
            return 1;
        } else {
            //proceso en el padre 
			//si no es un proceso en background esperamnos a que el hjo termine 
            if(!background) {
                do {
                    waitpid(pid, &status, 0);
                } while(!WIFEXITED(status) && !WIFSIGNALED(status));
            } else {
				//si es background imprimrimos el PID y lo seguimos esperando 
                printf("[%d] ejecutando en background\n", pid);
            }
        }
    }
    return 1;
}

/**
 * esta funcion modifica la lista original cortandola donde empiezan la redirecciones y extrar nombrres de archivos medinate punteror s
 * 
 * 
 */
int procesar_redirecciones_en_comando(char **args, char **input_file, char **output_file, int *append) {
    /**
	 * inicamos los valores de retorno 
	 */
	*input_file = NULL;
    *output_file = NULL;
    *append = 0;
    //recorremos la lista de los argumentos en busca de un redireccionamineto de entrada 
    for(int i = 0; args[i] != NULL; i++) {

		//aqui es donde se detecta 
        if(strcmp(args[i], "<") == 0 && args[i + 1] != NULL) {	
			//asignamos el nombre del archivo al puntero externo
            *input_file = args[i + 1];
            //ponemos en NULL donde esta aba "<" esto corta la lista para que excevp deje de ller aqui para que no trate de procesalos como comandos o parametros y esto realizara un error 
			args[i] = NULL;
            i++; //satamos al nombre de archivo del bucle 

			//detectamos si hay salida
        } else if(strcmp(args[i], ">") == 0 && args[i + 1] != NULL) {
			//agarramos el nombre
            *output_file = args[i + 1];
            *append = 0; //asigmaos el modo de salida 
            args[i] = NULL; //terminamos la lista aqui 
            i++;
			//la misma gata nomas revolcada 
        } else if(strcmp(args[i], ">>") == 0 && args[i + 1] != NULL) {
            *output_file = args[i + 1];
            *append = 1;
            args[i] = NULL;
            i++;
        }
    }
    return 0;
}



//solo acepta de 1 en 1 archivo 
int borrar_rastro(char **args) {
    pid_t pid;
    int status;
    //inicamos varibales por si el docuemntos conteine redireccionamiento 
    char *input_file = NULL;
    char *output_file = NULL;
    int append = 0; //modo de escrutura si es que lo tiene 
    
    //hacemos una copia de args, esto porque la funcion de procesar_reidrecciones va a modificiar el arreglo original.
    char *args_copia[MAX_ARGS];
    int i;
    for (i = 0; args[i] != NULL && i < MAX_ARGS - 1; i++) {
        args_copia[i] = args[i];
    }
    args_copia[i] = NULL;
    
    //procesamos redirecciones 
    procesar_redirecciones_en_comando(args_copia, &input_file, &output_file, &append);
    
    pid = fork();
    
    if (pid == 0) {
        //proceso hijo
        //si el usuario pidio leer de un archivo entonces abrumos el archivo 
        if (input_file != NULL) {
            int fd_in = open(input_file, O_RDONLY);
            if (fd_in < 0) {
                perror("Error abriendo archivo de entrada");
                exit(EXIT_FAILURE);
            }
            //desconectamos el teclado y conecamos al archivo abierto a la entrada estar del proceso 
            dup2(fd_in, STDIN_FILENO);
            close(fd_in);
        }
        
        //en la salida primoer debemos veriifcar que tipo de banderas y de escirutra quera, si agregar al final ">>" o truncar todo y escibir con los nuevos datos">"
        if (output_file != NULL) {
            int flags = O_WRONLY | O_CREAT;
            if (append) {
                flags |= O_APPEND;
            } else {
                flags |= O_TRUNC;
            }
            //abre o crea el archivo asignando los permisos para el usurio y solo de lectrua para grupo y otros 
            int fd_out = open(output_file, flags, 0644);
            if (fd_out < 0) {
                perror("Error creando archivo de salida");
                exit(EXIT_FAILURE);
            }
            //conectamos la salida de pantalla al archivo
            dup2(fd_out, STDOUT_FILENO);
            close(fd_out);
        }
        
        //argumentos del para el programa externo 
        char *new_args[MAX_ARGS];
        new_args[0] = "./borrar_rastro/borrar_rastro";
        int j = 1;
        //copiamos los argumentos limpios 
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
        //esperoa a que el hijo termine una vez hecho eso la shell toma el control 
        do {
            waitpid(pid, &status, 0);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }
    
    return 1;
}

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




//puede procesar mas de 1 archivo 
//los demas comandos trabajan de manera muy similar 

int fantasma_rm(char **args) {
    pid_t pid;
    int status;
    
    char *input_file = NULL;
    char *output_file = NULL;
    int append = 0;
    
    
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

int comando_xor(char **args) {
    pid_t pid;
    int status;

    pid = fork();

    if(pid == 0) {
        char *new_args[4];
        new_args[0] = "./comando_xor/comando_xor";
        new_args[1] = args[1];  // archivo
        new_args[2] = args[2];  // clave de cifrado
        new_args[3] = NULL;

        if(execvp(new_args[0], new_args) == -1) {
            fprintf(stderr, "error al buscar el directorio ./comando_xor/comando_xor\n");
        }
        exit(EXIT_FAILURE);
    } else if(pid < 0) {
        perror("error en fork");
    } else {
        // Proceso padre
        do {
            waitpid(pid, &status, 0);
        } while(!WIFEXITED(status) && !WIFSIGNALED(status));
    }
    
    return 1;
}


int verificador_integridad(char **args) {
    pid_t pid;
    int status;
    
    char *input_file = NULL;
    char *output_file = NULL;
    int append = 0;
    
    char *args_copia[MAX_ARGS];
    int i;
    for (i = 0; args[i] != NULL && i < MAX_ARGS - 1; i++) {
        args_copia[i] = args[i];
    }
    args_copia[i] = NULL;
    
    procesar_redirecciones_en_comando(args_copia, &input_file, &output_file, &append);
    
    pid = fork();
    
    if (pid == 0) {
        
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
            //0644 (rw-r--r--)
            int fd_out = open(output_file, flags, 0644);
            if (fd_out < 0) {
                perror("Error creando archivo de salida");
                exit(EXIT_FAILURE);
            }
            dup2(fd_out, STDOUT_FILENO);
            close(fd_out);
        }
        
        char *new_args[MAX_ARGS];
        new_args[0] = "./verificador-integridad/verificador";
        int j = 1;
        
        for (int k = 1; args_copia[k] != NULL && j < MAX_ARGS - 1; k++) {
            if (strlen(args_copia[k]) > 0) {
                new_args[j++] = args_copia[k];
            }
        }
        new_args[j] = NULL;
        
        execvp(new_args[0], new_args);
        
        //si execvp falla
        fprintf(stderr, "Error: No se encontró el ejecutable en %s\n", new_args[0]);
        perror("execvp");
        exit(EXIT_FAILURE);
    }
    else if (pid < 0) {
        perror("Error en fork");
        return 1;
    }
    else {
        do {
            waitpid(pid, &status, 0);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }
    
    return 1;
}

int bit_log(char **args) {
    pid_t pid;
    int status;
    
    char *input_file = NULL;
    char *output_file = NULL;
    int append = 0;
    
    
    char *args_copia[MAX_ARGS];
    int i;
    for (i = 0; args[i] != NULL && i < MAX_ARGS - 1; i++) {
        args_copia[i] = args[i];
    }
    args_copia[i] = NULL;
    
    procesar_redirecciones_en_comando(args_copia, &input_file, &output_file, &append);
    
    pid = fork();
    
    if (pid == 0) {

        
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
            if (append) flags |= O_APPEND;
            else flags |= O_TRUNC;
            
            int fd_out = open(output_file, flags, 0644);
            if (fd_out < 0) {
                perror("Error creando archivo de salida");
                exit(EXIT_FAILURE);
            }
            dup2(fd_out, STDOUT_FILENO);
            close(fd_out);
        }
        
        
        char *new_args[MAX_ARGS];
        
        new_args[0] = "./bit-log/bit-log"; 
        
        int j = 1;
        
        for (int k = 1; args_copia[k] != NULL && j < MAX_ARGS - 1; k++) {
            if (strlen(args_copia[k]) > 0) {
                new_args[j++] = args_copia[k];
            }
        }
        new_args[j] = NULL;
        
        
        execvp(new_args[0], new_args);
        
        // Si execvp falla:
        fprintf(stderr, "Error: No se encontró el ejecutable en %s\n", new_args[0]);
        perror("execvp");
        exit(EXIT_FAILURE);
    }
    else if (pid < 0) {
        perror("Error en fork");
        return 1;
    }
    else {
        
        do {
            waitpid(pid, &status, 0);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }
    
    return 1;
}
