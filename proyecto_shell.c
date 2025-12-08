

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
int bit_log(char **args);
// script
int fantasma_rm(char **args);
int borrar_rastro(char **args);

//en la funcion main se toma el dato en crudo del usuarios 
int main() {

	//buffer para guardar lo que ingresa el usaurio
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
        if (fgets(input, MAX_INPUT, stdin) == NULL) {
            break;
        }
        status = procesar_linea_comando(input);
    }
    return 0;
}

char **parse_input(char *line) {

    int position = 0;

    char **tokens = malloc(MAX_ARGS * sizeof(char*));

	//apuntador para guarda cada token que se extrae
    char *token;

	//verificar que se haya asignado correctamente la memoria
    if (!tokens) {
        fprintf(stderr, "Error de asignación de memoria\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, " \t\n\r");
    
    while (token != NULL) {
        tokens[position] = strdup(token); 
        position++;

        if (position >= MAX_ARGS) {
            break;
        }
		

        token = strtok(NULL, " \t\n\r");
    } 
    tokens[position] = NULL;
    return tokens;
}


int procesar_linea_comando(char *line) { 
    line[strcspn(line, "\n")] = 0;
	

	if(strlen(line) == 0) {
        return 1;
    }

    // verificar si hay pipeline 
    char *pipe_pos = strchr(line, '|');
    
    if(pipe_pos == NULL) {
		  
        // comando simple
		
        char **args = parse_input(line);
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
        
        //hay pipelines 
        char *comandos[MAX_PIPES];

        int num_comandos = 0;

		
        char linea_copia[MAX_INPUT];
		
		strcpy(linea_copia, line);
        
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
           
            if(strlen(token) > 0) {
				//guarda una copia del comando 
                comandos[num_comandos] = strdup(token);
                num_comandos++;
            }
			
            token = strtok(NULL, "|");
        }

        if(num_comandos < 2) {
			 
            fprintf(stderr, "error: pipeline invalido\n");
            for(int i = 0; i < num_comandos; i++) {
                free(comandos[i]);
            }
            return 1;
        }

       
        char ***comandos_args = malloc(num_comandos * sizeof(char**));
        for(int i = 0; i < num_comandos; i++) {
			
            comandos_args[i] = parse_input(comandos[i]);
			//libera la cadena de comando cruda
            free(comandos[i]);
        }

        //ejecutar pipeline
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

    int pipes[num_comandos - 1][2];

	
    pid_t pids[num_comandos];
    int i;

  
    for(i = 0; i < num_comandos - 1; i++) {

        if(pipe(pipes[i]) == -1) {
            perror("error creando pipe");
            return 1;
        }
    }

   
    for(i = 0; i < num_comandos; i++) {

        pids[i] = fork();

        if(pids[i] == 0) {
            
            if(i > 0) {
            
			
                dup2(pipes[i-1][0], STDIN_FILENO);
            }
			
            if(i < num_comandos - 1) {
                
                dup2(pipes[i][1], STDOUT_FILENO);
            }

           
            for(int j = 0; j < num_comandos - 1; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }

         
            ejecuta_comandos_simple(comandos[i]);
         
			exit(EXIT_SUCCESS);  
        } else if(pids[i] < 0) {

			//marcamos error si no se pudo crear un proceos 
            perror("error en fork");
            return 1;
        }
    } 
    for(i = 0; i < num_comandos - 1; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);  
    }

     
    for(i = 0; i < num_comandos; i++) {
        waitpid(pids[i], NULL, 0); 
    }
    
    return 1;
}

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
 
        if(strcmp(args[i], "<") == 0) {

            if(args[i+1] == NULL) {
                fprintf(stderr, "ERROR: se necesita un archivo de entrada\n");
                return 1;
            }

            input_file = args[i+1];
            i++; 

			//es redireccionamineto de slaida?,
        } else if(strcmp(args[i], ">") == 0) {
             
            if(args[i+1] == NULL) {
                fprintf(stderr, "ERROR: se necesita un archivo de salida\n");
                return 1;
            }
			
            output_file = args[i+1];
            append_mode = 0;
            i++;//saltamos el nombre del archivo 

			//redireccionamineo de salida que agrega al final del archivo
        } else if(strcmp(args[i], ">>") == 0) {
            if(args[i+1] == NULL) {
                fprintf(stderr, "ERROR: se necesita un archivo de salida\n");
                return 1;
            }
            output_file = args[i+1];
            append_mode = 1;
            i++;

			
        } else if(strcmp(args[i], "&") == 0) {
            
            background = 1;
        } else {

			
            filtrados[filtrado_idx++] = args[i];
        }
    }

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
				
                int fd_in = open(input_file, O_RDONLY);
                if(fd_in < 0) {

                    perror("error al abrir el archivo de entrada");
                    exit(EXIT_FAILURE);
                }
				
                dup2(fd_in, STDIN_FILENO);
				
                close(fd_in);
            }

            //redireccionamineto de salida 
            if(output_file != NULL) {
				//establecemos las banderas base la cual serai de escritrua si existe o crear si no existe
                int flags = O_WRONLY | O_CREAT;
				
                if(append_mode) {
                    flags |= O_APPEND;
                } else {
                    flags |= O_TRUNC;
                }

				
                int fd_out = open(output_file, flags, 0644);
                if(fd_out < 0) {
                    perror("Error creando el archivo de salida");
                    exit(EXIT_FAILURE);
                }
				
                dup2(fd_out, STDOUT_FILENO);
                close(fd_out);
            }

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
           
			args[i] = NULL;
            i++;  

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


int borrar_rastro(char **args) {
    pid_t pid;
    int status;
    //inicamos varibales por si el docuemntos conteine redireccionamiento 
    char *input_file = NULL;
    char *output_file = NULL;
    int append = 0; //modo de escrutura si es que lo tiene 
    
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
            /
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
