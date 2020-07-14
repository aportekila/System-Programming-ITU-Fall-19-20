Group 20:
	Bengisu Güresti	150150105
	Ufuk Demir		150170710
	Abdullah Akgül	150150133

In order to compile the source code;
	gcc fuse.c -o fuse -Wall -ansi -W -std=c99 -g -ggdb -D_GNU_SCORE -D_FILE_OFFSET_BITS=64 -lfuse -lmagic -lgd -Wextra -pedantic -lansilove -D_BSD_SOURCE

Then you need to create a src and an empty dst folders.

In order to run the file system;
	./fuse -d src dst

