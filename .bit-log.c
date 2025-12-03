#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

//define la ruta del archivo de log 
#define LOGPATH "%s/.crypto_shell/system.bitlog"

void w64(unsigned char *b, unsigned long long v){
    for(int i=0;i<8;i++) b[i]=v>>(i*8);
}
unsigned long long r64(unsigned char *b){
    unsigned long long v=0;
    for(int i=7;i>=0;i--) v=(v<<8)|b[i];
    return v;
}
void w16(unsigned char *b,unsigned short v){
    b[0]=v; b[1]=v>>8;
}
unsigned short r16(unsigned char *b){
    return b[0]|(b[1]<<8);
}

int main(int c,char *v[]){
    if(c<2){
        write(1,"Uso: ./bit-log record <ruta> R|W <bytes>\n     ./bit-log view\n",61);
        return 1;
    }

    char *h=getenv("HOME")?getenv("HOME"):".";
    char log[512];
    snprintf(log,512,LOGPATH,h);
    mkdir(strcat(strcpy((char[512]){},h),"/.crypto_shell"),0700);

    if(!strcmp(v[1],"record") && c>=5){
        int fd=open(log,O_WRONLY|O_CREAT|O_APPEND,0600);
        if(fd<0){ write(2,"Error log\n",10); return 1; }

        unsigned char b[19];
        w64(b,time(NULL));
        b[8]=v[3][0];
        unsigned short L=strlen(v[2]);
        w16(b+9,L);
        w64(b+11,strtoull(v[4],0,10));

        write(fd,b,19);
        write(fd,v[2],L);
        fsync(fd);
        close(fd);
        write(1,"Registrado\n",11);
        return 0;
    }

    if(!strcmp(v[1],"view")){
        int fd=open(log,O_RDONLY);
        if(fd<0){ write(2,"Sin registros\n",14); return 1; }

        unsigned char b[19];
        char path[1024],line[1200];
        write(1,"\n=== Registros ===\n\n",21);

        while(read(fd,b,19)==19){
            unsigned short L=r16(b+9);
            if(read(fd,path,L)!=L)break;
            path[L]=0;

            time_t ts=r64(b);
            struct tm *t=localtime(&ts);
            int n=snprintf(line,1200,"%04d-%02d-%02d %02d:%02d:%02d  %c  %llu bytes  %s\n",
                t->tm_year+1900,t->tm_mon+1,t->tm_mday,t->tm_hour,t->tm_min,t->tm_sec,
                b[8],r64(b+11),path);
            write(1,line,n);
        }
        close(fd);
        return 0;
    }

    write(2,"Opcion invalida\n",16);
    return 1;
}
