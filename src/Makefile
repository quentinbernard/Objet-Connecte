all: alarme_v1 alarme_v2 envoi serveur client

alarme_v1.o: alarme_v1.c fredzer.h
	gcc -c alarme_v1.c

alarme_v2.o: alarme_v2.c fredzer.h
	gcc -c alarme_v2.c

envoi.o: envoi.c
	gcc -c envoi.c

alarme_v1: alarme_v1.o
	gcc alarme_v1.o -o alarme_v1 -lwiringPi

alarme_v2: alarme_v2.o
	gcc alarme_v2.o -o alarme_v2 -lwiringPi

envoi: envoi.o
	gcc envoi.o -o envoi

serveur.o: serveur.c stream.h
	gcc -c serveur.c

client.o: client.c stream.h
	gcc -c client.c

serveur: serveur.o
	gcc serveur.o -o serveur -lwiringPi

client: client.o
	gcc client.o -o client

clean:
	rm -rf envoi alarme_v1 alarme_v2 serveur client
