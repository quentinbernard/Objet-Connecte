/*
** Gestion multi client
**
*/
#include "stream.h"

//************Prototypes******************//
int createSocketEcoute(char*,int);
void dialogueAvecClt(int sockDial);
int acceptConnect(int);
int setupAlarme(void);
void setup(void);
void traitement_signal(int num);
void signaux_alarme(int num);
void processAlarme(void);

//******variables globales****************//
//PID alarme
int pidAlarme;

//Broche LED sortie -> 15 = 3
const int PIN15 = 3;

//Broche bouton "détecteur de mouvement" sortie -> 7 = 7
const int PIN7 = 7;

//Broche bouton "détecteur de mouvement" entrée -> 11 = 0
const int PIN11 = 0;

//Broche alarme sortie -> 13 = 2
const int PIN13 = 2;

//****************************************//

int main(){

    int sockEcoute, sockDialogue, retour;
    sigset_t intmask;
    struct sigaction handler;
    pid_t pidClt;

    //creation socket ecoute, association adressage et mise en ecoute
    sockEcoute = createSocketEcoute(IP_SVC,PORT_SVC);

    //initialisation des différentes variables (ports GPIO...)
    wiringPiSetup();
    setup();

    //Initialise processus fils -> gestion mode sécurité (alarme)
    CHECK(pidAlarme = fork(), "Probleme fork alarme\n");
    if(pidAlarme == 0){
	setupAlarme();
	processAlarme();

	exit(0);
    }

    //Préparation capture des signaux du processus père
    handler.sa_handler = traitement_signal;
    CHECK(sigemptyset(&intmask), "Problème appel sigemptyset (processus père)\n");
    handler.sa_flags=SA_RESTART;

    //Capture des signaux
    CHECK(sigaction(SIGINT, &handler, NULL), "Capture de SIGINT ratée\n");

    //Boucle de service
    while(1){
       //Acceptation d'un appel
        sockDialogue = acceptConnect(sockEcoute);
	// creation processus de service pour le client appelant
	CHECK(pidClt = fork(),"Probleme fork client\n");
	if(pidClt == 0){
		//fermeture socket ecoute
		close(sockEcoute);
		//Dialogue avec un client
        	dialogueAvecClt(sockDialogue);
        	//Fermeture socket dialogue
        	close(sockDialogue);

		exit(0);
	}

	//Fermeture socket dialogue
        close(sockDialogue);

    }
    //fermeture socket ecoute
    close(sockEcoute);

    return 0;
}

/**
 *
 * Créé une socket d'ecoute sur l'ip et le port donnée en paramêtre
 *
 */

int createSocketEcoute(char *ipSvc, int portSvc){

    struct sockaddr_in server;
    int sock,len;
    time_t now;
    //Creation de la socket
    CHECK(sock = socket(AF_INET, SOCK_STREAM,0),"Create socket\t[FAILED]");
    //preparation adressage serveur
    server.sin_family = AF_INET;
    server.sin_port = htons(portSvc);
    server.sin_addr.s_addr = inet_addr(ipSvc);
    bzero(server.sin_zero,8);
    //Association de l'adressage avec la socket
    CHECK(bind(sock, (struct sockaddr *)&server, sizeof server),"Bind\t\t[FAILED]");

    //Recuperation des infos de la socket 
    len = sizeof(server);
    CHECK(getsockname(sock,(struct sockaddr *)&server, (socklen_t *)&len),"Get socket name\t[FAILED]");
    printf("SRV[%i] : %s\tService is started at adress %s:%4i in socket %i\n",getpid(),ctime(&now),inet_ntoa(server.sin_addr),htons(server.sin_port),sock);

    CHECK(listen(sock, MAX_CLTS),"Listen\t\t[FAILED]");
    return sock;
}


/**
 * str2req
 * Deserialization d'une chaine de caractère
 */
void str2req(buffer_t b, requete *req){
	sscanf(b,"%d##%s", &req->code, &req->msg);
}

/**
 * rep2str
 * Deserialization d'une reponse
 */
void rep2str(reponse *rep, buffer_t b){
	memset(b, 0, MAX_BUFFER);
	sprintf(b,"%d##%s", rep->code, rep->msg);
}


/**
 * lireRequete
 * deserialise une requete et affiche son code ainsi que son message
 *
 */
void lireRequete(int sockDial, requete *req){
	int nbCarLus;
	buffer_t b;

	memset(b, 0, MAX_BUFFER);
	CHECK(nbCarLus=read(sockDial, b, MAX_BUFFER), "Probleme lecture requete");
	str2req(b, req);
	printf("Requete reçue: code = #%u#, msg = #%s# \n", req->code, req->msg);

}


/*
** Serialize la reponse et envoie la reponse via la socket de dialogue
*/
void ecrireReponse(int sockDial,reponse *rep){

	buffer_t b;

	memset(b, 0, MAX_BUFFER);
	rep2str(rep, b);

	CHECK(write(sockDial, b, strlen(b)+1), "Probleme ecriture reponse");
	printf("Reponse envoyée code = #%u#, msg = #%s# \n", rep->code, rep->msg);

}

/**
 * traite les requete avec le code 100
 * Activation/désactivation alarme
 */
void traiterRequete100(requete req, reponse *rep, int* modeSecurite){

	rep->code = req.code + 10;

	if(*modeSecurite==1){
		kill(pidAlarme, SIGUSR2);
		*modeSecurite = 0;
		sprintf(rep->msg,"Desactivation");
	}else{
		kill(pidAlarme, SIGUSR1);
		*modeSecurite = 1;
		sprintf(rep->msg,"Activation");
	}
}


/**
 * traite les requete avec le code 200
 * Activation/désactivation LED
 */
void traiterRequete200(requete req, reponse *rep, int* ledOn){

	rep->code = req.code + 10;

	if(*ledOn==1){
		digitalWrite(PIN15,LOW);
		*ledOn = 0;
		sprintf(rep->msg,"OFF");
	}else{
		digitalWrite(PIN15,HIGH);
		*ledOn = 1;
		sprintf(rep->msg,"ON");
	}
}


/**
 * traite les requetes 100 ou 200
 */
void traiterRequete(requete req, reponse *rep, int* modeSecurite, int* ledOn){

	if(req.code == 200){
		traiterRequete200(req, rep, ledOn);
		return;
	}else if (req.code == 100){
		traiterRequete100(req, rep, modeSecurite);
		return;
	}
}

/**
 * setupAlarme
 * initialise les variables liées au programme d'alarme
*/
int setupAlarme(void){

	sigset_t intmask_alarme;
	struct sigaction handler_alarme;

	printf("Setup alarme\n");

	//Préparation capture des signaux
	handler_alarme.sa_handler = signaux_alarme;
	CHECK(sigemptyset(&intmask_alarme), "Problème appel sigemptyset alarme\n");
	handler_alarme.sa_flags=SA_RESTART;

	//capture des signaux
	CHECK(sigaction(SIGUSR1, &handler_alarme, NULL), "Capture SIGUSR1 ratée\n");
	CHECK(sigaction(SIGUSR2, &handler_alarme, NULL), "Capture SIGUSR2 ratée\n");

	//Préparation broches
	//Bouton "détecteur de mouvement"
	pinMode(PIN7, OUTPUT);
	pinMode(PIN11, INPUT);

	//Alarme
	pinMode(PIN13, OUTPUT);

	//Etre sûr sorties alarme + bouton à 0
	digitalWrite(PIN13, LOW);
	digitalWrite(PIN7, LOW);

	return 0;
}

/**
 * processAlarme
 * Processus du mode de sécurité (alarme)
 */
void processAlarme(void){

	while(1){
		if(digitalRead(PIN11>0)){
			digitalWrite(PIN13, HIGH);
		}
	}
}

/**
 * setup
 * initialise les ports GPIO pour la LED
*/
void setup(void){

	pinMode(PIN15, OUTPUT);
	//Make sure it is already at LOW
	digitalWrite(PIN15, LOW);
}



/**
 * dialogueAvecClient
 * Reçois la requete, la traite et envoie la reponse au client
 *
 */
void dialogueAvecClt(int sockDial){
	reponse rep;
	requete req;
	int  modeSecurite;
	int  ledOn;

	modeSecurite = 0;
	ledOn = 0;

	while(1){
		lireRequete(sockDial, &req);
		traiterRequete(req, &rep, &modeSecurite, &ledOn);
		ecrireReponse(sockDial, &rep);
	}

}

/*
** Fonction de connexion avec le client 
** Crée une socket de dialogue
*/

int acceptConnect(int sockEcoute){
    int sock,len;
    struct sockaddr_in clt,server;
    time_t now;
    len = sizeof(clt);
    CHECK(sock=accept(sockEcoute,(struct sockaddr *)&clt, (socklen_t *)&len),"Accept\t\t[FAILED]");
    len = sizeof(server);
    CHECK(getsockname(sock,(struct sockaddr *)&server, (socklen_t *)&len),"Get socket name\t[FAILED]");
    printf("SRV[%i] : %s\tClient %s:%4i is connected with server %s:%4i using socket %i\n",getpid(),ctime(&now),inet_ntoa(clt.sin_addr),htons(clt.sin_port),inet_ntoa(server.sin_addr),htons(server.sin_port),sock);

    return sock;
}

/**
 * signaux_alarme
 * Fonction de traitement des signaux capturés pour l'alarme
 */
void signaux_alarme(int num){

	printf("[signaux_alarme] Réception signal %d\n", num);

	switch(num){
	case SIGUSR1:
		printf("[signaux_alarme] SIGUSR1 reçu, mode de sécurité activé\n");
		digitalWrite(PIN7, HIGH);
	break;
	case SIGUSR2:
		printf("[signaux_alarme] SIGUSR2 reçu, mode de sécurité désactivé\n");
		digitalWrite(PIN7, LOW);
		//On ne coupe l'alarme que sur le signal de
		//désactivation du mode de sécurité
		digitalWrite(PIN13, LOW);
	break;
	}
}

/**
 * traitement_signal
 * Fonction de traitement des signaux capturés pas le processus père
 */
void traitement_signal(int num){

	printf("[traitement_signal] Réception signal %d\n", num);

	switch(num){
	case SIGINT:
		printf("[traitement_signal] SIGINT reçu, fin des processus...\n]");
		kill(pidAlarme, SIGINT);
		exit(0);
	break;
	}
}

