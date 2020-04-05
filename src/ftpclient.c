
#include "csapp.h"
#include <time.h>
#define TAILLE_BUFFER 10


void get_cmd(char buf[],char cmd[]){
    int i;
    
    for(i = 0; buf[i] != ' ' && buf[i] != '\n'; i++)cmd[i] = buf[i];
    
    if(buf[i+1] == '-' && buf[i+2] == 'r'){
        strcat(cmd, " -r");
    }
}


void get_fichier(char buf[], char fichier[]){
    int i, j;
    
    for(i = 0; buf[i] != ' '; i++){}
    i++;

    for(j = i; buf[j] != '\0'; j++){
        fichier[j-i] = buf[j];
    }

    fichier[j] = '\0';
}


void stat_transfere(clock_t debut, clock_t fin, int somme){
    double temps = (double)(fin-debut)*1000/CLOCKS_PER_SEC;
    double kilo_bits_par_sec=somme;
    kilo_bits_par_sec /= 100;
    
    if(temps != 0.0){
        kilo_bits_par_sec /= temps;
    }

    printf("%d bytes received in %f seconds\n(%f Kbits/sec)\n", somme, temps, kilo_bits_par_sec);
}


void envoi_fichier(rio_t rio, int clientfd, char fichier[], char buf[]){
    int fd, n;
    int donnee[TAILLE_BUFFER];
    fichier[strlen(fichier)-1] = '\0';

    fd = open(fichier, O_RDONLY);
    
    if(fd < 0){
        printf("Erreur de fichier\n");
    
    } else {
        Rio_writen(clientfd, buf, MAXBUF);
        memset(buf, 0, MAXBUF);
        Rio_readlineb(&rio, buf, MAXBUF);
        printf("%s", buf);
        
        if(!strcmp(buf,"Création du fichier ok\n")){
            
            while((n = Read(fd, donnee, TAILLE_BUFFER)) > 0){
                Rio_writen(clientfd, donnee, n);
            }
        
        } else {
            printf("Erreur lors de la création du fichier\n");
        }
    }
    close(fd);
}


void create_mkdir(rio_t rio, int clientfd, char buf[]){
    Rio_writen(clientfd, buf, strlen(buf));
    Rio_readlineb(&rio, buf, MAXBUF);
    printf("%s", buf);
}


void supp_fich(rio_t rio,int clientfd,char buf[]){
    Rio_writen(clientfd, buf, strlen(buf));
    
    if (Rio_readlineb(&rio, buf, MAXBUF) > 0){
        printf("%s",buf);
    }
}


int security(rio_t rio, int clientfd){
    char bool[10] = "1";
    char login[MAXBUF];
    char mdp[MAXBUF];
    
    Fputs("Saisir votre login :\n", stdout);
    Fgets(login, MAXBUF, stdin);
    Rio_writen(clientfd, login, strlen(login));
    Fputs("Saisir votre mot de passe :\n", stdout);

    while(bool[0] == '1' && (Fgets(mdp, MAXBUF, stdin) != NULL)){
        Rio_writen(clientfd, mdp, strlen(mdp));
        Rio_readlineb(&rio, bool, 10);
        memset(mdp, 0, MAXBUF);
        
        if(bool[0] == '1'){
            Fputs("Saisir votre mot de passe :\n", stdout);
        }
    }

    bool[strlen(bool)-1] = '\0';
    if(!strcmp(bool, "0")){
        return 1;
    } else {
        return 0;
    }
}



int main(int argc, char **argv){
    int clientfd, port;
    char *host, fichier[MAXBUF],buf[MAXBUF], cmd[10];
    rio_t rio;
    int autorisation;
    clock_t debut,fin;
    struct sockaddr_in clientaddr;
    socklen_t clientlen = sizeof(clientaddr);
    struct stat stat;

    #ifndef DEBUG
    if (argc != 2) {
        fprintf(stderr, "usage: %s <host>\n", argv[0]);
        exit(0);
    }
    host = argv[1];
    #else
    host = "127.0.0.1";
    #endif
    port = 2121;

    /*
     * Note that the 'host' can be a name or an IP address.
     * If necessary, Open_clientfd will perform the name resolution
     * to obtain the IP address.
     */
    clientfd = Open_clientfd(host, port);
    
    /*
     * At this stage, the connection is established between the client
     * and the server OS ... but it is possible that the server application
     * has not yet called "Accept" for this connection
     */
    printf("client connected to %s\n",argv[1]); 
    
    getpeername(clientfd, (SA *) &clientaddr, &clientlen);
    printf("numero de port distant : %d\n", ntohs(clientaddr.sin_port));
    Rio_readinitb(&rio, clientfd);
   
   #ifdef SECU
        autorisation=security(rio,clientfd);
    #else
        autorisation=1;
    #endif

    while(1 && autorisation){
       
        printf("ftp> ");
        int somme = 0;

        if (Fgets(buf, MAXLINE, stdin) != NULL) {

            get_cmd(buf, cmd);
            if (!strcmp(cmd, "resume")){
                Rio_writen(clientfd, buf, strlen(buf));
                int flog = open(".log", O_RDONLY, 0666);
                char nom_fichier[MAXBUF];
                rio_t fdlog, rior;

                Rio_readinitb(&fdlog,flog);
                Rio_readlineb(&fdlog,nom_fichier,MAXBUF);
                
                nom_fichier[strlen(nom_fichier)]='\n';
                Rio_writen(clientfd,nom_fichier,MAXBUF);                
                
                Rio_readlineb(&rio, nom_fichier, MAXBUF);
                printf("nom fichier = %s\n", nom_fichier);
                
                
                int fd;
                nom_fichier[strlen(nom_fichier)-1] = '\0';
                fd = open(nom_fichier, O_WRONLY, 0666);
                
                if (fd < 0){
                    
                    printf("le fichier ne s'est pas ouvert!!!\n");
                    Rio_readlineb(&rio, buf, MAXBUF);
                
                } else {
                    Rio_readinitb(&rio, fd);
                    Fstat (fd, &stat);
                    int nbre_p = stat.st_size/TAILLE_BUFFER;
                    printf("nbre_p = %d", nbre_p);

                    memset(buf, 0, sizeof(buf));
                    Lseek(fd, 0, SEEK_END);
                    int taille;
                    size_t n;
                    char buf[TAILLE_BUFFER];
                    char taille_buf[4];
                    Rio_readinitb(&rior, clientfd);

                    while((Rio_readnb(&rior, taille_buf, 4) > 0) &&  ((taille = atoi(taille_buf)) !=0)){
                        n = Rio_readnb(&rior, buf, taille);
                        write(fd, buf, n);
                    }

                    memset(buf, 0, MAXBUF);
                    close(flog);
                    close(fd);
                    
                    printf("Transfer successfully complete.\n");
                   remove(".log");
                   exit(0);
            }
        } 

            if(!strcmp(cmd, "get")){ // Code pour la commande get

                Rio_writen(clientfd, buf, strlen(buf));
                debut = clock();
                
                if (Rio_readlineb(&rio, &fichier, MAXBUF) > 0) {
                    fichier[strlen(fichier)-1] = '\0';
                    printf("Nom du fichier en réception : %s\n", fichier);
                    int fd, taille;
                    size_t n;
                    char buf[TAILLE_BUFFER];
                    char taille_buf[4];
                    fd = open(fichier, O_CREAT | O_WRONLY, 0666);
                    
                    while((Rio_readnb(&rio, taille_buf, 4) > 0) &&  ((taille = atoi(taille_buf)) !=0)){
                        n = Rio_readnb(&rio, buf, taille);
                        somme += n;
                        write(fd, buf, n);
                    }
                    int nbre_de_paquets_recus;
                    
                    if (n == TAILLE_BUFFER){
                        nbre_de_paquets_recus = (somme/TAILLE_BUFFER);
                    } else { 
                        nbre_de_paquets_recus = (somme/TAILLE_BUFFER)+1;
                    }
                    
                    close(fd);
                    fin = clock();
                    printf("Transfer successfully complete.\n");
                    stat_transfere(debut, fin, somme);
                    Rio_readlineb(&rio,buf,strlen(buf));

                    int nbuf = atoi(buf);
                    printf("nbre_de_paquets_recus : %d\nnbuf : %d\n", nbre_de_paquets_recus,nbuf);
                    
                    if (nbre_de_paquets_recus == nbuf){
                        //printf("transfert realisé à 100 pourcents\n");
            
                    } else {
                        //reprendre la lecture a un certain point du fichier
                        //creer un fichier log dans lequel il y le nbre de paquets a telecharger
                        
                        int flog = open(".log", O_CREAT | O_WRONLY, 0666);
                        int arret_buf = nbuf;
                        char abuff[MAXLINE];
                        sprintf(abuff, "%d", arret_buf);
                        strcat(fichier, " ");
                        write(flog, fichier, strlen(fichier));
                        write(flog, abuff, strlen(abuff));
                        //on met le nom du fichier qu'on a lu et on met un espace
                        //pour ajouter ensuite le nombre de paquets recus
                        close(flog);
                    }
                }
            }

            else if(!strcmp(cmd, "put")){
                get_fichier(buf,fichier);
                printf("Début du transfère du fichier : %s", fichier);
                envoi_fichier(rio, clientfd, fichier, buf);
                printf("Fin du transfère\n");

            }

            else if(!strcmp(cmd, "cat")){
                Rio_writen(clientfd, buf, strlen(buf));
                memset(buf, 0, MAXBUF);
                while(Rio_readnb(&rio, buf, MAXBUF)){
                    Fputs(buf, stdout);
                }
            }

            else if(!strcmp(cmd, "ls")){
                int n;
                Rio_writen(clientfd, buf, strlen(buf));
                if((n = Rio_readlineb(&rio,&buf, MAXBUF) ) != 0){               
                printf("%s", buf);
                fflush(stdout);
                }
            }

            else if(!strcmp(cmd, "mkdir")){
                create_mkdir(rio, clientfd, buf);
            }

            else if(!strcmp(cmd, "rm")){
                supp_fich(rio, clientfd, buf);
            }

            else if(!strcmp(cmd, "bye")){
                printf("Fin de la connection\n");
                Close(clientfd);
                exit(0);
            }

            else if(!strcmp(cmd,"cd")){
                Rio_writen(clientfd, buf, strlen(buf));
                printf("changement de repertoire\n");
                memset(buf, 0, MAXBUF);
                Rio_readlineb(&rio, &buf, MAXLINE);
            }

            else if(!strcmp(cmd,"rm -r")){
                Rio_writen(clientfd, buf, strlen(buf));

                if(Rio_readlineb(&rio, &buf, MAXLINE) > 0){
                    printf("%s", buf);
                }
            }

            else if (!strcmp(cmd, "pwd")){
                Rio_writen(clientfd, buf, strlen(buf));
                memset(buf, 0, MAXBUF);
                Rio_readlineb(&rio, &buf, MAXLINE);
                printf("%s", buf);
            }

            else { /* the server has prematurely closed the connection */
                printf("entrez une commande valide!\n");
               // exit(0);

            }

        memset(cmd, 0, MAXBUF);
        memset(buf, 0, MAXBUF);
        }
    }
    Close(clientfd);
    exit(0);
}
