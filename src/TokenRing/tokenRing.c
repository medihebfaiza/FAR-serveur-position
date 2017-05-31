#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <signal.h>
#include <time.h>
#define PORT 10000

//Positions à envoyer
int* posX ;
int* posY ;

void setX(int x) {
  *posX = x ;
}

void setY(int y) {
  *posY = y ;
}

void connectToRobots(char** adressesRobots, int num, int* sock){
  struct sockaddr_in sin;

  /* Creation de la socket */
  *sock = socket(AF_INET, SOCK_STREAM, 0);

  /* Configuration de la connexion */
  sin.sin_addr.s_addr = inet_addr(adressesRobots[num]);
  sin.sin_family = AF_INET;
  sin.sin_port = htons(PORT);

  /* Tentative de connexion au serveur */
  connect(*sock, (struct sockaddr*)&sin, sizeof(sin));
  fprintf(stderr,"Connexion a %s sur le port %d\n", inet_ntoa(sin.sin_addr), htons(sin.sin_port));

}


void flush_buffer(char* x){
  char *pos = strchr(x, '\n');
  if(pos){
    *pos = '\0';
  }
  else{
    int c = 0;
    while((c = getchar()) != '\n' && c != EOF){};
  }
}


int main(int argc,  char *argv[ ]) {
  pid_t childpid;             /* indicates process should spawn another     */
  int error;                  /* return value from dup2 call                */
  int fd[2];                  /* file descriptors                           */
  int i = 0 ;
  int nprocs= 6;                 /* nombre total de process dans le ring    */
  char** adressesRobots = (char**) malloc(6*sizeof(char*));
  int num=0;
  char adresse[16] = "" ;
  long ppid;
  char token[10];
  int keyboard ;
  posX = (int*) malloc (sizeof(int)) ;
  *posX = -1 ;
  posY = (int*) malloc (sizeof(int)) ;
  *posY = -1 ;

  /* sauvegarder le desc de l'entrée clavier*/
  keyboard = dup(STDIN_FILENO);

  for (i=0;i<6;i++){
    adressesRobots[i] = (char*) malloc(16*sizeof(char)) ;
    printf("Joueur %d, Entrez l'adresse de votre Robot \n", i);
    fgets(adresse,16,stdin) ;
    adressesRobots[i] = adresse ;
  }
  i = 0;
  printf("data initialized\n");
  if (pipe (fd) == -1) {      /* connect std input to std output via a pipe */
    perror("Failed to create starting pipe");
    return 1;
  }
  if ((dup2(fd[0], STDIN_FILENO) == -1) ||
    (dup2(fd[1], STDOUT_FILENO) == -1)) {
    perror("Failed to connect pipe");
    return 1;
  }
  if ((close(fd[0]) == -1) || (close(fd[1]) == -1)) {
    perror("Failed to close extra descriptors");
    return 1;
  }

  for (i = 0; i < nprocs-1; i++) {        /* création des process */
    if (pipe (fd) == -1) {
      fprintf(stderr, "[%ld]:failed to create pipe %d: %s\n", (long)getpid(), i, strerror(errno));
      return 1;
    }
    if ((childpid = fork()) == -1) {
      fprintf(stderr, "[%ld]:failed to create child %d: %s\n", (long)getpid(), i, strerror(errno));
      return 1;
    }

    if (childpid > 0)               /* for parent process, reassign stdout */
      error = dup2(fd[1], STDOUT_FILENO);
    else {                              /* for child process, reassign stdin */
      error = dup2(fd[0], STDIN_FILENO);
      num=i+1;
      ppid=(long)getppid();
    }
    if (error == -1) {
      fprintf(stderr, "[%ld]:failed to dup pipes for iteration %d: %s\n", (long)getpid(), i, strerror(errno));
      return 1;
    }
    if ((close(fd[0]) == -1) || (close(fd[1]) == -1)) {
      fprintf(stderr, "[%ld]:failed to close extra descriptors %d: %s\n", (long)getpid(), i, strerror(errno));
      return 1;
    }
    if (childpid){
      break ;
    }
  }


  int sock = 0;
  connectToRobots(adressesRobots,num,&sock);                                         
  fprintf(stderr, "This is process %d with ID %ld and parent id %ld\n", num, (long)getpid(), ppid);

   sleep(2) ;// SERT à RIEN

   if(num==0){

    fprintf(stderr,"Proc %d je crée le token \n",num);
    sprintf(token,"abcde");
    //close(STDIN_FILENO) ;//fermer le pipe
    write(STDOUT_FILENO,token,10*sizeof(char)+1);
    //sprintf(token," ");
   }

   while(true){
    //close(STDOUT_FILENO) ;
    //sprintf(token," ");
    char copietoken[10] ;
    read(STDIN_FILENO,copietoken,10*sizeof(char)) ;
    if(strcmp("abcde",copietoken)== 0){
      fprintf(stderr,"\nProc %d mon tour, token : %s\n",num,copietoken);

      //fork
      pid_t pidSaisie = fork() ;

      if (pidSaisie == 0) {//proc fils
        char* x = (char*) malloc(sizeof(char)*10);
        char* y = (char*) malloc(sizeof(char)*10); ;
        dup2(keyboard,STDIN_FILENO) ;

        fprintf(stderr,"Entrez coordonnées  :\n");
        fprintf(stderr,"X = ");

        fgets(x,10*sizeof(char),stdin) ;

        //purge du flux stdin
        //fflush(stdin) ;
        flush_buffer(x);



        fprintf(stderr,"Y = ") ;
        fgets(y,10*sizeof(char),stdin) ;



		//---Envoyer les coordonnées saisies dans un fichier--
        remove("X");
        remove("Y");
        FILE * xfile = fopen( "./X", "w+");
        FILE * yfile = fopen( "./Y", "w+");
        fputs(x,xfile) ;
        fputs(y,yfile) ;
        /* envoi au serveur */
        send(sock,x,4,0) ;
        send(sock,y,4,0) ;
        fclose(xfile) ;
        fclose(yfile) ;
        flush_buffer(y);
      }

      if (pidSaisie != 0) {
        //attendre la reponse du fils :
        unsigned int retTime = time(0) + 10;
        char bufferx[10] = "" ;
        char buffery[10] = "" ;
        while ((strlen(bufferx) == 0 || strlen(buffery) == 0) && time(0) < retTime){ //tant que le buffer est vide (le fichier est vide) et que le temps n'est pas écoulé
          FILE * xfile = fopen( "./X", "r");
          FILE * yfile = fopen( "./Y", "r");
          if (xfile && yfile) {
            fgets(bufferx,10,xfile) ;
            fgets(buffery,10,yfile) ;
            fclose(xfile) ;
            fclose(yfile) ;
          }

        }

    	//Affichage des positions

        fprintf(stderr,"Nouveau X : %s \n", bufferx);
        fprintf(stderr,"Nouveau Y : %s \n", buffery);
        remove("X");
        remove("Y");

        //tuer le fils
        kill(pidSaisie, SIGTERM) ;

        //reinitialiser les vars globaux
        /*setX(-1) ;
        setY(-1) ;
        */
        //sleep(2) ;
      //close(STDIN_FILENO);

      write(STDOUT_FILENO,copietoken,10*sizeof(char)); //passage du token
      sprintf(copietoken," ");
      }
    }
   }

   return 0;
}
