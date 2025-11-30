#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<fcntl.h>
#include<string.h>
#include<string.h>
#include<sys/stat.h>
#include<errno.h>

//macros 

/**
 * @brief la direccion en donde se creara la ruta en donde se guardaran los datos, 
 * la cual es en la ruta raiz pero se crea de manera coculta en el sistema de archivos
 * 
 * 
 */
#define GHOSTZONE_BASE "/.andromeda_shell/.ghostzone/"
#define BUFFER_SIZE 1024

//funciones 

/**
 *
 * @brief crea el direvtorio fantasma si es que no esta creado 
 *
 * */
void crear_directorio_fantasma();


int mover_a_zona_fantasma(const char *archivo_org);


int procesar_archivos(const char *nombre_archivo);



int main(int argc, char *argv[]){
	
	//creo que n seria buena poner este cmando asi como asi, debe de haber una validacion que me indique si esta esta no lo crea pero si esta que lo cree
	crear_directorio_fantasma();
	int ret;
	
	if(argc > 1){
		
		//esta verificando que se le hayan pasados al menos 1 archivo a ocultar, se pueden pasar varios
		for(int i=1; i< argc; i++){

			ret= procesar_archivos(argv[i]);
		}
		
		return 0; //termina
	}

	

	char buffer[BUFFER_SIZE];
	int archivos_procesados = 0;

	while(fgets(buffer, sizeof(buffer), stdin)!=NULL){
	

		//elimina saltos de linea
		buffer[(strcspn(buffer, "\n"))]= 0;

		
		//ignora linea vacias
		if((strlen(buffer) == 0)) {
				
			continue;	
		}

		if(procesar_archivos(buffer)==0){
		
			archivos_procesados++;
		}

		if(archivos_procesados>0){
		
			printf("procesados %d archivos via entrada estandar\n", archivos_procesados);
		}


	
	}


	return 0;
}


void crear_directorio_fantasma(){

	char path[1024];

	snprintf(path, sizeof(path), "%s%s", getenv("HOME"), GHOSTZONE_BASE);

	//creamos el directorio
	mkdir(path,0700);
}



int procesar_archivos(const char *nombre_archivo){

	//verificando que el archivo existe 
	if(access(nombre_archivo, F_OK)==-1){
		fprintf(stderr, "Error archivo %s no existe\n", nombre_archivo);
		return -1;
	}


	//nombre del archivo sin ruta 
	char *nombre_base = strrchr(nombre_archivo, '/');
	if(nombre_base == NULL){
	
		nombre_base = (char *)nombre_archivo;
	}
	else{
		nombre_base++;
	}
	
	//construir ruta destino 
	
	char ruta_destino[1024];
	snprintf(ruta_destino, sizeof(ruta_destino),"%s%s%s", getenv("HOME"), GHOSTZONE_BASE, nombre_base);


	//abrir archivo original 
	int fd_original = open(nombre_archivo, O_RDONLY);
	if(fd_original == -1){
		perror("error abrinedo el archivo origen");
		return -1;
	}

	//crear archivo fantasma
	//
	int fd_destino = open(ruta_destino, O_WRONLY | O_CREAT | O_TRUNC, 0600);
	if(fd_destino == -1){
	
		perror("error al crear archivo fantasma");
		close(fd_original);
		return -1;
	}

	//transferir contenido 
	char buffer[4096];
	ssize_t bytes_leidos, bytes_escritos; 
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

	if(resultado ==0){
	
		//eliminar el archivo original si la transferencia fue exitosa 
		if(unlink(nombre_archivo)==-1){
		
			perror("Error eliminado el archivo original");
			return -1;
		}

		printf("MOVIENDO: %s -> %s", nombre_archivo, ruta_destino);
	}
	else{
	
		//si hiubo un error al escribri el archivo borrar el archivo original 
		//
		unlink(ruta_destino);
		fprintf(stderr, "Error: no se pudo mover %s\n", nombre_archivo);
	}

	return resultado;

}


