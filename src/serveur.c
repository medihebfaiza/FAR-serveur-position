// serveur.c
// serveur tcp basique
// Prob : on écrase la socket à chaque tour de boucle

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#define PORT 10000

int main(void) {


  /* Socket et contexte d'adressage du serveur */
  struct sockaddr_in sin;
  int sock;
  socklen_t recsize = sizeof(sin);

  /* Socket et contexte d'adressage du client */
  struct sockaddr_in csin;
  int csock;
  socklen_t crecsize = sizeof(csin);

  /* Creation d'une socket */
  sock = socket(AF_INET, SOCK_STREAM, 0);

  /* Configuration */
  sin.sin_addr.s_addr = htonl(INADDR_ANY);  /* Adresse IP automatique */
  sin.sin_family = AF_INET;                 /* Protocole familial (IP) */
  sin.sin_port = htons(PORT);               /* Listage du port */
  bind(sock, (struct sockaddr*)&sin, recsize);

  /* Demarrage du listage (mode server) */
  listen(sock, 5);

  /* Descriteur du Pipe pour la communication entre processus père/fils */
  int descr[2];

  /* Tableau pour stocker les csock des clients */
  int clients[10];
  int nbclients = 0 ;

  /* Attente d'une connexion client */
  while(1){
	  char *buffer = "Bonjour ! \n";
	  csock = accept(sock, (struct sockaddr*)&csin, &crecsize);
	  pid_t pid ;
    pipe(descr);
	  pid = fork() ;
	  if (pid == 0) {
		  printf("Un client se connecte avec la socket %d de %s:%d\n",
			 csock, inet_ntoa(csin.sin_addr), htons(csin.sin_port));
      /* envoi du csock au processus père */
      close(descr[0]);
      write(descr[1],&csock,sizeof(int));

      /* envoi du premier message au client */
		  send(csock, buffer, 32, 0);
		  printf("%s\n",buffer) ;
  		  while(1){ //on recoit plusieurs messages
			  char buffer2[32] = "" ;
			  recv(csock, buffer2, 32, 0);
			  printf("Le client %d a dit %s \n", csock, buffer2) ;
		  }
		  /* Fermeture de la socket client et de la socket serveur */
		  close(csock);
	  }
    else {
      close(descr[1]);
      read(descr[0], &csock, sizeof(int)) ;
      printf("ajout du client %d \n", csock);
      clients[nbclients] = csock ;
      nbclients++ ;
      printf("nb clients = %d \n", nbclients);
    }
  }
  close(sock);

  return EXIT_SUCCESS;

}//main
