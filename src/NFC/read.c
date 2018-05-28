#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wiringPi.h>
#include <sys/types.h>
#include <signal.h>

#define CHECK(sts, msg) if((sts) == -1) { perror(msg); exit(-1); }

#define CHECK_IF(stat, val, msg) if((stat) == (val)){ perror(msg);exit(EXIT_FAILURE);}

#define MAX_LEN 40


//Correspondance broche wiringPi
//Broche 3 -> 8
//Broche 5 -> 9
//Broche 7 -> 7
//Broche 8 -> 15
//Broche 10 -> 16
//Broche 11 -> 0
//Broche 12 -> 1
//Broche 13 -> 2
//Broche 15 -> 3
//Broche 16 -> 4
//Broche 18 -> 5
//Broche 19 -> 12
//Broche 21 -> 13
//Broche 22 -> 6
//Broche 23 -> 14
//Broche 24 -> 10
//Broche 26 -> 11

// Sortie sur broche 
const int PIN3 = 8;
const int PIN13 = 2;
// Entrée sur broche
const int PIN12 = 1;

//Fonction de traitement des signaux capturés
void traitement_signal(int num){

	char buffer[MAX_LEN]="";
	int reading=1;
	
	char uid[20];

	printf("Reception signal %d\n",num);

	switch(num){
	case SIGUSR1:
	
		sleep(1);
	
		printf("SIGUSR1 reçu\n");

		//on recupere l'uid du badge
		FILE *file=fopen("UID.txt", "r");
		fscanf(file, "%s", uid);
		
		fclose(file);

		// on vérifie qu'il s'agit du bon badge 
		if(strcmp(uid, "190.193.37.217") == 0){

			//ecriture sur le port gpio pour allumer la LED
			digitalWrite(PIN12, HIGH);

			// on attend la led allumée
			sleep(2);

			// on éteint la LED
			digitalWrite(PIN12, LOW);
		}

	break;
	}

}

void waitSign(){

	sigset_t intmask;
	struct sigaction handler;

	//Préparation de capture des signaux
	handler.sa_handler = traitement_signal;
	CHECK(sigemptyset(&intmask), "appel de sigemptyset");
	handler.sa_flags=SA_RESTART;

	printf("Setup test\n");
	pinMode(PIN12, OUTPUT);

	//Capture des signaux
	CHECK(sigaction(SIGUSR1, &handler, NULL), "appel de sigusr1 raté");
	
	pause();
	
}

void main(){

pid_t pid;
FILE * fp;

//Sauvegarde du pid dans le fichier "read.pid"
pid = getpid();
fp = fopen("read.pid","w");
fprintf(fp,"%d\n",pid);
fclose(fp);

// Setup de wiringPi
wiringPiSetup();

pid_t pidf;

// On utilise un fork pour que la lecture avec le programme en python tourne en continu
CHECK(pidf=fork(),"fork - Problème de création de processus !");

if (pidf==0){
	// appel à Read.py
	system("python Read.py");
}

for(;;){

	// attente d'un signal
	waitSign();
	

}


}
