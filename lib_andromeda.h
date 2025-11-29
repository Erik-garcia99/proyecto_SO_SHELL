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

char **parse_input(char *line);


/** 
 *
 *@info esta funcion solo es como para debuggear en el formato final no sera utilada 
 *
 *@brief imprime la infromacion de lo que se ingrese, solo apa confirmar 
 *
 *
 *@params args -> argumentos que se ingresaron como e comando y sus parametros por dar un ejemplo 
 *
 *
 * @return NULL
*/

void print_command_info(char **args);



//aqui van las funciones del deep y ver como las puedo adaptar 





int ejecuta_comandos_sistema(char **args);


int borrar_rastro(char **args);

int procesar_comando(char **args);




#endif
