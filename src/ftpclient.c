/*
 * echoclient.c - An echo client
 */
#include "csapp.h"
#include <time.h>

int main(int argc, char **argv)
{
    int clientfd, port;
    char *host, buf[MAXLINE];
    rio_t rio;
    double temps,kilo_bits_par_sec;
    clock_t debut,fin;
    struct sockaddr_in clientaddr;
    ssize_t nb,b=0;
    socklen_t clientlen = sizeof(clientaddr);

    if (argc != 2) {
        fprintf(stderr, "usage: %s <host> <port>\n", argv[0]);
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
    printf("ftp>");
    while (Fgets(buf, MAXLINE, stdin) != NULL) {
        Rio_writen(clientfd, buf, strlen(buf));
        debut=clock();
        if (Rio_readlineb(&rio, &buf, strlen(buf)-4) > 0) {
            printf("Nom du fichier en réception : %s\n",buf);        
            fd=open(buf,O_CREAT | O_WRONLY,0666);
            while(Rio_readlineb(&rio, buf, MAXLINE) > 0){
                if(strcmp(buf,"EOF\n")==0){break;}
                b=strlen(buf);
                write(fd,buf,b);
                nb+=b;    
            }
            fin=clock();
            Close(fd); // on a atteint la fin de fichier donc on le ferme
            printf("Transfer successfully complete.\n");
            temps=(double)(fin-debut)*1000/CLOCKS_PER_SEC;
            kilo_bits_par_sec=nb;
            kilo_bits_par_sec/=100;
            if(temps!=0.0){kilo_bits_par_sec/=temps;}
            printf("%ld bites reçu en %f secondes\n (%f Kbits/sec)",b,temps,kilo_bits_par_sec);
            exit(0);
        } else { /* the server has prematurely closed the connection */
            exit(0);
            //break;
        }
    }
    Close(clientfd);
    exit(0);
}
