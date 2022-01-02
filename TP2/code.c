#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include "dijkstra.h"
#include <unistd.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <errno.h>

#define CLE 123
#define CLE_sem_nv 1
#define CLE_sem_np 2




//global
int n, m, sem_np, sem_nv;;
int fd[2]; /* descripteur du tube */

//procedure deposer du producteur
int Deposer(int art, int *tampon){
	static int t = 0;
		/*
	* ecriture dans la zone de memoire partagee
	*/
	*(tampon+t) = art;
	printf("P1 depose dans le tampon la valeur %d a la case %d \n", art, t);
	t = (t+1) % m;
	return art;
}

//procedure prelever du consommateur
int Prelever(int *tampon){
	static int q = 0;
	int art;
	art = *(tampon+q);
	printf("P2 preleve du tampon la valeur %d a la case %d\n", art, q);
	q = (q+1) % m;
	return art;
}

//creation de la memoire partagée
int creer_mem_partage(){
	int shmid ;
	int size = sizeof(int) * 5;
	char* path = "code.c";
	

	if (( shmid = shmget(ftok(path ,(key_t)CLE), size, IPC_CREAT|0666)) == -1) {
		perror("Echec de shmget") ;
		exit(1) ;
	}
	
	printf("creation de la memoire partagé reussi\n\n");
	return shmid;
}



// Producteur
void P1(){
	int i = 0;
	int art;	
	int* mem;
	int flag = 0;
	int shmid = creer_mem_partage();
	

	
	/*
	* attachement du processus a la zone de memoire
	* recuperation du pointeur sur la zone memoire commune
	*/
	if ((mem = shmat(shmid, 0, flag)) == (int*)-1){
		perror("attachement impossible") ;
		exit (1) ;
	}
	
	//repeter jusqua i = n produire un nombre i
	while(i<n){
		//production de l'article
		i = i+1;
		//test si le tampon n'est pas plein sinon se bloque
		printf("-------------------------------------------------\n");
		P(sem_nv);
		printf("-------------------------------------------------\n");
		printf("P1 entre en section critique\n");
		//depot de l'article 		
		art = Deposer(i, mem);	
		printf("P1 depose %d dans la memoire partagé\n", art);
		//incremente np de 1 et reveille le consommateur
		V(sem_np);
		sleep(1);
	}
	
	/*
	* detachement du segment
	*/
	if (shmdt(mem)== -1){
		perror("detachement impossible") ;
		exit(1) ;
	}
	printf("detachement de la memoire partagée qui debute a l'adresse: %p par P1\n", mem);
	exit(0);
}

// Consommateur
void P2(){
	int i = 0;
	int* mem;
	int flag = 0;
	int shmid = creer_mem_partage();
	
		/*
	* attachement du processus a la zone de memoire
	* recuperation du pointeur sur la zone memoire commune
	*/
	if ((mem = shmat (shmid, 0, flag)) == (int*)-1){
		perror("attachement impossible") ;
		exit (1) ;
	}
	
	while(i!=n){
	//test si le tampon n'est pas vide sinon se bloque
	P(sem_np);
	printf("\n-------------------------------------------------\n");
	printf("P2 entre en section critique\n");
	i = Prelever(mem);	
	//incremente nv de 1 et reveille le producteur
	V(sem_nv);
	/*consommer l'article
	ecriture dans le tube */
	close(fd[0]);
	
	if ( write(fd[1], &i, sizeof(int)) == -1){
		perror("erreur lors de l'ecriture sur le tube");
	} else {
		printf("P2 viens d'envoyer par le tube la valeur %d\n", i);
	}
	
	
	}
	
	close(fd[1]);	
	
	/*
	* detachement du segment
	*/
	if (shmdt(mem)== -1){
		perror("detachement impossible") ;
		exit(1) ;
	}
	printf("detachement de la memoire partagée qui debute a l'adresse: %p par P2\n", mem);
	/*
	* destruction du segment
	*/
	if ( shmctl(shmid, IPC_RMID,0) == -1){
		perror("destruction impossible") ;
		exit(1) ;
	}
	
	printf("destruction de la memoire partagée id: %d par P2\n", shmid);
	exit(0);
}

void P3(){
	int i = 0;

	while(i!=n){
	
		close(fd[1]);
	
		if ( read(fd[0], &i, sizeof(int)) == -1){
			perror("erreur lors de la lecture sur le tube");
		} else {
			printf("P3 viens de recevoir par le tube la valeur %d\n", i);
		}

	}
	
	close(fd[0]);
	exit(0);
}


void main(){
	//parametres du programme
	n = 20;
	m = 5;
	
	
	//initialisation des semaphores
	sem_np = sem_create(CLE_sem_nv, 0);
	sem_nv = sem_create(CLE_sem_np, m);

	
	printf("---------------------------------------------------\n");
	
	//creation du tube
	if ( pipe(fd) < 0){
		perror("erreur lors de la creation du tube");
		exit(0);	
	}
	
		
	if (fork()==0) 
	{
		printf("Creation de P1 pid: %d\n", getpid());
		P1();
	} 
	
	if (fork()==0)
	{
		printf("Creation de P2 pid: %d\n", getpid());
		P2();
	}
	
	
	if (fork()==0)
	{
		P3();
	}
	
	
	//le pere attends que les processus crées finissent leurs travail avant de detruire les semaphores crées
	while( wait(0) != -1);

	sem_delete(sem_np);
	sem_delete(sem_nv);
}
