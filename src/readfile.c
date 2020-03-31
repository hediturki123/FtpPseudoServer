#include "csapp.h"
#include <dirent.h>
#define _GNU_SOURCE

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
    char Rbuf[TAILLE_BUFFER];
    char nom[TAILLE_BUFFER];
    nom_fichier(buf,nom);
        
    fd=Open(nom,O_RDONLY,S_IRUSR);
    if(fd<0){printf("Erreur lors de l'ouverture du fichier\n");}
        
    while((n=Read(fd,&Rbuf,TAILLE_BUFFER))!=0){
        Rio_writen(connfd, Rbuf, n);
        exit(0);
    }
}

void transfere_fichier(char fichier[],int connfd){
    int fd,n;
    char buf[TAILLE_BUFFER];
    char message[TAILLE_BUFFER];
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
        if(buf[i]==' '){
            if(buf[i+1]=='-' && buf[i+2]=='r'){
                espace=1;
                sauve_i=i+4;
                strcat(commande," -r");
            }
            else{
                espace=1;
                sauve_i=i+1;
            }
        }
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
    
    struct dirent *dir;
    char nom[TAILLE_BUFFER];
    DIR *d = opendir("."); 
    if (d){
        while ((dir = readdir(d)) != NULL){
            if (!strcmp(dir->d_name,".") || !strcmp(dir->d_name, "..")){}
            else {
                strcpy(nom,dir->d_name);
                strcat(nom, " ");
                Rio_writen(connfd,nom,strlen(nom));
            }
        }
        closedir(d);
    }
}

void creation_repertoire(char fichier[],int connfd){
    char message[MAXBUF];
    char buf[MAXBUF];
    nom_fichier(fichier,buf);
    if(mkdir(buf,S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH)<0){
        strcpy(message,"Erreur lors de la création du repertoire\n");
        Rio_writen(connfd,message,strlen(message));
    }
    else{
        strcpy(message,"Repertoire créé\n");
        Rio_writen(connfd,message,strlen(message));
    }
}

void remove_file(char fichier[],int connfd){
    char message[MAXBUF];
    char buf[MAXBUF];
    nom_fichier(fichier,buf);
    if(remove(buf)==0){
        strcpy(message,"Fichier supprimé\n");
        Rio_writen(connfd,message,strlen(message));
    }
    else{
        strcpy(message,"Fichier supprimé\n");
        Rio_writen(connfd,message,strlen(message));
    }
}
void change_directory(int connfd,char fichier[MAXBUF]){
    char nom[MAXBUF];
    char message[MAXBUF];
    nom_fichier(fichier,nom);
    if (chdir(nom) == 0){
        strcpy(message,"ok\n");
        Rio_writen(connfd,message,strlen(message));
    }
}

int remove_rec(char fichier[],int connfd){
    int n=0;
    struct dirent *dir;
    char buf[MAXBUF];
    DIR *d = opendir(fichier);
    chdir(fichier);
    if (d){
        while ((dir = readdir(d)) != NULL){
            //printf("%s\n", dir->d_name);
            if (!strcmp(dir->d_name,".") || !strcmp(dir->d_name, "..")){}
            else {
                if (dir->d_type == DT_DIR){
                    n+=remove_rec(dir->d_name,connfd);
                }
                else{
                    strcpy(buf,dir->d_name);
                    if(remove(buf)!=0){n++;}
                }
            }
        }
        chdir("..");
        closedir(d);
        rmdir(fichier);
    }
    return n;
}

void remove_folder(char fichier[],int connfd){
    char message[MAXBUF];
    if(remove_rec(fichier,connfd)==0){
        strcpy(message,"Le répertoire a été supprimé\n");
        Rio_writen(connfd,message,strlen(message));
    }
    else{
        strcpy(message,"Le répertoire n'a pas pu être supprimé\n");
        Rio_writen(connfd,message,strlen(message));
    }
}
void chemin(int connfd){
    char cwd[MAXBUF];
    getcwd(cwd, sizeof(cwd));
    Rio_writen(connfd,cwd,strlen(cwd));
}



int demande_client(int connfd)
{
    size_t n;
    char buf[MAXBUF];
    char commande[10];
    char fichier[TAILLE_BUFFER];
    rio_t rio;

    Rio_readinitb(&rio, connfd);
    while(1){
        if ((n = Rio_readnb(&rio, buf, TAILLE_BUFFER)) != 0) {

          
            decoupe(commande,fichier,buf);
            if(!strncmp(commande,"get",3)){
                transfere_fichier(fichier,connfd);
            }

            else if(!strncmp(commande,"cat",3)){
                lecture_fichier(fichier,connfd);
            }

            else if(!strncmp(buf,"ls",2)){
                affiche_rep(connfd);
            }

            else if(!strncmp(commande,"put",3)){
                recup_fichier(fichier,connfd,rio);
            }

            else if(!strncmp(commande,"mkdir",5)){
                creation_repertoire(fichier,connfd);
            }

            else if(!strncmp(commande,"cd",2)){
                change_directory(connfd,fichier);
            }

            else if(!strncmp(commande,"rm",2)){
                remove_file(fichier,connfd);
            }

            else if(!strncmp(commande,"rm -r",5)){
                remove_folder(fichier,connfd);
            }

            else if(!strncmp(commande,"pwd",3)){
                chemin(connfd);
            }

            else if(!strncmp(commande,"bye",3)){
                return 1;
            }            
        }
        return 0;
    }
    return 0;

}


