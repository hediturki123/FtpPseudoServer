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

void decoupe_secu(char buf[],char log[],char mdp[]){
    int space=0;
    int i,j;
    for(i=0;buf[i]!='\n';i++){
        if(buf[i]==' '){
            log[i]='\0';
            space=1;
            i++;
            j=i;
        }
        if(space){
            mdp[i-j]=buf[i];
        }
        else{
            log[i]=buf[i];
        }
    }
    mdp[i-j]='\0';
}

int secutrity_serv(int connfd){
    
    int nb_essai=0;
    char login_USR[MAXBUF];
    char mdp_USR[MAXBUF];
    char log_fich[MAXBUF];
    char mdp_file[MAXBUF];
    char buf[MAXBUF];
    rio_t rio,fich;
    int fd=open(".security",O_RDONLY);
    Rio_readinitb(&rio, connfd);
    Rio_readlineb(&rio,login_USR,MAXBUF);
    Rio_readinitb(&fich,fd);
    login_USR[strlen(login_USR)-1]='\0';
    while((Rio_readlineb(&fich,buf,MAXBUF)!=0)){
            //Fputs(buf,stdout);
            decoupe_secu(buf,log_fich,mdp_file);
            if((strcmp(login_USR,log_fich)==0)){
                break;
            }
            memset(mdp_file,0,MAXBUF);
            memset(log_fich,0,MAXBUF);
            memset(buf,0,MAXBUF);
        }
    Close(fd);
    Fputs(mdp_file,stdout);
    while(nb_essai<ESSAI_MAX){
        Rio_readlineb(&rio,mdp_USR,MAXLINE);
        mdp_USR[strlen(mdp_USR)-1]='\0';
        
        if(strcmp(mdp_USR,mdp_file)==0){
            Rio_writen(connfd,"0\n",strlen("0\n"));
            return 1;
        }
        else{
            if(nb_essai<ESSAI_MAX-1){
                Rio_writen(connfd,"1\n",strlen("1\n"));
            }
            else{
                Rio_writen(connfd,"2\n",strlen("2\n"));
                break;
            }
        }
        nb_essai++;
        memset(mdp_USR,0,MAXBUF);
    }
    
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

     for (int i=0;i<NPROC;i++){
         if  ((pid=fork() ) != 0){
                break;
         }
     }
    while (1) {
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
        
    
    exit(0);
}

