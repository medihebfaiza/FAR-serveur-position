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
#define PORT 10000

void connectToRobots(char** adressesRobots, int num){
  
   int sock;
   struct sockaddr_in sin;
   char buffer[32] = "";

   /* Creation de la socket */
   sock = socket(AF_INET, SOCK_STREAM, 0);

  fprintf(stderr,"socket configured\n");//TEST   

       /* Configuration de la connexion */
      sin.sin_addr.s_addr = inet_addr(adressesRobots[num]);
      sin.sin_family = AF_INET;
      sin.sin_port = htons(PORT);

      /* Tentative de connexion au serveur */
      connect(sock, (struct sockaddr*)&sin, sizeof(sin));
      recv(sock, buffer, 32, 0);
      fprintf(stderr,"Connexion a %s sur le port %d\n", inet_ntoa(sin.sin_addr),
         htons(sin.sin_port));
      printf("le serveur a dit : %s \n",buffer) ;
}

int main(int argc,  char *argv[ ]) {
   pid_t childpid;             /* indicates process should spawn another     */
   int error;                  /* return value from dup2 call                */
   int fd[2];                  /* file descriptors returned by pipe          */
   int i = 0 ;                      /* number of this process (starting with 0)   */
   int nprocs= 6;                 /* total number of processes in ring          */
   char** adressesRobots = (char**) malloc(6*sizeof(char*));
   int num=0;
   char adresse[16] = "" ;
   long ppid;

   for (i=0;i<6;i++){
    adressesRobots[i] = (char*) malloc(16*sizeof(char)) ;
    printf("Joueur %d, Entrez l'adresse de votre Robot \n", i);
    fgets(adresse,16,stdin) ;
    adressesRobots[i] = adresse ;
   }
   i = 0;
   printf("data initialized\n");//TEST

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

   //char buffer[32] = "";

   for (i = 0; i < nprocs-1; i++) {        /* create the remaining processes */



      if (pipe (fd) == -1) {
         fprintf(stderr, "[%ld]:failed to create pipe %d: %s\n",
                (long)getpid(), i, strerror(errno));
         return 1;
      }
      if ((childpid = fork()) == -1) {
         fprintf(stderr, "[%ld]:failed to create child %d: %s\n",
                 (long)getpid(), i, strerror(errno));
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
         fprintf(stderr, "[%ld]:failed to dup pipes for iteration %d: %s\n",
                 (long)getpid(), i, strerror(errno));
         return 1;
      }
      if ((close(fd[0]) == -1) || (close(fd[1]) == -1)) {
         fprintf(stderr, "[%ld]:failed to close extra descriptors %d: %s\n",
                (long)getpid(), i, strerror(errno));
         return 1;
      }
      if (childpid){
        break ;
      }

   }      

   connectToRobots(adressesRobots,num);                                         /* say hello to the world */
   fprintf(stderr, "This is process %d with ID %ld and parent id %ld\n",
           num, (long)getpid(), ppid);
   
   return 0;
}
