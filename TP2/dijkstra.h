/* fichier dijkstra.h */
/* Implementation des semaphores de Dijkstra a l'aide des semaphores
* de SystemV.
*
* sem_create(): creation d'un semaphore
* P() : realisation de l'operation P sur un semaphore
* V() : realisation de l'operation V sur un semaphore
* sem_delete() : suppression d'un semaphore
*/
union semun {
	int val ;
	struct semid_ds *buf ;
	ushort *array ;
	} arg_ctl ;
	
int sem_create(cle, initval) /* creation d'un semaphore relie a cle */
/* de valeur initiale initval */
key_t cle ;

int initval ;
{
	int semid ;
	
	
	semid = semget(ftok("dijkstra.h",cle),1,IPC_CREAT|IPC_EXCL|0666) ;
	if (semid == -1) {
	semid = semget(ftok("dijkstra.h",cle),1, 0666) ;
	if (semid == -1) {
	perror("Erreur semget()") ;
	exit(1) ;
	}
	}
	arg_ctl.val = initval ;
	if (semctl(semid,0,SETVAL,arg_ctl) == -1) {
	perror("Erreur initialisation semaphore") ;
	exit(1) ;
	}
	printf("Creation du semaphore d'identificateur: %d avec valeur %d\n", semid, semctl(semid,0,GETVAL,arg_ctl)) ;
	return(semid) ;
}

void P(semid)
int semid ;
{
	struct sembuf sempar ;
	sempar.sem_num = 0 ;
	sempar.sem_op = -1 ;
	sempar.sem_flg = SEM_UNDO ;
	if (semop(semid, &sempar, 1) == -1)
	perror("Erreur operation P") ;
	else
	printf("le processus %d effectue l'operation P sur le semaphore %d valeur %d\n\n", semctl(semid,0,GETPID,arg_ctl), semid, semctl(semid,0,GETVAL,arg_ctl));
}

void V(semid)
int semid ;
{
	struct sembuf sempar ;
	sempar.sem_num = 0 ;
	sempar.sem_op = 1 ;
	sempar.sem_flg = SEM_UNDO ;
	if (semop(semid, &sempar, 1) == -1)
	perror("Erreur operation V") ;
	else
	printf("le processus %d effectue l'operation V sur le semaphore %d valeur %d\n\n", semctl(semid,0,GETPID,arg_ctl), semid, semctl(semid,0,GETVAL,arg_ctl));
}

void sem_delete(semid)
int semid ;
{
	if (semctl(semid,0,IPC_RMID,0) == -1)
	perror("Erreur dans destruction semaphore") ;
	else 
	printf("Destruction du semaphore d'identificateur: %d\n", semid);
}

