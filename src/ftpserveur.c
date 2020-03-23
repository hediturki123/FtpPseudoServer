/*
 * echoserveri.c - An iterative echo server
 */

#include "csapp.h"

#define MAX_NAME_LEN 256
#define NPROC 2
pid_t nb_fils[NPROC];

void demande_client(int connfd);



/* 
 * Note that this code only works with IPv4 addresses
 * (IPv6 is not supported)
 */
void handler(int sig){
    for(int i=0;i<NPROC;i++){
        Kill(nb_fils[i],SIGKILL);
    }
    exit(0);
}



int main(int argc, char **argv)
{
    int listenfd, connfd, port;
    socklen_t clientlen;
    struct sockaddr_in clientaddr;
    char client_ip_string[INET_ADDRSTRLEN];
    char client_hostname[MAX_NAME_LEN];
    pid_t pid;
    Signal(SIGINT,handler);

/*
    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(0);
    }*/
    port = 2121;
    
    clientlen = (socklen_t)sizeof(clientaddr);

    listenfd = Open_listenfd(port);
    while (1) {

        for (int i=0;i<NPROC;i++){
            
            pid=fork();
            if (pid==0){
                connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
                getsockname(connfd, (SA *) &clientaddr, &clientlen);
                printf("numero de port distant : %d\n", ntohs(clientaddr.sin_port));
            /* determine the name of the client */
                Getnameinfo((SA *) &clientaddr, clientlen,
                            client_hostname, MAX_NAME_LEN, 0, 0, 0);
                
                /* determine the textual representation of the client's IP address */
                Inet_ntop(AF_INET, &clientaddr.sin_addr, client_ip_string,
                        INET_ADDRSTRLEN);
                
                printf("server connected to %s (%s)\n", client_hostname,
                    client_ip_string);

                demande_client(connfd);
                Close(connfd);
            
            }
            
        }
        if(waitpid(pid,NULL,0)==-1){
                printf("error\n");
        }


    }
    exit(0);
}

