#ifndef LIB_FANTASMA.H
#define LIB_FANTASMA.H

//macros 
#define GHOSTZONE_BASE "/.andromeda_shell/.ghostzone/"

//funciones 
/**
 *
 * @brief crea el direvtorio fantasma si es que no esta creado 
 *
 * */
void crear_directorio_fantasma();


int mover_a_zona_fantasma(const char *archivo_org);


int procesar_archivos(const char *nombre_archivo);


#endif
