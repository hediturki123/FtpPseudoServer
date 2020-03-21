/*
 * echoclient.c - An echo client
 */
#include "csapp.h"

int main(int argc, char **argv)
{
    int clientfd, port;
    char *host, buf[MAXLINE];
    rio_t rio;
    struct sockaddr_in clientaddr;
    socklen_t clientlen = sizeof(clientaddr);

    if (argc != 2) {
        fprintf(stderr, "usage: %s <host> <port>\n", argv[0]);
        exit(0);
    }
    host = argv[1];
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
    printf("ftp>");
    while (Fgets(buf, MAXLINE, stdin) != NULL) {
        Rio_writen(clientfd, buf, strlen(buf));
        if (Rio_readlineb(&rio, buf, MAXLINE) > 0) {
            Fputs(buf, stdout);
            exit(0);
        } else { /* the server has prematurely closed the connection */
            break;
        }
    }
    Close(clientfd);
    exit(0);
}
