#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>


int main()
{
	socklen_t addr_len;
	struct sockaddr_in clientaddr;
	struct sockaddr_in serveraddr;
	char buffer[128];
	int newfd;
	int n;
	_Bool flag_conn_lost = 1;

	// Creamos socket
	int s = socket(AF_INET,SOCK_STREAM, 0);

	// Cargamos datos de IP:PORT del server
    	bzero((char*) &serveraddr, sizeof(serveraddr));
    	serveraddr.sin_family = AF_INET;
    	serveraddr.sin_port = htons(10000);
    	if(inet_pton(AF_INET, "127.0.0.1", &(serveraddr.sin_addr))<=0)
    	{
        	fprintf(stderr,"ERROR invalid server IP\r\n");
        	return 1;
    	}

	// Abrimos puerto con bind()
	if (bind(s, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) == -1) {
		close(s);
		perror("listener: bind");
		return 1;
	}

	// Seteamos socket en modo Listening
	if (listen (s, 10) == -1) // backlog=10
  	{
    	perror("error en listen");
    	exit(1);
  	}

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
		
		inet_ntop(AF_INET, &(clientaddr.sin_addr), ipClient, sizeof(ipClient));
		printf  ("server:  conexion desde:  %s\n",ipClient);
        flag_conn_lost = 1;

		while( flag_conn_lost )
		{
            // Funcion de lectura bloqueante
            n = read(newfd,buffer,128);

            switch ( n )
            {
            case -1:
                perror("Error leyendo mensaje en socket");
				exit(1);
                break;

            case 0:
                flag_conn_lost = 0;
                break;

            default:
            	buffer[n]=0x00;
			    printf("Recibi %d bytes.:%s\n",n,buffer);

			    // Enviamos mensaje a cliente
    		    if (write (newfd, ">SW:1,1\r\n", strlen(">SW:1,1\r\n")) == -1)
    		    {
      			    perror("Error escribiendo mensaje en socket");
      			    exit (1);
    		    }
                // Necesita un delay porque sino satura el programa de python
			    sleep(2);
                break;
            }
		}
		close(newfd);
	}

}

