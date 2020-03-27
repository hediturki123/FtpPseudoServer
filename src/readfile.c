#include "csapp.h"
#define TAILLE_BUFFER 256
void nom_fichier(char *buf,char *nom){
    int i;
    for(i=0;buf[i]!='\n' && buf[i]!='\0';i++){
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
        strcat(fichier,"\n");
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

void recup_fichier(char fichier[],int connfd,rio_t rio){
    int fd,n;
    char buf[MAXBUF];
    char message[MAXBUF];
    nom_fichier(fichier,buf);
    fd=open(buf,O_CREAT | O_WRONLY,0666);
    if(fd<0){
        strcpy(message,"Erreur de fichier\n");
        Rio_writen(connfd,message,strlen(message));
    }
    else{
        strcpy(message,"Création du fichier ok\n");
        Rio_writen(connfd,message,strlen(message));
        while((n=Rio_readnb(&rio,buf,TAILLE_BUFFER))>0){
            write(fd,buf,n);
        }
    }
    close(fd);
}

void affiche_rep(int connfd){ 
    FILE *fp;
    char buf[MAXLINE];
    if((fp=popen("ls","r"))==NULL){printf("---erreur");}
    fscanf(fp,"%s",buf);
    printf("%s",buf);
    pclose(fp);
}

void creation_repertoire(char fichier[],int connfd){
    char message[MAXBUF];
    char buf[MAXBUF];
    nom_fichier(fichier,buf);
    if(mkdir(buf,S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH)<0){
        strcpy(message,"Erreur lors de la création du repertoir\n");
        Rio_writen(connfd,message,strlen(message));
    }
    else{
        strcpy(message,"Repertoir créé\n");
        Rio_writen(connfd,message,strlen(message));
    }
}

void remove_file(char fichier[],int connfd){
    char message[MAXBUF];
    char buf[MAXBUF];
    nom_fichier(fichier,buf);
    if(remove(fichier)==0){
        strcpy(message,"Fichier supprimé\n");
        Rio_writen(connfd,message,strlen(message));
    }
    else{
        strcpy(message,"Fichier supprimé\n");
        Rio_writen(connfd,message,strlen(message));
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
        else if(!strcmp(commande,"ls")){affiche_rep(connfd);}
        else if(!strcmp(commande,"put")){recup_fichier(fichier,connfd,rio);}
        else if(!strcmp(commande,"mkdir")){creation_repertoire(fichier,connfd);}
        else if(!strcmp(commande,"rm")){remove_file(fichier,connfd);}
        //printf("server received %u bytes\n", (unsigned int)n);
        //printf("%s", buf);
        
    }

}

