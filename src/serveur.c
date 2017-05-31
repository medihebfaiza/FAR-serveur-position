// serveur.c
// serveur tcp basique pour les robots
// Prob : le processus père rentre dans une boucle infinie et donc un deuxième client ne peut pas se connecter

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
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

  int X = -1 ;
  int Y = -1 ;
  char sX[4] = "" ;
  char sY[4] = "" ;

  /* connexion au client */
  csock = accept(sock, (struct sockaddr*)&csin, &crecsize);

  printf("client connected with socket %d \n",csock );

  while( 1 ){ //on recoit plusieurs messages
      recv(csock, sX, 4, 0) ;
      X = atoi(sX);
      //printf("J'ai recu sX = %s, apres convertion X = %d \n", sX, X);
      recv(csock, sY, 4, 0) ;
      Y = atoi(sY);
    //  printf("J'ai recu sY = %s, apres convertion Y = %d \n", sY, Y);
  }

  /* Fermeture de la socket client et de la socket serveur */
  close(csock);
  close(sock);

  return EXIT_SUCCESS;

}//main
