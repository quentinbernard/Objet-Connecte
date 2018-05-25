#include <stdio.h>
#include <wiringPi.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include "fredzer.h"

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
const int PIN7 = 7; //bouton "détecteur de mouvement"
const int PIN13 = 2; //alarme

// Entrée sur broche
const int PIN12 = 1;

//Fonction de traitement des signaux capturés
void traitement_signal(int num){

	printf("Reception signal %d\n",num);

	switch(num){
	case SIGUSR1:
		printf("SIGUSR1 reçu, mode de sécurité activé\n");
		digitalWrite(PIN7, HIGH);
	break;
	case SIGUSR2:
		printf("SIGUSR2 reçu, mode de sécurité désactivé\n");
		digitalWrite(PIN7, LOW);
	break;
	}

}

//Initialisation
void setup(){

	pid_t pid;
	FILE * fp;
	sigset_t intmask;
	struct sigaction handler;

	//Préparation de capture des signaux
	handler.sa_handler = traitement_signal;
	CHECK(sigemptyset(&intmask), "appel de sigemptyset");
	handler.sa_flags=SA_RESTART;

	printf("Setup test\n");
	pinMode(PIN7, OUTPUT);
	pinMode(PIN13, OUTPUT);

	//Etre sur avoir sortie alarme à nulle
	digitalWrite(PIN13,LOW);
	digitalWrite(PIN7, LOW);//Idem pour le bouton

	pinMode(PIN12, INPUT);
	//test V1
	//digitalWrite(PIN7, HIGH);

	//Sauvegarde du pid dans le fichier "alarme.pid"
	pid = getpid();
	printf("PID alarme: %d\n", pid);
	fp = fopen("alarme.pid","w");
	fprintf(fp,"%d\n",pid);
	fclose(fp);

	//Capture des signaux
	CHECK(sigaction(SIGUSR1, &handler, NULL), "appel de sigusr1 raté");
	CHECK(sigaction(SIGUSR2, &handler, NULL), "appel de sigusr2 raté");
}


//Fonction de boucle
void loop(){

	if(digitalRead(PIN12)>0){
		digitalWrite(PIN13, HIGH);
	}else{
		digitalWrite(PIN13, LOW);
	}
}

int main (void){

	wiringPiSetup(); //Always declared
	setup();

for(;;)
	loop();

return 0;

}
 
