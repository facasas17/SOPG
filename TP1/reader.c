#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdint.h>

#define FIFO_NAME "fifoTP1"
#define BUFFER_SIZE 300
#define FORMAT_DATA "DATA"
#define FORMAT_SIGUSR "SIGN"

void namedFIFO(int32_t *fd)
{
	int32_t returnCode;
    /* Create named fifo. -1 means already exists so no action if already exists */
    if ( (returnCode = mknod(FIFO_NAME, S_IFIFO | 0666, 0) ) < -1  )
    {
        printf("Error creating named fifo: %d\n", returnCode);
        exit(1);
    }
    
    /* Open named fifo. Blocks until other process opens it */
	printf("waiting for writers...\n");
	if ( (*fd = open(FIFO_NAME, O_RDONLY) ) < 0 )
    {
        printf("Error opening named fifo file: %d\n", *fd);
        exit(1);
    }
    /* open syscalls returned without error -> other process attached to named fifo */
	printf("got a writer\n");
}

 
void createFile(FILE **fptr, char *file_name)
{
     /* Open file in a (append) mode. */
	*fptr = fopen(file_name, "a");

	if(*fptr == NULL)
    {
        /* File not created hence exit */
        printf("Unable to create file.\n");
        exit(1);
    }
}

int main(void)
{
	uint8_t inputBuffer[BUFFER_SIZE];
	int32_t bytesRead, returnCode, fd;

    /* File pointer to hold reference to our files */
	FILE * fptr_data;
	FILE * fptr_sig;

	/* Create named FIFO */
	namedFIFO(&fd);

	/* Create and open file .txt */
	createFile(&fptr_data, "log.txt");
	createFile(&fptr_sig, "signal.txt");

    /* Loop until read syscall returns a value <= 0 */
	do
	{
        /* read data into local buffer */
		if ((bytesRead = read(fd, inputBuffer, BUFFER_SIZE)) == -1)
    	{
			perror("read");
       	}
        else
		{
			inputBuffer[bytesRead] = '\n';
			inputBuffer[bytesRead+1] = '\0';
		}
		
		/* Compares first 4 bytes of received string */
		if ( !strncmp(inputBuffer, FORMAT_DATA, 4) )
		{
			/* Write data to file */
    		fputs(inputBuffer, fptr_data);
		}
		else if ( !strncmp(inputBuffer, FORMAT_SIGUSR, 4) )
		{
			/* Write data to file */
    		fputs(inputBuffer, fptr_sig);
		}
	} while (bytesRead > 0);
	
	/* Close files */
	fclose(fptr_data);
	fclose(fptr_sig);
	
	/* Close named FIFO */
	close(fd);

	return 0;
}
