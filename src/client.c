/*
** Gestion multi client
**
*/

#include "stream.h"
#include <wiringPi.h>

//****************Prototypes****************//
int connectServer(char*,char*);
void dialogueAvecServButton1(int sock);
void dialogueAvecServButton2(int sock);
void setup();
void handling(int num);

//*********Varriables globales***************//
//Broche sortie bouton1
const int PIN15 = 3;

//Broche entrée bouton1
const int PIN16 = 4;

//Broche sortie bouton2
const int PIN12 = 1;

//Broche entrée bouton2
const int PIN13 = 2;

//pid processus fils
//mis en var globales pour la fonction de capture des signaux
pid_t pidButton1, pidButton2;

//******************************************//

int main(int c, char**v){
   int sockAppel;
   sigset_t intmask;
   struct sigaction handler;

   if(c < 3){
	printf("usage: <ip_serveur> <port>\n");
	exit(-1);
   }

   //Initialisation de l'environnement GPIO
   wiringPiSetup();
   setup();

   CHECK(pidButton1 = fork(),"Probleme creation fork button1\n");
   if(pidButton1 == 0){
	//Processus fils qui enverra des requêtes 100 (mode sécurité)
	//Quand le bouton 1 sera pressé
   	sockAppel = connectServer(v[1], v[2]);
   	dialogueAvecServButton1(sockAppel);
   	close(sockAppel);

	exit(0);
   }

   CHECK(pidButton2 = fork(),"Probleme creation fork button2\n");
   if(pidButton2 == 0){
	//Processus fils qui enverra des requêtes 200 (LED)
	//Quand le bouton 2 sera pressé
	sockAppel = connectServer(v[1], v[2]);
	dialogueAvecServButton2(sockAppel);
	close(sockAppel);

	exit(0);
   }

   //Préparation capture des signaux
   handler.sa_handler = handling;
   CHECK(sigemptyset(&intmask), "erreur sigemptyset\n");
   handler.sa_flags=SA_RESTART;

   //Capture des signaux
   CHECK(sigaction(SIGINT, &handler, NULL), "capture sigint ratée\n");

   while(1){
   //On ne ferme pas l'application client pour simplifier l'envoi de SIGINT
   //vers les processus fils et fermer tous les processus simplement
   //Ceci sert notamment aux tests
   }

    return 0;
}

/**
 * str2rep
 * Deserialization d'une chaine de caractère
 */
void str2rep(buffer_t b, reponse *rep){
	sscanf(b,"%d##%s", &rep->code, &rep->msg);
}

/**
 * req2str
 * Deserialization d'une requete
 */

void req2str(requete req, buffer_t b){
	memset(b, 0, MAX_BUFFER);
	sprintf(b,"%d##%s", req.code, req.msg);
}

/**
 * printBuffReq
 * printf d'un buffer de requete
 */
void printBuffReq(buffer_t b){
	printf("Requete envoyée: #%s#\n", b);
}

/**
 * printRep
 * printf d'une structure reponse
 */
void printRep(reponse rep){
	printf("Reponse recue: code #%u# message #%s#\n", rep.code, rep.msg);
}

/**
 * dialogueAvecServButton1
 * Fonction du processus fils qui gèrera l'envoi de requêtes 100
 * lorsque le bouton 1 sera pressé
 */
void dialogueAvecServButton1(int sock){
	buffer_t b;
	requete req;
	int nbCarLus;
	reponse rep;

	while(1){

		if(digitalRead(PIN16)){
		   //Lecture du front descendant pour éviter l'envoi
		   //de plusieurs requêtes à la fois
		   if(!digitalRead(PIN16)){
			req.code = 100;
			sprintf(req.msg, "Mode securite");
			req2str(req, b);

			//Envoi requête au serveur
			CHECK(write(sock, b, strlen(b)+1), "Probleme ecriture requete 100\n");
			printBuffReq(b);

			//reception message retour
			memset(b, 0, MAX_BUFFER);
			CHECK(nbCarLus = read(sock, b, MAX_BUFFER), "Probleme lecture reponse 100");
			str2rep(b, &rep);
			printRep(rep);

			//ajout d'un sleep en cas de rebond du bouton
			sleep(1);
		   }
		}

	}
}

/**
 * dialogueAvecServButton2
 * fonction du processus fils qui gère l'envoi de requêtes 200
 * lorsque le bouton 2 est pressé
 */
void dialogueAvecServButton2(int sock){

	buffer_t b;
	requete req;
	reponse rep;
	int nbCarLus;

	while(1){
		if(digitalRead(PIN13)){
		   //Lecture d'un front descendant pour éviter
		   //l'envoi de plusieurs requêtes si l'on reste appuyé longtemps
		   if(!digitalRead(PIN13)){
			req.code = 200;
			sprintf(req.msg, "LED");
			req2str(req, b);

			//Envoi requête 200 au serveur via la socket
			CHECK(write(sock, b, strlen(b)+1),"Probleme envoi requete 200 au serveur\n");
			printBuffReq(b);

			//Reception réponse
			memset(b, 0, MAX_BUFFER);
			CHECK(nbCarLus = read(sock, b, MAX_BUFFER),"Probleme lecture reponse 210\n");
			str2rep(b, &rep);
			printRep(rep);

			//Ajout d'un sleep en cas de rebond du bouton 2
			sleep(1);
		   }
		}
	}

}

/**
 * setup
 * initialise les ports GPIO
 */
void setup(void){

	//Bouton 1
	pinMode(PIN15, OUTPUT);
	pinMode(PIN16, INPUT);
	digitalWrite(PIN15, HIGH);

	//Bouton 2
	pinMode(PIN12, OUTPUT);
	pinMode(PIN13, INPUT);
	digitalWrite(PIN12, HIGH);
}

/**
 * Créer une connexion avec le serveur via une socket
 *
 */
int connectServer(char *hostAddr, char *portNum){
    struct hostent *host;
    buffer_t msgUsage;
    int sock,len;
    struct sockaddr_in serv, myAddr;
    time_t now;

    //Resolution DNS par adresse 
    len = sizeof(myAddr);
    //sprintf(msgUsage,"%s is unknow host : can't resolve\n",hostAddr);
    //CHECKp(host = gethostbyaddr(hostAddr, len , AF_INET),msgUsage);
    // ou resolution DNS par nom
    //CHECKp(host = gethostbyname(hostName),msgUsage);

    //Creation socket
    CHECK(sock=socket(AF_INET,SOCK_STREAM,0),"Can't create socket");

    //Preparation serveur
    serv.sin_family = AF_INET;
    serv.sin_port = htons(atoi(portNum));
    //bcopy(host->h_addr,(char *) &serv.sin_addr, host->h_length);
    serv.sin_addr.s_addr = inet_addr(hostAddr);
    bzero(serv.sin_zero,8); //ie : memset(serv.sin_zero,8,0)

    //Connexion au serveur 
    len = sizeof(serv);
    CHECK(connect(sock, (struct sockaddr *)&serv,len),"Can't connect");

    //Quel est mon adressage ?
    len = sizeof(myAddr);
    CHECK(getsockname(sock, (struct sockaddr *)&myAddr,(socklen_t *)&len),"Can't get socket name");
    CHECK(time(&now),"Get time\t[FAILED]");
    printf("client[%i]: %s\n\tUsed address by the client is %s\n\tAllocated port for the client is %i\n",getpid(),ctime(&now),inet_ntoa(myAddr.sin_addr),ntohs(myAddr.sin_port));

    return sock;
}

/**
 * handling
 * fonction de traitement des signaux capturés
 */
void handling(int num){

	printf("Reception signal %d\n",num);

	switch(num){
	case SIGINT:
		printf("SIGINT reçu, fin des processus...\n");

		kill(pidButton1, SIGINT);
		kill(pidButton2, SIGINT);
		exit(0);
	break;
	}
}
