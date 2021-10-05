//Juan Carlos Ortiz de Montellano Bochelen
//A01656520
//Actividad-6
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#define TCP_PORT 8000
#define Time 30

//Variables
int cliente;
int next_pid;
int estado;
int intervalo;
//#=============================================================
//Metodos
//Manejadores de señales
void cambioEstado(int signal) {
    estado = 1;
    char state[] = "Verde"; 
    write(cliente, &state, sizeof(state));
    alarm(Time);
}

void cambioEstadoAdelante(int signal) {
    estado = 0;
    kill(next_pid, SIGUSR1);
}
//#=============================================================
//Manejadores de strings
//invertira el string que le demos 
void invertir(char s[]){
    char c;
    int j;
    for (int i = 0, j=strlen(s)-1; i < j; j++)
    {
        c = s[i];
        s[i] = s[j];
        s[j] = c;
    }
    
}
//dependiendo del int que le demos sera convertido a un string
void change(int n, char s[]){
    int i;
    int j;
    if ((j=n)<0)
    {
        n=-n;
    }else
    {
        i=0;
    }
    do
    {
        s[i++]=n %10 +'0';
    } while ((n /= 10) > 0);
    if (j<0)
    {
        s[i++]='-';
        s[i] = '\0';
        invertir(s);
}
    }
//#=============================================================
//Main
//Aqui se ejecutara nuestro programa por primera ocasion
int main(int argc, char const *argv[])
{
    //Nos conectamos con consola
    struct sockaddr_in direccion;
    char buf[1000];
    ssize_t lectura;
    ssize_t escritura;
    if (argc != 2) {
        exit(-1);
    }
    //creamos nuestro socket
    cliente = socket(PF_INET, SOCK_STREAM, 0);
    inet_aton(argv[1], &direccion.sin_addr);
    direccion.sin_port = htons(TCP_PORT);
    direccion.sin_family = AF_INET;
    //creamos nuestras señales
    sigset_t conjunto, pendientes;
    sigemptyset(&conjunto);
    sigaddset(&conjunto, SIGALRM);
    sigaddset(&conjunto, SIGUSR1);
    //levantaremos nuestra conexion
    escritura = connect(cliente, (struct sockaddr *) &direccion, sizeof(direccion));
    //Estableceremos las señales de los semaforos
    signal(SIGUSR1, cambioEstado);
    signal(SIGALRM, cambioEstadoAdelante);
    //obtendremos los PID de los semaforos
    change(getpid(), buf); 
    write(cliente, buf, sizeof(int));
    lectura = read(cliente, &buf, sizeof(buf));
    next_pid = atoi(buf);
    
    //recibiremos los cambios desde la consola
    while (lectura == read(cliente, &buf, sizeof(buf))) {
        if (strcmp(buf, "Inicio") == 0) {
            raise(SIGUSR1);
        } else if (strcmp(buf, "Alto") == 0 && intervalo != 2) {
            intervalo = 2;
            // Bloquear señales SIGUSR1 y SIGALRM
            sigprocmask(SIG_BLOCK, &conjunto, NULL);
        } else if (strcmp(buf, "INTERMITENT") == 0 && intervalo != 3) {
            intervalo = 3;
            // Bloquear señales SIGUSR1 y SIGALRM
            sigprocmask(SIG_BLOCK, &conjunto, NULL);
        }
        else {
            // Ranudar señales si se mandan por segunda vez (se reanudarán las señales pendientes continuando el ciclo)
            intervalo = 0;
            sigprocmask(SIG_UNBLOCK, &conjunto, NULL);
        }
    }
    close(cliente);
    return 0;
}

