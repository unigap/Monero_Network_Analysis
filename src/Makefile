all: main

main: main.c bst.c request1001.c check1003.c location.c recv2002.c
	gcc main.c bst.c request1001.c check1003.c location.c recv2002.c -lpthread -o main

clean:
	rm -f main [0-9]* *.o              # don't remove log files

remove_o:
	rm -f *.o		           # remove only .o files

move_logs:
	mkdir -p flogs && mv log* ./flogs  # move log files

remove_logs:
	rm -f log*                         # remove log files

# or
#main: main.o bst.o request1001.o check1003.o location.o recv2002.o
#	gcc main.o bst.o request1001.o check1003.o location.o recv2002.o -lpthread -o main
#main.o: main.c
#	gcc -c main.c
#bst.o: bst.c
#	gcc -c bst.c
#request1001.o: request1001.c
#	gcc -c request1001.c
#check1003.o: check1003.c
#	gcc -c check1003.c
#location.o: location.c
#	gcc -c location.c
#recv2002.o: recv2002.c
#	gcc -c recv2002.c

# or
#%.o: %.c
#	gcc -c %.c
#
