/*
 * echoserveri.c - An iterative echo server
 */

#include "csapp.h"

#define MAX_NAME_LEN 256
#define NPROC 2
#define ESSAI_MAX 3
pid_t nb_fils[NPROC];

int demande_client(int connfd);



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

void mdp_login(char log[],char mdp_file[]){
    char log_fich[MAXBUF];
    char buf[1];
    int space=0;
    strcat(log_fich,".");
    int fd=open(".security",O_RDONLY);
    Fputs("dans la fonciton",stdout);
    while(strcmp(log,log_fich)!=0){
        while(buf[0]!='\n'){
            Read(fd,buf,strlen(buf));
            if(strcmp(buf," ")==0){space=1;}
            if(space){
                strcat(mdp_file,buf);
            }
            else{
                strcat(log_fich,buf);
            }
        }
    }
    Close(fd);
    Fputs(log_fich,stdout);
    Fputs(mdp_file,stdout);
}

int secutrity_serv(int connfd){
    FILE * f=fopen("log","w+");
    int nb_essai=0;
    char login_USR[MAXBUF];
    char mdp_USR[MAXBUF];
    //char mdp_in_file[MAXBUF];
    rio_t rio;
    Rio_readinitb(&rio, connfd);
    Rio_readlineb(&rio,login_USR,MAXBUF);
    //mdp_login(login_USR,mdp_in_file);
    while(nb_essai<ESSAI_MAX){
        Rio_readlineb(&rio,mdp_USR,MAXLINE);
        mdp_USR[strlen(mdp_USR)-1]='\0';
        fprintf(f,"%s",mdp_USR);
        if(strcmp(mdp_USR,"root")==0){
            Rio_writen(connfd,"0\n",strlen("0\n"));
            return 1;
        }
        else{
            if(nb_essai<ESSAI_MAX-1){
                Rio_writen(connfd,"1\n",strlen("1\n"));
                Fputs("envoi ok\n",stdout);
            }
            else{
                Rio_writen(connfd,"2\n",strlen("2\n"));
                break;
            }
        }
        nb_essai++;
        memset(mdp_USR,0,MAXBUF);
    }
    fclose(f);
    return 0;
}
int main(int argc, char **argv)
{
    int listenfd, connfd, port;
    socklen_t clientlen;
    int autorisation;
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
    pid=fork();

    while (1) {

        for (int i=0;i<NPROC;i++){

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
                #ifdef SECU
                    autorisation=secutrity_serv(connfd);
                #else
                    autorisation=1;
                #endif
                if(autorisation){demande_client(connfd);}
                Close(connfd);
            
            }
        }
    }
    exit(0);
}

