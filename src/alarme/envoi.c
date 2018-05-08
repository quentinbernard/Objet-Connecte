#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <stdlib.h>

int pid;

void setup(void){

	FILE * fp=NULL;
	//récupération pid alarme
	fp = fopen("alarme.pid", "r");
	if(fp==NULL){
		printf("Erreur, fichier alarme.pid manquant\n");
		exit(-1);
	}
	fscanf(fp,"%d\n",&pid);
	printf("PID de l'alarme: %d\n",pid);
}

void test(void){

	int read;

	printf("Envoi SIGUSR1 -> 1 || Envoi SIGUSR2 -> 2\n");
	scanf("%d",&read); 
	if(read==1){
		kill(pid, SIGUSR1);
		sleep(1);
	}else if (read ==2){
		kill(pid, SIGUSR2);
		sleep(1);
	}
}

int main(void){

	setup();
	for(;;)
		test();

return 0;
}
