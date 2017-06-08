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
#include <netdb.h>
#define PORT 10000

//Positions à envoyer
int* posX ;
int* posY ;

//clef de testVB
char* clefCanal = "1494771555601_5SGQdxJaJ8O1HBj4";
char* ressource = "msg";


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

void error(const char *msg)
{
  fprintf(stderr, "%s", msg);
  exit(0);
}

int envoiBeeBotte(char* data[], char *canal)
{
    // data est un tableau de chaines (char[]), c-a-d un tableau de char a deux dimensions
    // printf("data[0] is %s\n",data[0]);
    //printf("data[3] is %s\n",data[3]);

    char *host = "api.beebotte.com";
    /* !! TODO remplacer 'testVB' par le canal dans lequel publier (ex: partie12)
        (ici msg est la "ressource" que ce canal attend */
    char path[100] = "/v1/data/write/";
    strcat(path,canal);strcat(path,"/"); strcat(path,ressource);
    struct hostent *server;
    struct sockaddr_in serv_addr;
    int sockfd, bytes, sent, received, total, message_size;
    char *message, response[4096];

    // Necessaire pour envoyer des donnees sur beebottle.com (noter le token du canal a la fin) :
    char headers[300] ="Host: api.beebotte.com\r\nContent-Type: application/json\r\nX-Auth-Token: ";
    strcat(headers,clefCanal);strcat(headers,"\r\n");

    char donnees[4096] = "{\"data\":\""; // "data" est impose par beebotte.com


    // Norme d'envoi projet FAR
    strcat(donnees,"type_msg=");
    strcat(donnees,data[0]);
    strcat(donnees,",");

    strcat(donnees,"type_ent=");
    strcat(donnees,data[1]);
    strcat(donnees,",");

    strcat(donnees,"num=");
    strcat(donnees,data[2]);
    strcat(donnees,",");

    strcat(donnees,"data=");
    strcat(donnees,data[3]);
    strcat(donnees,"\"}");

    /* How big is the whole HTTP message? (POST) */
    message_size=0;
    message_size+=strlen("%s %s HTTP/1.0\r\n")+strlen("POST")+strlen(path)+strlen(headers);
    message_size+=strlen("Content-Length: %d\r\n")+10+strlen("\r\n")+(int)strlen(donnees);
    /* allocate space for the message */
    message=malloc(message_size);

    /* Construit le message POST */
    sprintf(message,"POST %s HTTP/1.0\r\n",path);
    sprintf(message+strlen(message), "%s",headers);
    sprintf(message+strlen(message),"Content-Length: %d\r\n",(int)strlen(donnees));
    strcat(message,"\r\n");              /* blank line     */
    strcat(message,donnees);             /* body           */

    /* What are we going to send? */
  //  printf("Request:\n%s\n-------------\n",message);

    /* create the socket */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
      error("ERROR opening socket");
    }

    /* lookup the ip address */
    server = gethostbyname(host);
    if (server == NULL)
    {
      error("ERROR, no such host");
    }

    /* fill in the structure */
    memset(&serv_addr,0,sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(80); // port 80
    memcpy(&serv_addr.sin_addr.s_addr,server->h_addr,server->h_length);
    /* connect the socket */
    if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0)
    {
      error("ERROR connecting");
    }
    /* send the request */
    total = strlen(message);
    sent = 0;
    do {
        bytes = write(sockfd,message+sent,total-sent);
        if (bytes < 0)
            error("ERROR writing message to socket");
        if (bytes == 0)
            break;
        sent+=bytes;
    } while (sent < total);

    /* receive the response */
    memset(response,0,sizeof(response));
    total = sizeof(response)-1;
    received = 0;
    do {
        bytes = read(sockfd,response+received,total-received);
        if (bytes < 0) error("ERROR reading response from socket");
        if (bytes == 0)
            break;
        received+=bytes;
    } while (received < total);

    if (received == total) error("ERROR storing complete response from socket");
    /* close the socket */
    close(sockfd);

    /* process response */
    printf("Response:\n%s\n",response);

    free(message);
    return 0;
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
    printf("Joueur %d, entrez l'adresse IP de votre robot \n", i);
    fgets(adressesRobots[i],16*sizeof(char),stdin) ;
    flush_buffer(adressesRobots[i]);
  }

  for(int j=0;j<6;j++)
  {
    fprintf(stderr, "%s\n", adressesRobots[j]);
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
  fprintf(stderr, "Robot %d connecté avec le pid %ld ppid %ld\n\n", num, (long)getpid(), ppid);

   sleep(2) ;// SERT à RIEN

   if(num==0){

    fprintf(stderr,"Le processus %d crée le token \n",num);
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
      fprintf(stderr,"\nJoueur %d, à ton tour : \n",num);

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
        FILE * xfile = fopen( "X", "w+");
        FILE * yfile = fopen( "Y", "w+");
        system("chmod +777 X");
        system("chmod +777 Y");
        fputs(x,xfile);
        fputs(y,yfile);

        //fputs(x,xfile) ;
        //fputs(y,yfile) ;
        /* envoi au serveur */
        send(sock,x,4,0) ;
        send(sock,y,4,0) ;

      //  char* canalBeebotteCoord = "testVB";


        char* data = malloc(256*sizeof(char));
        sprintf(data,"%s+%s+%s",adressesRobots[num],x,y);
        flush_buffer(data);
        char* mess [4] = {"COORD","SP","2",data};

        envoiBeeBotte(mess,"testVB");

        fclose(xfile) ;
        fclose(yfile) ;
        flush_buffer(y);
      }

      if (pidSaisie != 0) {
        //attendre la reponse du fils :
        unsigned int retTime = time(0) + 30;
        char bufferx[10] = "" ;
        char buffery[10] = "" ;
        while ((strlen(bufferx) == 0 || strlen(buffery) == 0) && time(0) < retTime){ //tant que le buffer est vide (le fichier est vide) et que le temps n'est pas écoulé
          FILE * xfile = fopen( "X", "r");
          FILE * yfile = fopen( "Y", "r");
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
