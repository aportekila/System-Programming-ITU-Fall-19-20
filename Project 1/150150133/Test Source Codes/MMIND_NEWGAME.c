#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>


#include "mastermind_ioctl.h"

int main(int argc, char *argv[]){
	
	int status;
	
	
	int fd = open("/dev/mastermind", 'w');
	
	if (argc != 2){
		printf("Invalid number of parameter is given\n\
			\t the format is: ./MMIND_NEWGAME <number>\n");
		return -1;
	}
	else{
		status = ioctl(fd, MMIND_NEWGAME, argv[1]);

		if(status < 0)
			printf("Error: %s\n\tCannot start a new game!\n", strerror(errno));
		else
			printf("succesfully started to new game\n");
		
		return status;
	}
	
	
}
