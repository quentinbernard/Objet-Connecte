CC = gcc

all: serveur client

serveur.o: serveur.c stream.h
	${CC} -c serveur.c

client.o: client.c stream.h
	${CC} -c client.c

serveur: serveur.o
	${CC} serveur.o -o serveur -lwiringPi

client: client.o
	${CC} client.o -o client -lwiringPi

clean:
	rm -rf serveur client

cleanAll:
	rm -rf serveur.o serveur client.o client
