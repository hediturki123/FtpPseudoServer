#include "csapp.h"
#define TAILLE_BUFFER 256
void nom_fichier(char *buf,char *nom){
    int i;
    for(i=0;buf[i]!='\n';i++){
        nom[i]=buf[i];
    }
    nom[i]='\0';
}

void lecture_fichier(char buf[],int connfd){
    int fd,n;
    char Rbuf[MAXLINE];
    char nom[MAXLINE];
    nom_fichier(buf,nom);
        
    fd=Open(nom,O_RDONLY,S_IRUSR);
    if(fd<0){printf("Erreur lors de l'ouverture du fichier\n");}
        
    while((n=Read(fd,&Rbuf,MAXLINE))!=0){
        Rio_writen(connfd, Rbuf, n);
        exit(0);
    }
}

void transfere_fichier(char fichier[],int connfd){
    int fd,n;
    char buf[MAXLINE];
    char message[MAXLINE];
    fd=open(fichier,O_RDONLY,0);
    if(fd<0){
        strcpy(message,"Erreur de fichier\n");
        Rio_writen(connfd,message,strlen(message));
    }
    else{
        int nbre_de_paquets = 0;
        Rio_writen(connfd,fichier,strlen(fichier));
        while((n=Rio_readn(fd,buf,TAILLE_BUFFER))!=0 ){
            Rio_writen(connfd,buf,n); 
            printf("taille du buffer = %d\n",n);
            nbre_de_paquets++;
            //printf("n = %d, taille_buffer = %d\n", n,TAILLE_BUFFER);

        }
        printf("nbre de paquets = %d",nbre_de_paquets);
        Close(fd);
    }
    exit(0);
}

void decoupe(char commande[],char fichier[],char buf[]){
    int sauve_i,espace=0;
    for(int i=0;buf[i]!='\0' && buf[i]!='\n';i++){
        if(buf[i]==' '){espace=1;sauve_i=i+1;}
        else{
            switch (espace){
                case 0:
                    commande[i]=buf[i];
                    break;
                case 1:
                    fichier[i-sauve_i]=buf[i];
                    break;
            }
        }
    }
}

void demande_client(int connfd)
{
    size_t n;
    char buf[MAXLINE];
    char commande[10];
    char fichier[MAXLINE];
    rio_t rio;

    Rio_readinitb(&rio, connfd);
    while ((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0) {
        decoupe(commande,fichier,buf);
        if(!strcmp(commande,"get")){transfere_fichier(fichier,connfd);}
        else if(!strcmp(commande,"cat")){lecture_fichier(fichier,connfd);}
        
        //printf("server received %u bytes\n", (unsigned int)n);
        //printf("%s", buf);
        
    }

}

