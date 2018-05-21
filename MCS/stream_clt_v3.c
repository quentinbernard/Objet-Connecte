/*
** Gestion multi client
**
*/

#include "stream.h"

int connectServer(char*,char*);
void dialogueAvecServ(int sock);

int main(int c, char**v){
    int sockAppel;
    if(c<4){
        printf ("usage : %s <adrIP> <port> \"message\"\n",v[0]);
		exit(-1);
    }
    sockAppel = connectServer(v[1],v[2]);
    dialogueAvecServ(sockAppel);
    close(sockAppel);
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

void dialogueAvecServ(int sock){
    buffer_t b;
	requete req;
	int nbCarLus;
	int i, len;
reponse rep;
	
	while(1){
		printf("Code requete : "); scanf("%ui",&req.code);
		if(req.code != 0){
			printf("Msg requete : "); scanf("%s", &req.msg);
		}
		else sprintf(req.msg, "fin");
		req2str(req, b);		
		CHECK(write(sock, b, strlen(b)+1), "Probleme ecriture req");
		printf("message requete (lg=%4d : #%s#\n envoyé", strlen(b), b);
		if(req.code == 0) break;
		
		memset(b, 0, MAX_BUFFER);
		CHECK(nbCarLus=read(sock, b, MAX_BUFFER), "Probleme lecture rep");
		str2rep(b, &rep);
		printf("message reponse (lg=%4d : #%s#\n", strlen(b), b);
		printf("\t code = #%ui#, msg = #%s# \n", rep.code, rep.msg);
	}

}

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
