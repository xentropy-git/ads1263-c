default: program

program: 
	gcc -Wall -o bin/main src/main.c -llgpio
