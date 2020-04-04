#include "csapp.h"
#include <dirent.h>
#define _GNU_SOURCE

#define TAILLE_BUFFER 10
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
    }
    Close(fd);
}

int transfere_fichier(char fichier[], int connfd){
    int fd;
    size_t n;
    char buf[MAXLINE];
    char message[MAXLINE];
    char taille[4];
    char taille_paquets[100];
    int nbre_de_paquets = 0;
    rio_t rio;
    fd = open(fichier, O_RDONLY, 0);
    if(fd < 0){
        strcpy(message, "Erreur de fichier\n");
        Rio_writen(connfd, message, strlen(message));
    
    } else {

        strcat(fichier, "\n");
        Rio_writen(connfd, fichier, strlen(fichier));
        Rio_readinitb(&rio, fd);

        while((n = Rio_readnb(&rio, buf, TAILLE_BUFFER)) > 0){
            sprintf(taille, "%ld", n);
            Rio_writen(connfd, taille, 4);
            Rio_writen(connfd, buf, n); 
            memset(buf, 0, sizeof(buf));
            memset(taille, 0, sizeof(taille));
            nbre_de_paquets++;
            //printf("nbre de paquets = %d\n", nbre_de_paquets);
        }
        Rio_writen(connfd, "0", 4);
        sprintf(taille_paquets,"%d",nbre_de_paquets);
        strcat(taille_paquets,"\n");
        Rio_writen(connfd, taille_paquets,strlen(taille_paquets));
        //Rio_readnb(&rio,buf,strlen(buf));
        //printf("buf = %s\n", buf);
        memset(buf, 0, TAILLE_BUFFER);
        Close(fd);
    }
    return nbre_de_paquets;
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


void crash_et_reprise(int connfd,rio_t crio){
    int fd;
    size_t n;
    char buf[MAXBUF];
    char message[MAXLINE];
    char taille[4];
    char nb_paquets[MAXLINE];
    char fichier[MAXBUF];
    int paquet;
    //char taille_paquets[100];
    rio_t rio;
    
    Rio_readlineb(&crio,buf,MAXBUF);
    printf("buf = %s\n", buf);
    
    decoupe(fichier, nb_paquets, buf);
    paquet = atoi(nb_paquets);
    printf("paquet = %d\n", paquet);
    printf("fichier = %s\n", fichier);
    strcat(fichier, "\n");
    Rio_writen(connfd, fichier, MAXBUF);
    //printf("nb = %s\n", nb_paquets);

    fichier[strlen(fichier)-1] = '\0';
    
    fd = open(fichier, O_RDONLY, 0);    
    
    
    if(fd < 0){
        strcpy(message, "Erreur de fichier\n");
        Rio_writen(connfd, message, strlen(message));
    
    } else {

        Rio_readinitb(&rio, fd);
        int nbre_p = transfere_fichier(fichier, connfd);
        // on recupere le nombre de paquets du fichier
        printf("nbre p : %d\n", nbre_p);

        if (paquet != (nbre_p/TAILLE_BUFFER)){
            Lseek(fd,paquet*TAILLE_BUFFER,SEEK_SET);
        }
        //rintf("nbre : %d\n", paquet*TAILLE_BUFFER);
        //memset(buf, 0, sizeof(buf));
        //memset(fichier, 0, 100);
        //memset(taille, 0, TAILLE_BUFFER);

        while((n = Rio_readnb(&rio, buf, TAILLE_BUFFER)) > 0){
            
            sprintf(taille, "%ld", n);
            Rio_writen(connfd, taille, 4);
            Rio_writen(connfd, buf, n); 
            printf("buffer : %s\n", buf);
        }
        Rio_writen(connfd, "0", 4);
    printf("aaaaaaaaaa\n");
        memset(buf, 0, TAILLE_BUFFER);
        Close(fd);
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

void affiche_rep(int connfd, char fichier[MAXBUF]){ 
    
    struct dirent *dir;
    char nom[MAXBUF];
    DIR *d = opendir("."); 
    if (d){
        while ((dir = readdir(d)) != NULL){
            if (!strcmp(dir->d_name,".") || !strcmp(dir->d_name, "..") || !strcmp(dir->d_name, ".security")){}
            else {
                strcpy(nom,dir->d_name);
                strcat(nom, " ");
                Rio_writen(connfd,nom,strlen(nom));
            }
        }
        strcpy(nom, "\n");
        Rio_writen(connfd,nom,strlen(nom));
        closedir(d);
    }
}

void creation_repertoire(char fichier[],int connfd){
    char message[MAXBUF];
    char buf[MAXBUF];
    nom_fichier(fichier,buf);
    if(mkdir(buf,S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)<0){
        strcpy(message,"Erreur lors de la création du repertoire\n");
    }
    else{
        strcpy(message,"Repertoire créé\n");
    }
    Rio_writen(connfd,message,strlen(message));
}

void remove_file(char fichier[],int connfd){
    char message[MAXBUF];
    char buf[MAXBUF];
    nom_fichier(fichier,buf);
    if(remove(buf)==0){
        strcpy(message,"Fichier supprimé\n");
    } else {
        strcpy(message,"Erreur\n");
    }
    Rio_writen(connfd,message,strlen(message));
}

void change_directory(int connfd,char fichier[MAXBUF]){
    char nom[MAXBUF];
    char message[MAXBUF];
    nom_fichier(fichier,nom);
    if (chdir(nom) != 0){
        strcpy(message,"Aucun fichier dans ce repertoire\n");
    } else {
        strcpy(message,"ok\n");
    }
    Rio_writen(connfd,message,strlen(message));
}

int remove_rec(char fichier[], int connfd){
    int n = 0;
    struct dirent *dir;
    char buf[MAXBUF];
    DIR *d = opendir(fichier);
    chdir(fichier);
    
    if (d){
        while ((dir = readdir(d)) != NULL){
            
            if (!strcmp(dir->d_name, ".") || !strcmp(dir->d_name, "..")){}
            
            else {
                if (dir->d_type == DT_DIR){
                    n += remove_rec(dir->d_name, connfd);
                
                } else {
                    strcpy(buf, dir->d_name);
                    if(remove(buf) != 0){n++;}
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
    
    } else {
        strcpy(message,"Le répertoire n'a pas pu être supprimé\n");
    
    }
    Rio_writen(connfd,message,strlen(message));
}

void chemin(int connfd){
    char cwd[MAXBUF];
    getcwd(cwd, sizeof(cwd));
    cwd[strlen(cwd)]='\n';
    Rio_writen(connfd,cwd,strlen(cwd));
}


int demande_client(int connfd)
{
    size_t n;
    char buf[MAXLINE];
    char commande[10];
    char fichier[MAXLINE];
    rio_t rio;

    Rio_readinitb(&rio, connfd);
    while(1){
        if ((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0) {
            decoupe(commande,fichier,buf);
            if(!strcmp(commande, "get")){
                transfere_fichier(fichier,connfd);
            }

            else if(!strcmp(commande, "resume")){
                crash_et_reprise(connfd,rio);
            }

            else if(!strcmp(commande, "cat")){
                lecture_fichier(fichier,connfd);
            }
            
            else if(!strcmp(commande, "ls")){
                affiche_rep(connfd, fichier);
            }

            else if(!strcmp(commande, "put")){
                recup_fichier(fichier, connfd, rio);
            }

            else if(!strcmp(commande, "mkdir")){
                creation_repertoire(fichier, connfd);
            }

            else if(!strcmp(commande, "cd")){
                change_directory(connfd, fichier);
            }

            else if(!strcmp(commande, "rm")){
                remove_file(fichier, connfd);
            }

            else if(!strcmp(commande, "rm -r")){
                remove_folder(fichier, connfd);
            }

            else if(!strcmp(commande, "pwd")){
                chemin(connfd);
            }

            else if(!strcmp(commande, "bye")){
                return 1;
            }
        }
    memset(buf, 0, MAXLINE);
    memset(fichier, 0, MAXLINE);
    memset(commande, 0, 10);
    }
    return 0;
}
