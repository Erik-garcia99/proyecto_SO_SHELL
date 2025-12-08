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


	//procesando archivo via arguemntos normal,     
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

    
	// -> /home/dmrx/.andromeda_shell/ -> tratare de crear primero
	int n=snprintf(path_F, sizeof(path_F), "%s%s", getenv("HOME"), GHOSTZONE_BASE_FATHER);
	
	if(mkdir(path_F, 0700) == 0){
        
       		write(1,exito,strlen(exito));
        	write(1,padre,strlen(padre));
		write(1,"\n",1);



	}
	else if (errno == EEXIST){

	else{
        	//otro tipo de error
		perror("error al crear la carpeta padre");
    		write(1,"\n",1);

	}


	//ahora debemos de crear desntro de la carpeta .andromeda_shell -> .ghostzone/
    char directorio_oct[]=".ghostzone/";

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
     
    if(access(nombre_archivo, F_OK)==-1){
        fprintf(stderr, "Error archivo %s no existe\n", nombre_archivo);
        return -1;
    }
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


    int fd_original = open(nombre_archivo, O_RDONLY);
    if(fd_original == -1){
        perror("error abriendo el archivo origen");
        return -1;
    }
 
    int fd_destino = open(ruta_destino, O_WRONLY | O_CREAT | O_TRUNC, 0600);
    if(fd_destino == -1){
        perror("error al crear archivo fantasma");
        close(fd_original);
        return -1;
    }
 
    char buffer[4096]; //la cantidad de bytes que se estran escribiendo en cada psada. 

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


 
    if(resultado == 0){
        //eliminar el archivo original si la transferencia fue exitosa 
        if(unlink(nombre_archivo)==-1){
            perror("Error eliminando el archivo original");
            return -1;
        }
        
        printf("MOVIENDO: %s\n", nombre_archivo);
    }
    else{
	//si no se pudo mover el archivo eliminaos el archivo destini porque no se usara.
        unlink(ruta_destino);
        fprintf(stderr, "Error: no se pudo mover %s\n", nombre_archivo);
    }

    return resultado;
}
