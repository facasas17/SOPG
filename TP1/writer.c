#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdint.h>
#include <signal.h>

#define FIFO_NAME "fifoTP1"
#define BUFFER_SIZE 300
#define FORMAT_DATA "DATA:"
#define FORMAT_SIGUSR1 "SIGN:1"
#define FORMAT_SIGUSR2 "SIGN:2"

volatile sig_atomic_t got_usr1;
volatile sig_atomic_t got_usr2;

void createNamedFIFO( int32_t *fd )
{
	int32_t returnCode;

    /* Create named fifo. -1 means already exists so no action if already exists */
    if ( (returnCode = mknod(FIFO_NAME, S_IFIFO | 0666, 0) ) < -1 )
    {
        printf("Error creating named fifo: %d\n", returnCode);
        exit(1);
    }

    /* Open named fifo. Blocks until other process opens it */
	printf("waiting for readers...\n");
	if ( (*fd = open(FIFO_NAME, O_WRONLY) ) < 0 )
    {
        printf("Error opening named fifo file: %d\n", *fd);
        exit(1);
    }
    
    /* open syscalls returned without error -> other process attached to named fifo */
	printf("got a reader--type some stuff\n");
}


void recibiSigUSR1( int sig )
{
	got_usr1 = 1;
}

void recibiSigUSR2( int sig )
{
	got_usr2 = 1;
}

void createSig( void )
{
	struct sigaction sa_usr1;
	struct sigaction sa_usr2;

	sa_usr1.sa_handler = recibiSigUSR1;
	sa_usr1.sa_flags = 0; // SA_RESTART;
	sigemptyset(&sa_usr1.sa_mask);

	sa_usr2.sa_handler = recibiSigUSR2;
	sa_usr2.sa_flags = 0; // SA_RESTART;
	sigemptyset(&sa_usr2.sa_mask);

	if (sigaction(SIGUSR1,&sa_usr1,NULL) == -1) 
	{
		perror("sigaction");
		exit(1);
	}
	if (sigaction(SIGUSR2,&sa_usr2,NULL) == -1) 
	{
		perror("sigaction");
		exit(1);
	}
}

int main(void)
{
	char outputBuffer[BUFFER_SIZE];
	char fifoBuffer[BUFFER_SIZE];
	uint32_t bytesWrote;
	int32_t fd;

	/* Create named FIFO */
	createNamedFIFO( &fd );

	/* Define handler of SIGUSR */
	createSig( );

    /* Loop forever */
	while (1)
	{
		strcpy(fifoBuffer, FORMAT_DATA);

        /* Get some text from console */
		fgets(outputBuffer, BUFFER_SIZE, stdin);

		strcat(fifoBuffer, outputBuffer);	
        
        /* Write buffer to named fifo. Strlen - 1 to avoid sending \n char */
		if ( !got_usr1 && !got_usr2)	// Enter if no SIGUSR was received
		{
			if ((bytesWrote = write(fd, fifoBuffer, strlen(fifoBuffer)-1)) == -1)
        		{
				perror("write");
        		}		
		}
		else if ( got_usr1 )
			{
				write(fd, FORMAT_SIGUSR1, strlen(FORMAT_SIGUSR1));
				got_usr1 = 0;
			}
			else if ( got_usr2 )
			{
				write(fd, FORMAT_SIGUSR2, strlen(FORMAT_SIGUSR2));
				got_usr2 = 0;
			}
	}
	return 0;
}
