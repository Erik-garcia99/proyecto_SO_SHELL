/*
garcia chavez erik  01275863
Armando Tepale Chocolat  1280222
Sistemas operativos 2025-2
proyecto <adromeda_shell>
ingenieria en computacion 
UABC
*/


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h> //directorio y archivos 
#include <errno.h>
#include <dirent.h> //directorio

//macros 
#define GHOSTZONE_BASE "/.andromeda_shell/.ghostzone/"
#define GHOSTZONE_BASE_FATHER "/.andromeda_shell/"
#define GHOSTZONE_BASE_FINAL ".ghostzone/"
#define BUFFER_SIZE 1024

//funciones

//esta funcion debe de camibar debe de recibir igual 
void crear_directorio_fantasma();
//int procesar_archivos(const char *nombre_archivo);


int procesar_archivos(const char *nombre_archivo, const char *destino_personal);

int mover_recursivo(const char *origen, const char *destino);
int copiar_borrar(const char *origen, const char *destino);



int main(int argc, char *argv[]){


    	crear_directorio_fantasma();
    
	int ret;
	
	int archivos_procesador=0;
	char *destino_personal=NULL;

	int count_argc = argc;
	char **list_argv = malloc(argc * sizeof(char*));

    //hacer una copia de los argumentos originales para no afectar a los originales 
    for(int i=0; i< count_argc;i++){
        list_argv[i]= argv[i];
    }

	
	//el comando tiene un parametro el cual es "-d" indicando que queremos enviar el archivo/carpeta a un directroiro especial o otra ruta que el usuario indique si no esta entonces toma la ruta predefinida que se creo en la funcion "crear_directorio()".
	for(int i=0; i< count_argc ;i++){
	
		if(strcmp(list_argv[i], "-d")==0){
		
			if(i+1 < count_argc){
			
				destino_personal= list_argv[i+1];

				if(mkdir(destino_personal, 0700)==0){
					list_argv[i] = NULL; //-d se elimina
				    list_argv[i+1] =NULL; // la ruta se elimina 
				    i++;
                    
				}

                else if(errno ==EEXIST){
                    //el directroiro ya existe
                }
                else{
                    perror("error creando el directorio personal"); 
					return 1;
                }

				
			}
			else{
			
				fprintf(stderr, "se espera un nombre de directorio\n");
				return 1;
			}
		
		}
	
	}

    //este es el caso cunado no es una ruta personal si no que es la ruta por defecto 
	if(count_argc > 1){
		for(int i=1; i< count_argc; i++){
		
			if(list_argv[i]!=NULL){
			
				procesar_archivos(list_argv[i],destino_personal);
			}
		}
	
	}


    
	//entrada del nombre de archivo por medio de pipelines

    //la primera condicion, si no hubi arugmentos, nombre de archivos etc. quiere decir que debe de lleer el archivo desde stdin 

    //la segunda comparacion quiere decir que el usuario ingreso 3 argumentos el programa el parametro -d y la ruta destimo pero no el archivo que se tiene que guardan por lo que igual debe de leerlo desde la entrda estandar 
	if(count_argc <=1 || (count_argc == 3 && destino_personal!=NULL)){
	
		char buffer[BUFFER_SIZE];
		while(fgets(buffer, sizeof(buffer), stdin)!=NULL){
		
			buffer[strcspn(buffer, "\n")]=0;
            //pregunta si despues de elininar del buffer el salto de linea quiere decir que es una linea vacia por lo que salta lo que hay despues y no procesoa el buffer porque no es nada. 
			if(strlen(buffer)==0) continue;
			procesar_archivos(buffer,destino_personal);
			archivos_procesador++;
		}
	
	}

    free(list_argv);
    return 0;
}

void crear_directorio_fantasma(){
    
	char path[1024]; //este contendra la ruta final para crear todo el sismtea de archivos
	char path_F[1024];
	
	char exito[]="se ha creado el directorio con exito!";
	char fallo[]="fallo al crear el directorio directorio existente";
	
	char padre[]=".andormeda_shell/\n";//este es solo para imprimir 
	//const char *lf='\n';
    
	// -> /home/dmrx/.andromeda_shell/ -> tratare de crear primero
	int n=snprintf(path_F, sizeof(path_F), "%s%s", getenv("HOME"), GHOSTZONE_BASE_FATHER);
	
	//si EEXIT se establce quiere decir que el archivo que se quiere crear ya existe, por lo que el archivo no debe existor

	//cunado errno !=EEXIST quiere decir que ocurrio un error y no se puede crear el directorio 
	if(mkdir(path_F, 0700) == 0){
        
       		write(1,exito,strlen(exito));
        	write(1,padre,strlen(padre));
		write(1,"\n",1);
        //write(1,lf, strlen(lf));
        //hasta este punteo solo se ha creado el sigueinte sirectorio si es que tiene exito 
        // -> /home/dmrx/.andromeda_shell/ <- este es el directorio creado si todo ha salido bien. 


	}
	else if (errno == EEXIST){
        //verificando que no exista 
		//write(1,fallo,strlen(fallo));
       		//write(1,padre,strlen(padre));
        	//write(1,"\n",1);
	}
	else{
        	//otro tipo de error
		perror("error al crear la carpeta padre");
    		write(1,"\n",1);

	}


	//ahora debemos de crear desntro de la carpeta .andromeda_shell -> .ghostzone/
    char directorio_oct[]=".ghostzone/";
        //por ahora se que path_F contiene la ruta hasta .andromeda_shell, 
    strcat(path_F, directorio_oct);

	//en este momento el arreglo path_F tiene la sigueinte rua: /home/dmrx/.andormeda_shell/.ghostzone/ 
    // puts(path_F);

	if(mkdir(path_F,0700)==0){

		//se pudo crear el directorio 
		write(1,exito,strlen(exito));
		write(1,directorio_oct,strlen(directorio_oct));
		write(1,"\n",1);

	}
	else if(errno == EEXIST){
		//write(1,fallo,strlen(fallo));

	}
	else{

		//ocurrio otro error 
		perror("error al crear el directorio");
	}

}



int procesar_archivos(const char *nombre_archivo, const char *destino_personal){
    // Verificando que el archivo existe 
    //la funcion lo access comprueba si en el proceso en el que se hace la llamada al sistema puede acceder a la ruta del archivo que se le paso
    //utilizando el modo "F_OK" solo se quiere saber si existe el archivo, la funcion regresa un 0 si la comprobacion de genera de manera exitosa
    //el archivo existe en la ruta dada, puede ser en nuestro directorio actual o una direccionde ruta absolula que se le haya pasado 
    if(access(nombre_archivo, F_OK)==-1){
        fprintf(stderr, "Error archivo %s no existe\n", nombre_archivo);
        return -1;
    }


    /**
     * strrchr lo que hace es buscar dentro de la ruta o de la cadena que se le paso la utlima posicion en donde aparece x caractere en este caso
     * se le esta diciendo que queremos saber en donde esta el ultimo '/' esto porque?
     * nuestro shell tiene que tener la posibilidad de darle una ruta absoluta, entonces una ruta absolita lo que contendra seria 
     * /home/usuer/docuemntos/calificaciones.txt por dar un ejemplo 
     *la funcion regresara un punto a la posicion del ultimo '/' en este caso pero busca cualquier caracter. 
     * 
     * 
     * ahora la funcion puede regresar 2 cosas como lo vimos, un apuntador o NULL, que significa el NULL? NULL es cuando no encunetra el 
     * caracter puede significar que el archivo que le mandamos esta en nuestra ruta relaviva podria decirce en nuestro directorio en donde se
     * encuentra el scrpit por lo que el nombre base del archivo es el mismo que del archivo solo se le hace un casting para convetirlo en su tipo puntero 
     * 
     * en otro caso si es una ruta absoluta entonces, queremos solo el nombre del archivo por lo que debemos empezar una posicion despues de lo que
     * nos dio la funcion para tener el nombre limpio, 
     * 
     */
    char *nombre_base = strrchr(nombre_archivo, '/');
    if(nombre_base == NULL){
        nombre_base = (char *)nombre_archivo;
    }
    else{
        nombre_base++;
    }
    
    //construir ruta destino 
    char ruta_destino[1024]; 
	
	if(destino_personal!=NULL){
		//hibo -d
		// cunado el usuario ingreso la / al final de la carpeta donde quiere que se guarde el archivo para que el programa no lo ponga y se solapen y 
		if(destino_personal[strlen(destino_personal)-1]=='/'){
			//quiere decir que hubo alguna rua 
			snprintf(ruta_destino, sizeof(ruta_destino), "%s%s", destino_personal,nombre_base);
		
		}
		else{
            //en el caso que no sea asi solo se le agrega 
			snprintf(ruta_destino, sizeof(ruta_destino),"%s/%s",destino_personal, nombre_base);
		}
	
	}
	else{
		//opcion de base 
    		snprintf(ruta_destino, sizeof(ruta_destino),"%s%s%s", getenv("HOME"), GHOSTZONE_BASE, nombre_base);
	}
	

    ///////////////////////////////////////////////////



    return mover_recursivo(nombre_archivo, ruta_destino);
}

//esta funcion verifica si es un directrio o es un archivo ordinario 
int mover_recursivo(const char *origen, const char *destino){

    struct stat st;
    //usamos lstat para enlaces simbolicos si es que los hubiera 
    if(lstat(origen, &st)==-1){

        perror("error obtenido los metadatos del archivo");
        return -1;
    }

    if(S_ISDIR(st.st_mode)){

        //si es un directorio lo que nos mando creamos el directorio destino
        if(mkdir(destino, 0700)==0){
			list_argv[i] = NULL; //-d se elimina
		    list_argv[i+1] =NULL; // la ruta se elimina 
		    i++;
                    
		}

        else if(errno ==EEXIST){
            //el directroiro ya existe
        }
        else{
            perror("error creando el directorio personal"); 
			return 1;
        }
        //abrimos la carpeta. 
        DIR *dir = opendir(origen);
        if(!dir){
            perror("error abriendo directorio origen");
            return -1;
        }

        struct dirent *entrada;
        while((entrada=readdir(dir)) !=NULL){
            //va recorriendo los diferentes archivos hasta terminal en la ruta actual en donde se encuentra 

            //importante saltan las referencias porque sin esto intentara moverse dentro de si mismo infinitamente
            if(strcmp(entrada->d_name,".") == 0 || strcmp(entrada->d_name,"..") == 0){
                continue;
            }

            //rutas nuevas en caso de ser un directroior 
            char nuevo_origen[1024];
            char nuevo_destino[1024];

            snprintf(nuevo_origen, sizeof(nuevo_origen), "%s/%s", origen, entrada->d_name);
            snprintf(nuevo_destino, sizeof(nuevo_destino), "%s/%s", destino, entrada->d_name);
            //ira construyendo los directorio dentro del archivo si es que los tiene 
            mover_recursivo(nuevo_origen, nuevo_destino);

        }

        closedir(dir);

        if(rmdir(origen)==-1){
            perror("error al elminar el directrior");
            return -1;
        }
        char mess[]="MOVIENDO DIRECTORIO:";
        write(1, mess, strlen(mess));
        write(1, origen, strlen(origen));

    }
    else{
        //se topo con un archivo ordinario
        return copiar_borrar(origen, destino);

    }


}


int copiar_borrar(const char *origen, const char *destino){
    
    //abrir archivo original 
    //la funcion open reotrna el valor del descriptor el cual es un numero entreo, en el caso que que este falle rregresarea -1
    //entonces lo que aqui se esta haceindo es abrir el archivo original el cual se paso por
    int fd_original = open(origen, O_RDONLY);
    if(fd_original == -1){
        perror("error abriendo el archivo origen");
        return -1;
    }

    //crear archivo fantasma
    //abrimos o creamos el archivo en la ruta en donde se va a escbir los datos del archivo que 
    //indicamos que se cree si no existe, en solo lectura si existe se trinca a 0 bytes su contenido, y solo el otroga permidos de lectrua y 
    //escritura para el duenio 
    int fd_destino = open(destino, O_WRONLY | O_CREAT | O_TRUNC, 0600);
    if(fd_destino == -1){
        perror("error al crear archivo fantasma");
        close(fd_original);
        return -1;
    }

    //proceso para transferir archivos. 


    //transferir contenido 
    char buffer[4096]; //la cantidad de bytes que se estran escribiendo en cada psada. 

    /**
     * lo que devulve las funciones read() y write() es el numero de bytes leidos o escritos dependiendo el caso esto lo usaremos para saber 
     * si no han ocurrido errores al transferir. 
     * 
     * 
     */
    int bytes_leidos, bytes_escritos; 
    
    
    int resultado = 0; 

    while((bytes_leidos = read(fd_original, buffer, sizeof(buffer))) > 0){
        bytes_escritos = write(fd_destino, buffer, bytes_leidos);
        if(bytes_escritos != bytes_leidos){
            perror("Error en la escritura");
            resultado = -1;
            break;
        }
    }

    close(fd_original);
    close(fd_destino);


    //siempre y cunado no haya ocurrio un error al escribir el archivo resultara en unn exito el mover el archivo en otro caso mostrara un error, si es exitoso borramos el archivo en la direccion en donde se los paso. 
    if(resultado == 0){
        //eliminar el archivo original si la transferencia fue exitosa 
        if(unlink(origen)==-1){
            perror("Error eliminando el archivo original");
            return -1;
        }
        
        printf("MOVIENDO: %s\n", origen);
    }
    else{
	//si no se pudo mover el archivo eliminaos el archivo destini porque no se usara.
        unlink(destino);
        fprintf(stderr, "Error: no se pudo mover %s\n", origen);
    }


}
