/*
** Gestion multi client
**
*/
#include <wiringPi.h>
#include "stream.h"

int createSocketEcoute(char*,int);
void dialogueAvecClt(int sockDial);
int acceptConnect(int);
int setupAlarme(void);
void setup(void);

//PID alarme
int pidAlarme;

//Broche LED sortie -> 15 = 3
const int PIN15 = 3;

int main(){

    int sockEcoute, sockDialogue, retour;

    pid_t pidClt;
    //creation socket ecoute, association adressage et mise en ecoute
    sockEcoute = createSocketEcoute(IP_SVC,PORT_SVC);

    //initialisation des différentes variables (pid alarme, ports GPIO...)
    CHECK(retour = setupAlarme(),"Probleme setup alarme");
    wiringPiSetup();
    setup();

    //Boucle de service
    while(1){
       //Acceptation d'un appel
        sockDialogue = acceptConnect(sockEcoute);
	// creation processus de service pour le client appelant
	CHECK(pidClt = fork(),"Probleme creation fork");
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

	FILE * fp=NULL;
	//récupération pid alarme
	fp = fopen("alarme.pid","r");
	if(fp==NULL){
		printf("Erreur, fichier alarme.pid manquant\n");
		return -1;
	}

	fscanf(fp,"%d\n",&pidAlarme);
	printf("PID de l'alarme: %d\n",pidAlarme);
	return 0;
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
		if(req.code ==0) break;
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

