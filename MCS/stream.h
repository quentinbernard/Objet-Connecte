
#ifndef stream_h
#define stream_h

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <errno.h>
#include <time.h>
#include <netdb.h>

#define CHECK(sts,msg) if ((sts) == -1) {perror(msg); exit(-1); }
#define CHECKp(sts,msg) if ((sts) == NULL) {perror(msg); exit(-1); }

#define MAX_BUFFER 1024


#define IP_SVC "0.0.0.0"
#define PORT_SVC 5000

#define MAX_CLTS 5

typedef char buffer_t[MAX_BUFFER];


/*
typedef struct { 
    unsigned int codeReq; 
    msg_t msgRep;
} req2rep;

const req2rep REQ2REP [] = {
    100,"Réponse à la requete 100",
    150,"Réponse à la requete 150",
    200,""
};
*/



#define MSG "CECI EST UN MESSAGE"
#define OK "MSG bien recu"
#define ERR "Erreur"

#endif

//cmd netstat macos
/* netstat -f inet -p tcp -na */
