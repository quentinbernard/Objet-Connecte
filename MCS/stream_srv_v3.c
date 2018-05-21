/*
** Gestion multi client
**
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
	CHECK(nbCarLus=read(sockDial, b, MAX_BUFFER), "Probleme lecture req");
	str2req(b, req);
	printf("message requete (lg=%4d : #%s#\n envoyé", strlen(b), b);
	printf("\t code = #%ui#, msg = #%s# \n", req->code, req->msg);

}


/*
** Serialize la reponse et envoie la reponse via la socket de dialogue
*/
void ecrireRequete(int sockDial,reponse *rep){

	buffer_t b;

	memset(b, 0, MAX_BUFFER);
	rep2str(rep, b);

	CHECK(write(sockDial, b, strlen(b)+1), "Probleme ecriture rep");
	printf("message reponse (lg=%4d : #%s#\n", strlen(b), b);
	printf("\t code = #%ui#, msg = #%s# \n", rep->code, rep->msg);

}

/**
 * traite les requete avec le code 100
 */
void traiterRequete100(requete req, reponse *rep){
	sprintf(rep->msg, "Reponse a la requette %u", req.code);
	rep->code = req.code + 100;
}


/**
 * traite les requete avec le code 200
 */
void traiterRequete200(requete req, reponse *rep){

	int i, len;

	len=strlen(req.msg)-1;
	for(i=len;i>=0; i--){
		rep->msg[len-i] = req.msg[i];
	}

	rep->msg[len+1] = '\0';
	rep->code = req.code + len;

}


/**
 * traite les requete autre que 100 ou 200
 */
void traiterRequete(requete req, reponse *rep){

	int i, len;

	if(req.code == 200){
		traiterRequete200(req, rep);
		return;
	}

	for(i=0;i<sizeof(REQ2REP)/sizeof(req2rep); i++){
		if(req.code == REQ2REP[i].codeReq){
			strcpy(rep->msg, REQ2REP[i].msgRep);
			//REQ2REP[i].stmt(req,rep);
		}
		
	}
	

}


/**
 * dialogueAvecClient
 * Reçois la requete, la traite et envoie la reponse au client
 * 
 */
void dialogueAvecClt(int sockDial){
	reponse rep;
	requete req;
	int nbCarLus;
	int i, len;

	while(1){
		lireRequete(sockDial, &req);
		if(req.code ==0) break;
		traiterRequete(req, &rep);
		ecrireRequete(sockDial, &rep);
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

