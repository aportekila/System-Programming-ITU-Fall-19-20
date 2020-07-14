#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>


#include "mastermind_ioctl.h"

int main(int argc, char *argv[]){
	
	int status;
	int fd = open("/dev/mastermind", 'r');
	int* remaining;
	remaining = malloc(sizeof(int));
	status = ioctl(fd, MMIND_REMAINING, remaining);

	if(status < 0){
		printf("Error: %s\n\tRemaining number of guesses cannot get\
			\n", strerror(errno));
	}	
	else{
		printf("Remaining number of guesses = %d\n", *remaining);
	}
	
	free(remaining);
	return status;
}
