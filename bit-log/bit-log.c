#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// Ruta donde se guardará el archivo de log
#define LOGPATH "%s/.andromeda_shell/system.bitlog"

//que es lo que se necesita,
/**
 * el bit log de lo que se encargara? 
 * 
 * deberia de durante la shell el registro del archivo quedar "activo" en cada
 * operacion de la shell
 * 
 * entonces cuando se activa deberia de quedar en segundo plano para que no acapare
 * recursos para otros procesos, porque esta solo tendria qeu registrar las I/O
 * 
 * entonces seria suspendido hasta que una entrada lo active 
 * 
 * tal vez, que queda esperando a que se termine una operacion de I/O 
 * 
 * 
 * 
 */
typedef struct {
    unsigned long long timestamp;    
    char operacion;                  
    unsigned short longitud_ruta;    
    unsigned long long bytes;        
} RegistroLog;

