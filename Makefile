CC=gcc
CFLAGS=-I/usr/include/mysql -g

OPT_JR : list.o main.o utilities.o db.o localSearch.o
	$(CC) -o OPT_JR list.o main.o db.o utilities.o localSearch.o -lm -lmysqlclient

localSearch.o : src/localSearch.c src/localSearch.h 
	$(CC) $(CFLAGS) -c src/localSearch.c

list.o : src/list.c src/list.h 
	$(CC) $(CFLAGS) -c src/list.c

db.o : src/db.c src/db.h 
	$(CC) $(CFLAGS) -c src/db.c

main.o : src/main.c 
	$(CC) $(CFLAGS) -c src/main.c

utilities.o : src/utilities.c src/utilities.h
	$(CC) $(CFLAGS) -c src/utilities.c

clean :
	rm OPT_JR localSearch.o db.o main.o list.o utilities.o
