#ifndef LIB_ANDROMEDA_H
#define LIB_ANDROMEDA_H


//macros 

#define MAX_INPUT 1024
#define MAX_ARGS 64

//varibales globales <aunque lo mejor es no tener pero por si acaso>


//declaracion de funciones 

/***
*
*@brief genera los tokesn para el proceso de comandos 
*@params line apuntador de la linea actual de entrada 
*@retun toekens -> entrada separa en tokens
*
*/



int ejecuta_comandos_sistema(char **args);


int borrar_rastro(char **args);

int procesar_comando(char **args);




#endif
