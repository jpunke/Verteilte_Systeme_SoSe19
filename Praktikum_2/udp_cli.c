/*
 Beispiel: Einfacher UDP Echo-Client 
 Stevens, R., Fenner, B., Rudoff, A. M.: Unix Network Programming 
 getestet unter Ubuntu 10.04 32Bit
*/

#include <unistd.h>    // fuer read, write etc.
#include <stdlib.h>     // fuer exit
#include <stdio.h>
#include <string.h> 
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SRV_PORT 8998
#define MAXLINE 512

void dg_client(int, struct sockaddr*, int, char **);
void err_abort(char *str);


int main(int argc, char *argv[]) {
	// Deskriptor
	int sockfd;
	// Socket Adresse
	struct sockaddr_in srv_addr, cli_addr;
	char msg [MAXLINE] = "hallo\0";

	// Argumente testen
	if( argc != 4 ) {
		err_abort("Syntaxfehler! Geben Sie die Parameter wie folgt an: \n Adresse des Servers   Chunkgroesse   Dateiname ");
	}

	// UDP Socket erzeugen
	if( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		err_abort("Kann Stream-Socket nicht oeffnen!");
	}

	// lokale Adresse binden
	memset((void *)&cli_addr, '\0', sizeof(cli_addr));
	cli_addr.sin_family = AF_INET;
	cli_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if(bind(sockfd, (struct sockaddr *)&cli_addr, sizeof(cli_addr))<0){
		err_abort("Fehler beim Binden der lokalen Adresse!");
	}

	// Adress Struktur fuer Server aufbauen
	memset((void *)&srv_addr, '\0', sizeof(srv_addr));
	srv_addr.sin_family = AF_INET;
	srv_addr.sin_addr.s_addr = inet_addr(argv[1]);
	srv_addr.sin_port = htons(SRV_PORT);

	printf("UDP Echo-Client: bereit...\n");


	// Daten zum Server senden
	dg_client(sockfd,(struct sockaddr *)&srv_addr, sizeof(srv_addr), argv);

	close(sockfd);
	exit(0);
}

/*
 str_client: Daten von der Standardeingabe lesen, zum Server senden, auf das Echo warten und dieses ausgeben
*/
void dg_client(int sockfd, struct sockaddr *srv_addr, int srv_len, char **argv){
	int n;
	char rspnd[MAXLINE+6];
	char init[14] = "HSOSSTP_INITX;";
	char sid[15] = "HSOSSTP_SIDXX;";
	char filename[256];
	char chunksize[256];
	char out[MAXLINE],in[MAXLINE+6];
	char *tok;
	int chunkno =0;
	int key = -1;
	int check = 1;
	
	
	//Create INIT-Message
	strcpy(out, init);
	
	sprintf(chunksize, "<%d>;",(atoi(argv[2])));
	strcat(out,chunksize);
	
	sprintf(filename,"<%s>",(argv[3]));
	strcat(out, filename);

	n = strlen(out);
	//out[n]='\0';
	
	printf("out: %s\n",out);
	
	// INIT-Message an Server senden
	if(sendto(sockfd,out,n,0,srv_addr,srv_len)!=n){
		err_abort("Fehler beim Schreiben des Sockets!");
	}
	
	// Antwort von Server auf INIT
	n=recvfrom(sockfd,in,MAXLINE,0,(struct sockaddr *)NULL,(int *)NULL);
	if(n<0){
		err_abort("Fehler beim Lesen des Sockets!");
	} 
	printf("in: %s\n",in);
	
	tok = strtok(in, ";");
	
	//Erstellen der GET Nachricht und Senden
	if(strncmp(tok,"HSOSSTP_SIDXX",13)==0){
		
		tok = strtok(NULL, ";");
		tok++;
		tok[strlen(tok)-1]='\0';
		key = atoi(tok);
		sprintf(out,"HSOSSTP_GETXX;<%d>;<%d>",key,chunkno);
		
		printf("out: %s\n",out);
		
		n = strlen(out);
		
		if(sendto(sockfd,out,n,0,srv_addr,srv_len)!=n){
			err_abort("Fehler beim Schreiben des Sockets!");
		}
		
		strcpy(in,"nischt");
		n=recvfrom(sockfd,in,MAXLINE,0,(struct sockaddr *)NULL,(int *)NULL);
		if(n<0){
		err_abort("Fehler beim Lesen des Sockets!");
		} 
		
		printf("in: %s\n",in);
	}
	else if(strncmp(tok,"HSOSSTP_ERROR",13)==0){
		tok = strtok(NULL, ">");
		tok++;
		tok[strlen(tok)]='\0';
		printf("Die Ursache: %s\n", tok);
	}
	else{
		printf("hier ist was schiefgelaufen!");
	}
	
	
	//Iteratives Empfangen der Daten
	while(check){
		
		tok = strtok(in, ";");
		printf("in: %s\n",in);
		if(strncmp(tok,"HSOSSTP_DATAX",13)==0){
			check = 0;
			printf("Daten empfangen - mehr oder weniger :)");
		}
		else{
			printf("Daten nicht empfangen :'(");
			check = 0;
		}
		/*
		tok++;
		tok[strlen(tok)-1]='\0';
		key = atoi(tok);
		sprintf(out,"HSOSSTP_GETXX;<%d>;<%d>",key,chunkno);
		
		printf("out: %s\n",out);
		
		n = strlen(out);
		
		if(sendto(sockfd,out,n,0,srv_addr,srv_len)!=n){
			err_abort("Fehler beim Schreiben des Sockets!");
		}
		
		
		n=recvfrom(sockfd,in,MAXLINE,0,(struct sockaddr *)NULL,(int *)NULL);
		if(n<0){
		err_abort("Fehler beim Lesen des Sockets!");
		} 
		*/
		//printf("in: %s\n",in);
	}

	
	

}

/*
Ausgabe von fehlern und Beenden des Programms
*/
void err_abort(char *str){
	fprintf(stderr,"UDP Echo-Client: %s\n",str);
	fflush(stdout);
	fflush(stdin);
	exit(1);
}
