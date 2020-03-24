/*
 * echoclient.c - An echo client
 */
#include "csapp.h"
#include <time.h>
#define TAILLE_BUFFER 256

void get_cmd(char buf[],char cmd[]){
    for(int i=0;buf[i]!=' ';i++)cmd[i]=buf[i];
}

int main(int argc, char **argv)
{
    int clientfd, port;
    char *host, buf[MAXLINE],cmd[10];
    rio_t rio;
    double temps,kilo_bits_par_sec;
    clock_t debut,fin;
    struct sockaddr_in clientaddr;
    ssize_t nb,b=0;
    socklen_t clientlen = sizeof(clientaddr);

    if (argc != 2) {
        fprintf(stderr, "usage: %s <host>\n", argv[0]);
        exit(0);
    }
    host = argv[1];
    port = 2121;
    int fd;

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
    printf("ftp> ");
    int n;
    int somme = 0;
    while (Fgets(buf, MAXLINE, stdin) != NULL) {
        Rio_writen(clientfd, buf, strlen(buf));
        get_cmd(buf,cmd);
        if(!strcmp(cmd,"get")){ // Code pour la commande get
            debut=clock();
            if (Rio_readlineb(&rio, &buf, strlen(buf)-4) > 0) {
                printf("Nom du fichier en réception : %s\n",buf);        
                fd=open(buf,O_CREAT | O_WRONLY,0666);
                while((n=Rio_readnb(&rio, buf, TAILLE_BUFFER)) > 0){
                    b=strlen(buf);
                    printf("n = %d\n", n);
                    //printf("b = %ld\n",b);
                    somme += n;
                    write(fd,buf,n);
                    nb+=b;    
                }
                printf("somme  = %d\n", somme);
                Rio_writen(clientfd,buf,somme);
                fin=clock();
                Close(fd); // on a atteint la fin de fichier donc on le ferme
                printf("Transfer successfully complete.\n");
                temps=(double)(fin-debut)*1000/CLOCKS_PER_SEC;
                kilo_bits_par_sec=somme;
                kilo_bits_par_sec/=100;
                if(temps!=0.0){kilo_bits_par_sec/=temps;}
                printf("%d bytes received in %f seconds\n(%f Kbits/sec)\n",somme,temps,kilo_bits_par_sec);
                exit(0);
            }
        }
        else if(!strcmp(cmd,"cat")){ // Code pour la commande cat
            printf("ok\n");
            exit(0);
        }
        else if(!strcmp(cmd,"ls")){
            //printf("Commande ls\n");
            Rio_readlineb(&rio,&buf,MAXLINE);
            printf("%s",buf);
        }
        else { /* the server has prematurely closed the connection */
            exit(0);
            //break;
        }
        printf("ftp> ");
    }
    Close(clientfd);
    exit(0);
}
