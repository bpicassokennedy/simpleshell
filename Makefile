simpleshell: simpleshell.o
	gcc -Wall simpleshell.o -o simpleshell
simpleshell.o: simpleshell.c
	gcc -Wall -c simpleshell.c
clean:
	rm -f *.o simpleshell