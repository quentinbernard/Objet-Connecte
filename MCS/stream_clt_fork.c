/*
** Première etape multi client
** Utilisation de fork car pas de mémoire partagée
*/

#include "stream.h"

int connectServer(char*,char*);
void dialogueAvecServ(int sock, buffer_t req);

int main(int c, char**v){
    int sockAppel;

	// verification du nombre  de parametre
    if(c<4){
        printf ("usage : %s <adrIP> <port> \"message\"\n",v[0]);
		exit(-1);
    }
	//	ouverture de la socket
    sockAppel = connectServer(v[1],v[2]);
    dialogueAvecServ(sockAppel, v[3]);

	// fermeture de la socket
    close(sockAppel);
    return 0;
}

/*
** Fonction de dialogue avec le serveur
** On lui envoie un message, puis on récupère sa réponse 
*/

void dialogueAvecServ(int sock, buffer_t req){
    	buffer_t rep;
	int nbCarLus;
	int i, len;

	CHECK(write(sock, req, strlen(req)+1), "Probleme ecriture req");
	printf("message requete (lg=%4d : #%s#\n envoyé", strlen(req), req);

	CHECK(nbCarLus=read(sock, rep, MAX_BUFFER), "Probleme lecture rep");
	printf("message reponse (lg=%4d : #%s#\n", nbCarLus, rep);

	sleep(20);

}

/*
** Fonction de connexion avec le serveur 
**
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
