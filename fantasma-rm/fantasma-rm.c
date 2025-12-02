#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<fcntl.h>
#include<string.h>
#include<sys/stat.h>
#include<errno.h> //para verificar erroes globales del sistem 

//macros 
#define GHOSTZONE_BASE "/.andromeda_shell/.ghostzone/"
#define GHOSTZONE_BASE_FATHER "/.andromeda_shell/"
#define GHOSTZONE_BASE_FINAL ".ghostzone/\0"
#define BUFFER_SIZE 1024

//funciones 
void crear_directorio_fantasma();
int procesar_archivos(const char *nombre_archivo);

int main(int argc, char *argv[]){


    crear_directorio_fantasma();
    
    
    int ret;
    
    if(argc > 1){
        for(int i=1; i< argc; i++){
            ret = procesar_archivos(argv[i]);
        }
        return 0;
    }

    char buffer[BUFFER_SIZE];
    int archivos_procesados = 0;

    while(fgets(buffer, sizeof(buffer), stdin)!=NULL){
        buffer[strcspn(buffer, "\n")] = 0;
        
        if(strlen(buffer) == 0) {
            continue;    
        }

        if(procesar_archivos(buffer)==0){
            archivos_procesados++;
        }
    }


    if(archivos_procesados>0){
        printf("procesados %d archivos via entrada estandar\n", archivos_procesados);
    }

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



int procesar_archivos(const char *nombre_archivo){
    // Verificando que el archivo existe 
    //la funcion lo access comprueba si en el proceso en el que se hace la llamada al sistema puede acceder a la ruta del archivo que se le paso
    //utilizando el modo "F_OK" solo se quiere saber si existe el archivo, la funcion regresa un 0 si la comprobacion de genera de manera exitosa
    //el archivo existe en la ruta dada, puede ser en nuestro directorio actual o una direccionde ruta absolula que se le haya pasado 
    if(access(nombre_archivo, F_OK)==-1){
        fprintf(stderr, "Error archivo %s no existe\n", nombre_archivo);
        return -1;
    }

    //nombre del archivo sin ruta 
    // printf("\n");
    // puts(nombre_archivo);


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
     * nos dio la funcion para tener el nombre limpio, 4567
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
    char ruta_destino[1024]; // -> /home/user/.andromeda_shell/.ghostzone/achivo.a
    snprintf(ruta_destino, sizeof(ruta_destino),"%s%s%s", getenv("HOME"), GHOSTZONE_BASE, nombre_base);

    //abrir archivo original 
    //la funcion open reotrna el valor del descriptor el cual es un numero entreo, en el caso que que este falle rregresarea -1
    //entonces lo que aqui se esta haceindo es abrir el archivo original el cual se paso por
    int fd_original = open(nombre_archivo, O_RDONLY);
    if(fd_original == -1){
        perror("error abriendo el archivo origen");
        return -1;
    }

    //crear archivo fantasma
    //abrimos o creamos el archivo en la ruta en donde se va a escbir los datos del archivo que 
    //indicamos que se cree si no existe, en solo lectura si existe se trinca a 0 bytes su contenido, y solo el otroga permidos de lectrua y 
    //escritura para el duenio 
    int fd_destino = open(ruta_destino, O_WRONLY | O_CREAT | O_TRUNC, 0600);
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
        if(unlink(nombre_archivo)==-1){
            perror("Error eliminando el archivo original");
            return -1;
        }
        
        printf("MOVIENDO: %s\n", nombre_archivo);
    }
    else{
        unlink(ruta_destino);
        fprintf(stderr, "Error: no se pudo mover %s\n", nombre_archivo);
    }

    return resultado;
}
