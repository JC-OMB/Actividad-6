//Juan Carlos Ortiz de Montellano Bochelen
//A01656520
//Actividad-6
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#define TCP_PORT 8000
//definimos nuestra cantidad de semaforos
#define Numero_Semaforos 4
//declararemos nuestros datos para controlar nuestras funciones
int intervalo;
int semaforo_hijo;
//#==========================================================================
//funciones

//interrupcion de señal en la consola
void interrupcion(int pin){
    //arreglo de estados
    char estados[15];

    //manejador de las señales
    if (intervalo!=1 && pin==20)
    {
        intervalo =1;
        //cambiando el estado a color rojo
        strcpy(estados,"Rojo");
    }else if(intervalo!=2 && pin ==2){
        intervalo=2;
        //cambiaremos el estado a amarillo
        strcpy(estados,"Amarillo");
    }else
    {
        //El semaforo continua con su funcion
        intervalo=0;
        return;

    }
}
//#==========================================================================
//funciones del semaforo

//Alto total
//cambiara nuestro semaforo a rojo
void alto(int pin){
    char transicion[]="Alto!";
    write(semaforo_hijo,transicion,sizeof(transicion));
}

//Luz amarilla
//Cambiara el estado de nuestro semaforo a amarillo
void lento(int pin){
    char transicion[]="Disminuya_Velocidad";
    write(semaforo_hijo,transicion,sizeof(transicion));
}

//Estados
//Cuando un estado cambie a verde se imprimiran los estados
void estado(int color){
    for (int i = 0; i < Numero_Semaforos; i++)
    {
        if (i==color)
        {
            printf("El semaforo esta de color Verde %d \n",i+1);
        }else
        {
            printf("El semaforo esta de color Rojo %d \n",i+1);
        } 
    }
}
//#==========================================================================
//main
//Aqui se ejecutara nuestro codigo por primera ocasion

int main(int argc, char const *argv[])
{
    struct sockaddr_in direccion;
    char buf[1000];
    //Crearemos nuestro socket
    int server;
    int adelante=1;
    int cliente[Numero_Semaforos];
    pid_t pid;
    ssize_t lectura;
    ssize_t escritura;
    server = socket(PF_INET,SOCK_STREAM,0);
    inet_aton(argv[1], &direccion.sin_addr);
    direccion.sin_family = AF_INET;
    direccion.sin_port = htons(TCP_PORT);
    char semaforo[Numero_Semaforos][50];
    ssize_t pidInputSizes[Numero_Semaforos];
    bind(server, (struct sockaddr *) &direccion, sizeof(direccion));

    //ignorador de señal
    if (argc !=2)
    {
        printf("La direccion IP es %s \n",argv[0]);
        exit(-1);
    }

    //Escucharemos nuestra señal
    listen(server,Numero_Semaforos);
    escritura = sizeof(direccion);
    //cantidad de conexiones por numero de semaforos 
    for (int i = 0; i < Numero_Semaforos; i++)
    {
        cliente[i]=accept(server,(struct sockaddr *) &direccion, &escritura);
        pid=fork();
        if (pid==0)
        {
            //gestionaremos nuestras señales
            semaforo_hijo = cliente[i];
            signal(SIGTSTP,alto);
            signal(SIGINT,lento);
            close(server);
            if (semaforo_hijo>=0)   
            {
                while (lectura == read(cliente[i], &buf, sizeof(buf)))
                {
                    estado(i);
                }
            }
            close(semaforo_hijo);    
        }
        else
        {
            pidInputSizes[i]=read(cliente[i], &semaforo[i], sizeof(semaforo[i]));
        }
    }
    if (pid>0)
    {
        for (int i = 0; i < Numero_Semaforos; i++)
        {
            int adelante = Numero_Semaforos % (i+1);
            write(cliente[i], &semaforo[adelante],pidInputSizes[adelante]);
        }
        char ciclo[]="El ciclo comenzara";
        write(cliente[0], &ciclo,sizeof(ciclo));
        while(wait(NULL)!=-1);
        close(server);
        
    }
    return 0;
}

