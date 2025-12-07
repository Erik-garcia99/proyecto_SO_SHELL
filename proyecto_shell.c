
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

// script
int fantasma_rm(char **args);
int borrar_rastro(char **args);

int main() {

	//buffer para guardar lo que ingresa el usaurio, todo lo que e usuario esbrime en la linea de comando, mas no guarda token o otra cosa garda crudo 
    char input[MAX_INPUT];

	//estatus para verificar si seguir o salir de la shell 
    int status = 1;

    printf("===shell===\n");
    printf("Escribe 'exit' para salir\n\n");

    while (status) {

        char cwd[1024];
        //obetenemos lla direccion actual en donde se encuentra el usuarios 
        if(getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("%s >> ", cwd);
        } else {
			//si ocurre un error se muestra el promrp de apoyo indicando que ocurrio un errro al obtener la direccion, sera como el "pwd"
            printf("+++> ");
        }

        fflush(stdout);
		//capturamos la entrada y lo guardamos en el buffer
        if (fgets(input, MAX_INPUT, stdin) == NULL) {
            break;
        }

        status = procesar_linea_comando(input);
    }
    return 0;
}

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
	 * strok en la primrea pasada devuelve un apuntador de la primera subcadena 
	 * que se encuentre que este delimitado por los caracteres
	 * 
	 * !!importante -> token va a modificar el parametro "line"
	 */

    token = strtok(line, " \t\n\r");
    
    while (token != NULL) {
        //lo que se hace es recorrer
        tokens[position] = strdup(token); //guarda el punetro de es asubcadena. 
        position++;

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


int procesar_linea_comando(char *line) {

	//eliminamos el salto de linea 
	//
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
        int status = procesar_comando(args);

        //liberamos la memoria 
        int i = 0;
        while(args[i] != NULL) {
            free(args[i]);
            i++;
        }
        free(args);
        return status;
    } else {
        //hay pipelines por lo que hay comandos a ejecutar 
        char *comandos[MAX_PIPES];

        int num_comandos = 0;

		
        char linea_copia[MAX_INPUT];
		//trabajmao con una varbale auxiliar para no alterar la linea original. 
		strcpy(linea_copia, line);
        //tratamos de separar los comandos o lo que se va a relizar entre los pipelines. 
        char *token = strtok(linea_copia, "|");
        while(token != NULL && num_comandos < MAX_PIPES) {
            // eliminaos espacios al inicio
            while(*token == ' ') token++;
            
            // elimina espacios al final
            char *end = token + strlen(token) - 1;
            while(end > token && *end == ' ') {
                *end = '\0';
                end--;
            }
            //Si el comando no esta vacio después de limpiar los espacios
            if(strlen(token) > 0) {
				//guarda una copia del comando 
                comandos[num_comandos] = strdup(token);
                num_comandos++;
            }
			//llama a strtok para obtnener el sigueinte tokendespes de la sigueinte '|'
            token = strtok(NULL, "|");
        }

		//verificar que haya al menos 2 comandos para poder considerar un piprline correcto 
        if(num_comandos < 2) {
			//si no es asi marcada un error y limpa la memoria 
            fprintf(stderr, "error: pipeline invalido\n");
            for(int i = 0; i < num_comandos; i++) {
                free(comandos[i]);
            }
            return 1;
        }

        // parsear el comando, 
		//creamos un arreglo tridimencional, esto porque ahora estoy diviendo por token los token creados de los diferentes comandos enlazados por un pipeline. 
        char ***comandos_args = malloc(num_comandos * sizeof(char**));
        for(int i = 0; i < num_comandos; i++) {
			//tenemos el arregl con los comandos con archivos algunso que queremos relizar por lo que ahora los necesitamso tokenisar para obtener que es lo que el usuario quiere , 
            comandos_args[i] = parse_input(comandos[i]);
			//libera la cadena de comando cruda
            free(comandos[i]);
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

    //dependineo de cuantos procesos creamos ese total de procesos 
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
    } else if(strcmp(filtrados[0], "borrar_rastro") == 0) {
        return borrar_rastro(args);
    } else if(strcmp(filtrados[0], "comando_xor") == 0) {
        return comando_xor(args);
    } else if(strcmp(filtrados[0], "verificador-integridad") == 0){
		return verificador_integridad(args);
	}else {
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

			//detectamos si hay salid 
        } else if(strcmp(args[i], ">") == 0 && args[i + 1] != NULL) {
			//garfamos el nombre
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
    
    // 1. Hacer copia de args para no modificar el original al buscar redirecciones
    char *args_copia[MAX_ARGS];
    int i;
    for (i = 0; args[i] != NULL && i < MAX_ARGS - 1; i++) {
        args_copia[i] = args[i];
    }
    args_copia[i] = NULL;
    
    // 2. Procesar redirecciones (detectar <, >, >>)
    procesar_redirecciones_en_comando(args_copia, &input_file, &output_file, &append);
    
    pid = fork();
    
    if (pid == 0) {
        // --- Proceso Hijo ---
        
        // Configurar redirección de entrada
        if (input_file != NULL) {
            int fd_in = open(input_file, O_RDONLY);
            if (fd_in < 0) {
                perror("Error abriendo archivo de entrada");
                exit(EXIT_FAILURE);
            }
            dup2(fd_in, STDIN_FILENO);
            close(fd_in);
        }
        
        // Configurar redirección de salida
        if (output_file != NULL) {
            int flags = O_WRONLY | O_CREAT;
            if (append) {
                flags |= O_APPEND;
            } else {
                flags |= O_TRUNC;
            }
            // Permisos 0644 (rw-r--r--)
            int fd_out = open(output_file, flags, 0644);
            if (fd_out < 0) {
                perror("Error creando archivo de salida");
                exit(EXIT_FAILURE);
            }
            dup2(fd_out, STDOUT_FILENO);
            close(fd_out);
        }
        
        // 3. Construir argumentos para el ejecutable externo
        // La estructura de tu proyecto sugiere que el binario está en una subcarpeta
        char *new_args[MAX_ARGS];
        new_args[0] = "./verificador-integridad/verificador";
        int j = 1;
        
        // Copiar los argumentos limpios (sin los símbolos de redirección)
        for (int k = 1; args_copia[k] != NULL && j < MAX_ARGS - 1; k++) {
            if (strlen(args_copia[k]) > 0) {
                new_args[j++] = args_copia[k];
            }
        }
        new_args[j] = NULL;
        
        // 4. Ejecutar el comando externo
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
        // --- Proceso Padre ---
        do {
            waitpid(pid, &status, 0);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }
    
    return 1;
}