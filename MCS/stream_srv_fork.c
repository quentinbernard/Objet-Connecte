/*
** Première etape multi client
** Utilisation de fork car pas de mémoire partagée
*/

#include "stream.h"

int createSocketEcoute(char*,int);
void dialogueAvecClt(int sockDial);
int acceptConnect(int);
int main(){
    int sockEcoute, sockDialogue;

	pid_t pidClt;
    //creation socket ecoute, association adressage et mise en ecoute
    sockEcoute = createSocketEcoute(IP_SVC,PORT_SVC);
    //Boucle de service
    while(1){
       //Acceptation d'un appel 
        sockDialogue = acceptConnect(sockEcoute);
	// creation thread de service pour le client appelant
		CHECK(pidClt = fork(),"Probleme creation thread");
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
 * createSocketEcoute
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

/*
** Fonction de dialogue avec le client
** On recupère son message
** Puis on écrit la reponse au client
*/

void dialogueAvecClt(int sockDial){
	buffer_t req, rep;
	int nbCarLus;
	int i, len;

	CHECK(nbCarLus=read(sockDial, req, MAX_BUFFER), "Probleme lecture req");
	printf("message de la requete recue (lg=%4d : #%s#\n", nbCarLus, req);

	len=strlen(req)-1;
	for(i=len;i>=0; i--){
		rep[len-i] = req[i];
	}
	rep[len+1] = '\0';

	CHECK(write(sockDial, rep, strlen(rep)+1), "Probleme ecriture rep");
	printf("message reponse (lg=%4d : #%s#\n", strlen(rep), rep);

}

/*
** Fonction de connexion avec le client 
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

