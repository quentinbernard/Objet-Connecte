//******************************************//
//       	  fredzer.h		    //
// Contient variables et fonctions communes //
//******************************************//

#include <stdio.h>
#include <stdlib.h>

#define CHECK(sts, msg) if((sts) == -1) { perror(msg); exit(-1); }

#define CHECK_IF(stat, val, msg) if((stat) == (val)){ perror(msg);exit(EXIT_FAILURE);}
