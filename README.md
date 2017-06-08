# Projet IG3 - Fondamentaux des Applications Réparties (FAR) 2017
par Mohamed Iheb FAIZA et Cyprien LEGRAND

## Gestionnaire de positions
Un dispositif qui permet aux robots dans un jeu de demander de l’assistance à leurs concepteurs pour arriver à se localiser

Notre gestionnaire de position est programmé de la manière suivante :
Sur chaque robot est implémenté la partie serveur (fichier serveur.c) servant
à initialiser une socket pour que le gestionnaire de position s'y connecte
et y envoie des données (ici les coordonnées que les joueurs auront entré
pour leur robot).

Le code principal du gestionnaire de position se trouve dans le fichier
tokenRing.c.

Ce programme demande à chaque joueur de rentrer manuellement l'IP de leur robot
en début de partie. Une fois que les 6 robots ont entré leur IP correctement, un
token ring se crée : le processus principal fork, crée un tube entre lui et son
fils, et redirige sa sortie standard vers l'entrée standard du fils. Son fils va
à son tour forker, et ainsi de suite jusqu'à obtenir 6 processus reliés. Chaque
processus va alors se faire passer un token : seul le joueur correspondant au
processus qui a le token pourra entrer ses coordonnées à un moment donné. Une
fois que les coordonnées ont été saisies (grâce à un fgets d'un fork du processus
qui détient le token, qui stocke ces coordonnées dans des fichiers X et Y, qui
sont ensuite lus par le père), ces coordonnées sont envoyées à la fois au robot
grâce au programme serveur.c se trouvant dessus (par socket donc), ainsi que sur
BeeBotte (format IPRobot+coordonnéeX+coordonnéeY).

### Compiler :
~~~
make && make clean
~~~

### Exécuter :
Sur le robot :
~~~
./serveur
~~~

Sur la machine qui héberge le gestionnaire de positions :
~~~
./token
~~~
