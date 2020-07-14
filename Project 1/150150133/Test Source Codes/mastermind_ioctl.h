#ifndef __MMIND_H
#define __MMIND_H

#include <linux/ioctl.h> /* needed for the _IOW etc stuff used later */

//Command Number is the number that is assigned to the ioctl. It is used to differentiate the commands from one another.

#define MMIND_IOC_MAGIC 'f' //The Magic Number is a unique number or character that will differentiate our set of ioctl calls from the other ioctl calls.
#define MMIND_REMAINING _IOR(MMIND_IOC_MAGIC, 0, int *) //IOR copy_to_user
#define MMIND_ENDGAME _IO(MMIND_IOC_MAGIC,  1) //IO with no parameter
#define MMIND_NEWGAME _IOW(MMIND_IOC_MAGIC,  2, char *) //IOW copy_from_user

#define MMIND_IOC_MAXNR 2

#endif
