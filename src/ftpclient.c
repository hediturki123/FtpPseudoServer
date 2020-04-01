/*
 * echoclient.c - An echo client
 */
#include "csapp.h"
#include <time.h>
#define TAILLE_BUFFER 256

void get_cmd(char buf[],char cmd[]){
    int i;
    for(i=0;buf[i]!=' ' && buf[i] != '\n';i++)cmd[i]=buf[i];
    if(buf[i+1]=='-' && buf[i+2]=='r'){
        strcat(cmd," -r");
    }
}

void get_fichier(char buf[],char fichier[]){
    int i,j;
    for(i=0;buf[i]!=' ';i++){}
    i++;
    for(j=i;buf[j]!='\0';j++){
        fichier[j-i]=buf[j];
    }
    fichier[j]='\0';
}

void rm_bsn(char fichier[]){
    int i;
    for(i=0;fichier[i]!='\n' && fichier[i]!='\0';i++){}
    fichier[i]='\0';
}

void stat_transfere(clock_t debut,clock_t fin,int somme){
    double temps=(double)(fin-debut)*1000/CLOCKS_PER_SEC;
    double kilo_bits_par_sec=somme;
    kilo_bits_par_sec/=100;
    if(temps!=0.0){kilo_bits_par_sec/=temps;}
    printf("%d bytes received in %f seconds\n(%f Kbits/sec)\n",somme,temps,kilo_bits_par_sec);
}

int rempplit_fichier(rio_t rio,char fichier[]){
    int somme=0;
    int fd;
    ssize_t n;
    char buf[MAXBUF];
    fd=open(fichier,O_CREAT | O_WRONLY,0666);
    while((n=Rio_readnb(&rio, &buf, TAILLE_BUFFER)) > 0){
        printf("n = %ld\n", n);
        somme += n;
        write(fd,buf,n);
    }
    close(fd);
    return somme;
}

void envoi_fichier(rio_t rio,int clientfd,char fichier[],char buf[]){
    int fd,n;
    int donnee[TAILLE_BUFFER];
    rm_bsn(fichier);
    fd=open(fichier,O_RDONLY);
    if(fd<0){
        printf("Erreur de fichier\n");
    }
    else{
        Rio_writen(clientfd, buf, MAXBUF);
        memset(buf,0,MAXBUF);
        Rio_readlineb(&rio,buf,MAXBUF);
        printf("%s",buf);
        if(!strcmp(buf,"Création du fichier ok\n")){
            while((n=Read(fd,donnee,TAILLE_BUFFER))>0){
                Rio_writen(clientfd,donnee,n);
            }
        }
        else{
            printf("Erreur lors de la création du fichier\n");
        }
    }
    close(fd);
}
void create_mkdir(rio_t rio,int clientfd,char buf[]){
    Rio_writen(clientfd, buf, strlen(buf));
    Rio_readlineb(&rio,buf,MAXBUF);
    printf("%s",buf);
}

void supp_fich(rio_t rio,int clientfd,char buf[]){
    Rio_writen(clientfd, buf, MAXBUF);
    memset(buf,0,MAXBUF);
    Rio_readlineb(&rio,buf,MAXBUF);
    printf("%s",buf);
}
int main(int argc, char **argv)
{
    int clientfd, port;
    char *host, fichier[MAXBUF],buf[MAXBUF],cmd[10];
    rio_t rio;
    clock_t debut,fin;
    struct sockaddr_in clientaddr;
    socklen_t clientlen = sizeof(clientaddr);
    #ifndef DEBUG
    if (argc != 2) {
        fprintf(stderr, "usage: %s <host>\n", argv[0]);
        exit(0);
    }
    host = argv[1];
    #else
    host = "127.0.0.1";
    #endif
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
    while(1){
    printf("ftp> ");
    int somme = 0;

    if (Fgets(buf, MAXLINE, stdin) != NULL) {
        get_cmd(buf,cmd);
        if(!strcmp(cmd,"get")){ // Code pour la commande get
            Rio_writen(clientfd, buf, strlen(buf));
            debut=clock();
            if (Rio_readlineb(&rio, &fichier, MAXBUF) > 0) {
                rm_bsn(fichier);
                printf("Nom du fichier en réception : %s\n",fichier);
                somme=rempplit_fichier(rio,fichier);
                printf("aaaa\n");
                Rio_writen(clientfd,buf,somme);
                printf("bbb\n");
                fin=clock();
                printf("Transfer successfully complete.\n");
                stat_transfere(debut,fin,somme);
            }
        }
        else if(!strcmp(cmd,"put")){
            get_fichier(buf,fichier);
            printf("Début du transfère du fichier : %s",fichier);
            envoi_fichier(rio,clientfd,fichier,buf);
            printf("Fin du transfère\n");

        }
        else if(!strcmp(cmd,"cat")){
            Rio_writen(clientfd, buf, strlen(buf));
            memset(buf,0,MAXBUF);
            while(Rio_readnb(&rio,buf,MAXBUF)){
                Fputs(buf,stdout);
            }
        }
        else if(!strcmp(cmd,"ls")){
            int n;
            Rio_writen(clientfd,buf,strlen(buf));
            if((n =Rio_readlineb(&rio,&buf,MAXBUF) )!= 0){               
               printf("%s",buf);

               fflush(stdout);
            }
            buf[strlen(buf)] = '\n';
        }
        else if(!strcmp(cmd,"mkdir")){create_mkdir(rio,clientfd,buf);}
        else if(!strcmp(cmd,"rm")){supp_fich(rio,clientfd,buf);}
        else if(!strcmp(cmd,"bye")){
            printf("Fin de la connection\n");
            exit(0);
        }
	    else if(!strcmp(cmd,"cd")){
            Rio_writen(clientfd, buf, strlen(buf));
            printf("changement de repertoire\n");
            memset(buf,0,MAXBUF);
            Rio_readlineb(&rio,&buf,MAXLINE);
        }
        else if(!strcmp(cmd,"rm -r")){
            Rio_writen(clientfd, buf, strlen(buf));
            if(Rio_readlineb(&rio,&buf,MAXLINE)>0){
                printf("%s",buf);
            }
        }
        else if (!strcmp(cmd,"pwd")){
            Rio_writen(clientfd,buf,strlen(buf));
            memset(buf,0,MAXBUF);
            Rio_readlineb(&rio,&buf,MAXLINE);
            printf("%s",buf);
        }
        else { /* the server has prematurely closed the connection */
            printf("entrez une commande valide!\n");
            exit(0);

        }
    memset(cmd,0,MAXBUF);
    memset(buf,0,MAXBUF);
    }
    }
    Close(clientfd);
    exit(0);
}
