// client.c
// client tcp basique

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#define PORT 10000

int main(void) {
  char buffer[32] = "";

  int sock;
  struct sockaddr_in sin;

  /* Creation de la socket */
  sock = socket(AF_INET, SOCK_STREAM, 0);

  /* Configuration de la connexion */
  sin.sin_addr.s_addr = inet_addr("127.0.0.1");
  sin.sin_family = AF_INET;
  sin.sin_port = htons(PORT);

  /* Tentative de connexion au serveur */
  connect(sock, (struct sockaddr*)&sin, sizeof(sin));
  recv(sock, buffer, 32, 0);
  printf("Connexion a %s sur le port %d\n", inet_ntoa(sin.sin_addr),
         htons(sin.sin_port));
  printf("le serveur a dit %s \n",buffer) ;
  while(1){
	  char buffer2[32] = "" ;
	  fgets(buffer2, sizeof buffer2, stdin);
          printf("%s\n",buffer2);
	  send(sock, buffer2, 32, 0);
  }
  /* Fermeture de la socket client */
  close(sock);

  return EXIT_SUCCESS;

}//main
