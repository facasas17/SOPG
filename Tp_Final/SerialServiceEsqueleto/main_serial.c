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

int serial_open(int pn,int baudrate);
void serial_send(char* pData,int size);
void serial_close(void);
int serial_receive(char* buf,int size);


int main (void)
{
	printf("Inicio Serial Service\r\n");
	
	serial_open(1, 115200);

	char buffer[10];

	while(1)
	{
		serial_receive(buffer, sizeof(buffer));

		printf("%s\r\n",buffer);
		serial_send(">OUT:1,0\r\n",sizeof(">OUT:X,Y\r\n"));
		sleep(1);
		serial_send(">OUT:1,1\r\n",sizeof(">OUT:X,Y\r\n"));
		sleep(1);
	}
}
