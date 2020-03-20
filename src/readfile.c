/*
 * echo - read and echo text lines until client closes connection
 */
#include "csapp.h"
void nom_fichier(char *buf,char *nom){
    int i;
    for(i=0;buf[i]!='\n';i++){
        nom[i]=buf[i];
    }
    nom[i]='\0';
}

void readfile(int connfd)
{
    size_t n;
    char buf[MAXLINE];
    char Rbuf[MAXLINE];
    char nom[MAXLINE];
    rio_t rio;
    int fd;

    Rio_readinitb(&rio, connfd);
    while ((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0) {
        //printf("server received %u bytes\n", (unsigned int)n);
        //printf("%s", buf);
        nom_fichier(buf,nom);
        
        fd=Open(nom,O_RDONLY,S_IRUSR);
        if(fd<0){printf("Erreur lors de l'ouverture du fichier\n");}
        
        while((n=Read(fd,&Rbuf,MAXLINE))!=0){
            Rio_writen(connfd, Rbuf, n);
            //Fputs(Rbuf,stdout);
        }
    }
}

