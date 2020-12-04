#include <stdio.h>
#include <stdlib.h>
#include "SerialManager.h"
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>

char buffer_receive_tcp[10];
_Bool flag_data = 0;
_Bool flag_socket = 0;
volatile sig_atomic_t got_sig;

int initSocketTCP( void );

void sig_handler(int sig);

void createSig( void );

void bloquearSign(void);

void desbloquearSign(void);

void* serial_thread (void* fd);

int main (void)
{
	socklen_t addr_len;
	struct sockaddr_in clientaddr;
	int newfd;
	int n;
	int ret;
	pthread_t serial_thread_handler;

	// Inicializo servidor TCP
	int s = initSocketTCP();

	// Definicion SIGN y handler
	createSign();

	while(1)
	{
		char ipClient[32];

		// Ejecutamos accept() para recibir conexiones entrantes
		addr_len = sizeof(struct sockaddr_in);
    	if ( (newfd = accept(s, (struct sockaddr *)&clientaddr, &addr_len)) == -1)
      	{
			perror("error en accept");
		    exit(1);
	    }

		bloquearSign();	// Bloqueo SIGN
		// Creo Thread para manejar comunicacion con puerto serie
		ret = pthread_create (&serial_thread_handler, NULL, serial_thread, (void*) &newfd); //Le paso como parametro el buffer de recepcion de TCP client
		desbloquearSign();	// Desbloqueo SIGN
		
		if(ret)
		{
			errno = ret;
			perror("pthread_create");
			return -1;
		}

		inet_ntop(AF_INET, &(clientaddr.sin_addr), ipClient, sizeof(ipClient));
		printf  ("server:  conexion desde:  %s\n",ipClient);

		flag_socket = 1;	// Indico que la conexión con el socket esta activa
		
		while( flag_socket == 1 )
		{
			// Funcion de lectura bloqueante
        	n = read(newfd,buffer_receive_tcp,128);
        	switch ( n )
			{
        		case -1:
                	perror("Error leyendo mensaje en socket");
					exit(1);
                	break;
            	case 0:
                	flag_socket = 0;	// Indico que el socket se desconecto.
                	break;
            	default:
					flag_data = 1;
                	break;
        	}
			if (got_sig)
			{
				pthread_cancel(serial_thread_handler);
	    		pthread_join (serial_thread_handler, NULL);
				close(newfd);
			}
		}

        pthread_cancel(serial_thread_handler);
	    pthread_join (serial_thread_handler, NULL);
		close(newfd);
	}
}

/* Thread */
void* serial_thread (void* fd)
{
	int bytes_received;
	char buffer[10];

	printf("Inicio Serial Service\r\n");
	
	serial_open(1, 115200);

	while(1)
	{
		bytes_received = serial_receive(buffer, sizeof(buffer));
		if (bytes_received != 0)
		{
			if (flag_socket == 1)
			{
				// Enviamos mensaje a cliente
    			if (write (*(int *)fd, buffer, strlen(buffer)) == -1)
    			{
      				perror("Error escribiendo mensaje en socket");
      				exit (1);
        		}
			}
		}

		if(flag_data)
		{
			//printf("%s\r\n",buffer_receive_tcp);
			serial_send(buffer_receive_tcp,sizeof(buffer_receive_tcp));
			flag_data = 0;
		}
		
		//Necesita un delay porque sino satura el programa de python
	    usleep(200000);
	}
}

/* Inicializacion sockect TCP */
int initSocketTCP( void )
{
	struct sockaddr_in serveraddr;

	// Creamos socket
	int s = socket(AF_INET,SOCK_STREAM, 0);

	// Cargamos datos de IP:PORT del server
	bzero((char*) &serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(10000);
	if(inet_pton(AF_INET, "127.0.0.1", &(serveraddr.sin_addr))<=0)
	{
		fprintf(stderr,"ERROR invalid server IP\r\n");
		return 0;
	}

	// Abrimos puerto con bind()
	if (bind(s, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) == -1) 
	{
		close(s);
		perror("listener: bind");
		return 0;
	}

	// Seteamos socket en modo Listening
	if (listen (s, 10) == -1) // backlog=10
	{
		perror("error en listen");
		exit(1);
	}
	return s;
}

/*******************************
 * Funciones de manejo de SIGN *
 *******************************/
void sig_handler(int sig)
{
	got_sig = 1;
}

void createSign( void )
{
	struct sigaction sa_sigint;
	struct sigaction sa_sigterm;

	sa_sigint.sa_handler = sig_handler;
	sa_sigint.sa_flags = 0; // SA_RESTART;
	sigemptyset(&sa_sigint.sa_mask);

	sa_sigterm.sa_handler = sig_handler;
	sa_sigterm.sa_flags = 0; // SA_RESTART;
	sigemptyset(&sa_sigterm.sa_mask);

	if (sigaction(SIGINT,&sa_sigint,NULL) == -1) 
	{
		perror("sigaction");
		exit(1);
	}
	if (sigaction(SIGTERM,&sa_sigterm,NULL) == -1) 
	{
		perror("sigaction");
		exit(1);
	}
}

void bloquearSign(void)
{
    sigset_t set;
    int s;
    sigemptyset(&set);
    sigaddset(&set, SIGINT);
    //sigaddset(&set, SIGUSR1);
    pthread_sigmask(SIG_BLOCK, &set, NULL);
}

void desbloquearSign(void)
{
    sigset_t set;
    int s;
    sigemptyset(&set);
    sigaddset(&set, SIGINT);
    //sigaddset(&set, SIGUSR1);
    pthread_sigmask(SIG_UNBLOCK, &set, NULL);
}
