ej: p0.o p1.o p2.o p3.o list.o
	gcc -g p0.o list.o p1.o p2.o p3.o -lreadline -Wall -o ej
	
p0.o: p0.c
	gcc -g -c p0.c
	
p1.o: p1.c p1.h
	gcc -g -c p1.c
	
p2.o: p2.c p2.h
	gcc -g -c p2.c
	
p3.o: p3.c p3.h
    gcc -g -c p3.c
	
list.o: list.c list.h
	gcc -g -c list.c
	
clear:
	rm *.o ej

valgrind: 
	valgrind ./ej

