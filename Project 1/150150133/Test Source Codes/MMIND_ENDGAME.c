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
	status = ioctl(fd, MMIND_ENDGAME);

	if(status < 0)
		printf("Error: %s\n \tGame cannot end! \n", strerror(errno));
	else
		printf("Game is ended succesfully!! \n");
	
	return status;
}
