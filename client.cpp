// Nume: MARDALE ANDREI
// Grupa: 324 CB
// Tema 2 Sistem de Backup si Partajare a Fisierelor
// CLIENT
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <fstream>
#include <map>
#include <algorithm>
#include <vector>
#include <fcntl.h>
#include <unordered_map>

#define BUFLEN 4096

using namespace std;

bool is_LOGGED = false; /*retine daca exista un user logat */
bool TO_QUIT = false; /* retine daca s-a dat comanda QUIT*/
string LOGGED_USER; /* username-ul utilizatorul logat*/

/* Functie care imparte un string dupa un separator*/
vector<string> split(const string &text, char sep) {
    vector<string> tokens;
    size_t start = 0, end = 0;
    while ((end = text.find(sep, start)) != std::string::npos) {
        string temp = text.substr(start, end - start);
        if (temp != "") tokens.push_back(temp);
        start = end + 1;
    }
    string temp = text.substr(start);
    if (temp != "") tokens.push_back(temp);
    return tokens;
}

/* Functie care determina daca un fisier exista sau nu*/
bool isFile (string file){

	if (FILE *f = fopen(file.c_str(), "r")) {
        fclose(f);
        return true;
    } else {
        return false;
    }
}

/*Functie care determina dimensiunea unui fisier*/
unsigned long fsize(const char *filename) {
    struct stat st; 

    if (stat(filename, &st) == 0)
        return st.st_size;

    return -1; 
}

/*Incrementeaza contorul pentru politica de round-robin*/
void increment (int &count, vector <pair<string, int>> sending){
	if (count == sending.size() - 1){
		count = 0;
	}
	else {
		count++;
	}
}

/*Functie care recupereaza numele dintr-un header de pachet trimis/receptionat*/
string unwrapName (char* name){
	string fisier;
	for (int i = 0; i < 40; i++){ /*40 este dimensiunea maxima pe care o poate lua un nume de fisier*/
		if (name[i] != '@'){ /*elimina paddingul adaugat*/
			fisier = fisier + name[i];
		}
	}
	return fisier;
}

/*functie care formateaza "frumos" dimensiunea unui fisier*/
string prettyFormat (unsigned long n){
	string result;
	int chunks[100];
	int i = 0, j;
	while (n != 0){
		chunks[i++] = n % 1000;
		n = n / 1000;
	}
	for (j = i-1; j >= 0; j--){
		if (j != 0)
			result = result +  to_string(chunks[j]) + ".";
		else {
			result = result + to_string(chunks[j]);
		}
	}
	return result;
}

/*Wrapper peste send() -> asigura trimiterea totala a datelor*/
int sendAll(int s, char* buf, int* len){
    int total = 0;        // cati bytes am trimis
    int bytesleft = *len; // cati bytes au mai ramas de trimis
    int n;

    while(total < *len) {
        n = send(s, buf+total, bytesleft, 0);
        if (n == -1) { break; }
        total += n;
        bytesleft -= n;
    }

    *len = total; // intoarce numarul total de bytes

    return n==-1?-1:0; // 0 succes / -1 esec
}

/*Wrapper peste recv() -> asigura primirea totala a datelor*/
int recvAll (int sockfd, char* buf, int len ) {
	int n = 0;
	while (n < len) {
		int recvd = recv(sockfd, buf + n, len - n, 0);
		if ( recvd < 0){
			perror("recv error");
			exit(0);
		}
		n = n + recvd;
	}
	return n; 
}

void printPromt(char* buffer, FILE* g){
	printf("%s\n", buffer + 1); /* afisez raspunsul serverului */
	printf("%s> ", LOGGED_USER.c_str()); /* afisez promt*/
	fflush(stdout);

	fprintf(g, "%s\n", buffer + 1); /* afisez raspunsul serverului */
	fprintf(g, "%s> ", LOGGED_USER.c_str()); /* afisez promt*/
	fflush(g);
}

int main(int argc, char *argv[])
{
    int sockfd, n, i, download_socket;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char recv_buffer[BUFLEN]; /* buffer pentru citirea de la tastatura */
    char buffer[BUFLEN]; /* buffer pentru mesaje venite de la server*/
	char upload_buf[BUFLEN]; /* buffer pentru upload catre server*/

	char* continut = (char*) malloc (BUFLEN); /* payload-ul efectiv pt download*/
	char* msj = (char*) malloc (BUFLEN); /* buffer pt diferite mesaje*/


    int count = 0; /* contor folosit pentru politica de round robin */

    vector <pair<string, int>> sending;	/* lista de fisiere in curs de upload */
    vector<string> receving; /* lista de fisiere in curs de download */
    unordered_map <string, int> descriptor; /* mapeaza un nume de fisier cu un file descriptor pt upload*/
    unordered_map <string, int> down_descriptor; /* mapeaza un nume de fisier cu un file descriptor pt download*/

    
    fd_set read_fds;	//multimea de citire folosita in select()
    fd_set write_fds; // multimea de scriere folosita in select()
    fd_set tmp_wr_fds; // multimea folosita temporar pt scriere
    fd_set tmp_fds;	//multime folosita temporar 
    int fdmax;		//valoare maxima file descriptor din multimea read_fds

    if (argc < 3) {
       fprintf(stderr,"Usage %s server_address server_port\n", argv[0]);
       exit(0);
    }  

    /* golim multimea de descriptori */
    FD_ZERO(&read_fds);
    FD_ZERO(&tmp_fds);
    FD_ZERO(&write_fds);
    FD_ZERO(&tmp_wr_fds);

    /*creaza un socket pe care se va comunica cu serverul*/ 
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        fprintf(stderr,"Error opening socket");
        exit(0);
    }

    /* pregateste structurile pentru adrese*/
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(argv[2]));
    inet_aton(argv[1], &serv_addr.sin_addr);
    
    /*conectare la server*/
    if (connect(sockfd,(struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0) {
        fprintf(stderr,"Error connecting");
        exit(0);
    }

    /* seteaza multimile de descriptori*/
    FD_SET(sockfd, &read_fds); /* introduce socket-ul de comunicare in multimea de citire*/
    FD_SET(STDIN_FILENO, &read_fds); /* introduce tastatura in multimea de de citire*/
    FD_SET(sockfd, &write_fds); /* introduce socket-ul de comunicare cu serverul in multimea de scriere*/

    fdmax = sockfd;

    /* structura pentru time-out la select()*/
    struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 90;

	/* pregatire fisier de log*/
	char* path = (char*) malloc (100);
	sprintf(path, "client-%d.log", getpid());
	FILE *g = fopen (path, "wt");

    // main loop
	while (1) {
		tmp_fds = read_fds;
		tmp_wr_fds = write_fds; 
		if (select(fdmax + 1, &tmp_fds, &tmp_wr_fds, NULL, &tv) == -1) {
			fprintf(stderr,"ERROR in select");
        	exit(1);
		}
		for(i = 0; i <= fdmax; i++) {
			if (FD_ISSET(i, &tmp_fds)) {
				if (i == sockfd) { /* am primit mesaj de la server => actiunea recv()*/
					memset(buffer, 0, BUFLEN); /* curat buffer-ul*/
					// n = recv(i, buffer, sizeof(buffer), 0); /* primesc mesajul de server*/
					n = recvAll(i, buffer, sizeof(buffer));

					if (strncmp(buffer, "piece", 5) == 0){ /* partile de fisier downloadate incep cu "piece"*/
						char cmd[6]; /* va retine comanda "piece"*/
						char file[41]; /* numele fisierul transferat prin download*/
						char dim[5]; /* dimensiunea efectiva a datelor primite prin download (fara antet)*/


						/*obtin comanda = piece*/
						strncpy(cmd, buffer, 5);
						cmd[5] = '\0';
						/*obtin numele fisierului*/
						strncpy(file, buffer+5, 40);
						file[40] = '\0';
						string fisier = unwrapName(file); /* scot padding-ul*/
						/*obtin cantitatea de informatie utile*/
						strncpy(dim, buffer+45, 4);
						dim[5] = '\0';

						// printf("%s %s %s \n", cmd, fisier.c_str(), dim );
						int fd = down_descriptor[fisier]; /*obtin fd-ul fisierului ce va fi scris pe baza maparii*/
						/* daca in campul dim am "done", inseamna ca am terminat*/
						if (strcmp(dim, "done") == 0){
							vector<string >::iterator result = find(receving.begin(), receving.end(), fisier);
							receving.erase(result); /* sterg fisierul din lista de primiri*/
							close(fd); /* inchid fisierul pe disc*/

							/*afisez mesajul de final de download*/
							char* path = (char*) malloc (100);
							sprintf(path, "%d_%s", getpid(), fisier.c_str());

							printf("Download finished: %s - %s bytes \n", fisier.c_str(), prettyFormat(fsize(path)).c_str());
							fprintf(g, "Download finished: %s - %s bytes \n", fisier.c_str(), prettyFormat(fsize(path)).c_str());
							
							/* daca s-a dat comanda quit in timpul unui download*/
							if (TO_QUIT == true && sending.size() == 0 && receving.size() == 0){ /* daca este ultimul fisier downloadat*/
								/*pregatesc clientul pentru quit*/
								memset(buffer, 0, BUFLEN);
    							sprintf(buffer, "quit");
    							send(sockfd, buffer, sizeof(buffer), 0);
    							close(sockfd); 
								FD_CLR(sockfd, &read_fds);
								FD_CLR(sockfd, &write_fds);
    							exit(0);
							}

							free(path);

							printf("%s> ", LOGGED_USER.c_str());
							fflush(stdout);	
							fprintf(g, "%s> ", LOGGED_USER.c_str());
							fflush(g);
							break;
						}

						int sz;
						sscanf(dim, "%d", &sz); /* convertesc din char* in int dimensiunea utila*/

						memcpy(continut, buffer+49, sz); // copiere eficienta a continutului!
						continut[sz] = '\0';

						int m = write (fd, continut, sz); /*scriu payload-ul primit pe disc*/
						memset(continut, 0, sz);
					}
					else if (strcmp (buffer, "8") == 0){ /* codul pentru Brute Force*/

						printf("-8 Brute force detectat!\n "); /* afisez la consola*/
						fprintf(g, "-8 Brute force detectat!\n "); /* scriu in fisier de log*/

						close(sockfd); /* inchid conexiunea catre server*/
						FD_CLR(i, &read_fds);
						exit(0);
					}
					else if (strcmp (buffer, "3") == 0){ /* codul pentru user/parola gresita*/
						sprintf(msj, "-3 User/ parola gresita!\n");
						printf("-3 User/ parola gresita!\n"); /* afisez la consola */
						fprintf(g, "-3 User/ parola gresita!\n"); /* scriu in fisieru de log*/
					}
					else if (strcmp (buffer, "0") == 0){ /* codul pentru login cu succes*/
						printf("%s> ", LOGGED_USER.c_str()); /* afisez promptul pentru urmatoare linie*/
						fflush(stdout);

						fprintf(g, "%s> ", LOGGED_USER.c_str()); /* afisez promptul pentru urmatoare linie*/
						fflush(g);


						is_LOGGED = true; /* marchez ca logat clientul curent*/
					}
					else if (buffer[0] == 'l'){ // userlist
						printPromt(buffer,  g);
					}
					else if (buffer[0] == 'F'){ // getfilelist
						printPromt(buffer,  g);
					}
					else if (buffer[0] == 'S'){ // raspuns cu SHARE
						printPromt(buffer, g);
					}
					else if (buffer[0] == 'U'){ // raspuns cu UNSHARE
						printPromt(buffer, g);
					}
					else if (buffer[0] == 'u') { // raspuns despre upload
						if (buffer[1] == '0'){ // raspuns negativ. fisierul exista deja
							printf("%s\n", buffer + 2); /* afisez raspunsul serverului */
							printf("%s> ", LOGGED_USER.c_str()); /* afisez promt*/
							fflush(stdout);

							fprintf(g, "%s\n", buffer + 2); /* afisez raspunsul serverului */
							fprintf(g, "%s> ", LOGGED_USER.c_str()); /* afisez promt*/
							fflush(g);
						}
						else if (buffer[1] == '1') { // raspuns pozitiv: se poate incepe trimiterea fisierului
							string filename = string(buffer+2);
    						sending.push_back(make_pair(filename,0)); // se adauga fisierul la lista de trimitere

							int fd = open(filename.c_str() ,O_RDONLY); /* deschid fisierul pentru citire*/
							descriptor[filename] = fd; /* mapez numele fisierului cu fd-ul lui*/

    						printf("%s> ", LOGGED_USER.c_str());
							fflush(stdout);

							fprintf(g, "%s> ", LOGGED_USER.c_str());
							fflush(g);
						}
					}
					else if (buffer[0] == 'L') { // UPLOAD TERMINAT
						printPromt(buffer, g);
					}
					else if (buffer[0] == 'd'){ // raspuns cu delete
						printPromt(buffer, g);
					}
					else if (buffer[0] == 'D'){ // raspuns pentru download
						if (buffer[1] == '0' || buffer[1] == '1') { // fisierul nu exista sau e privat
							printf("%s\n", buffer + 2); /* afisez raspunsul serverului */
							printf("%s> ", LOGGED_USER.c_str()); /* afisez promt*/
							fflush(stdout);

							fprintf(g, "%s\n", buffer + 2); /* afisez raspunsul serverului */
							fprintf(g, "%s> ", LOGGED_USER.c_str()); /* afisez promt*/
							fflush(g);
						}
						else if (buffer[1] == '2'){
							string remain = string(buffer+2);
							vector<string> v = split(remain, ' ');

							string user = v.at(0); /* determin user-ul detinator al fisierului */
							string filename = string( buffer + 3 + user.size()); /* numele fisierului */

							receving.push_back(filename); /* adaug acest transfer in lista de primiri*/

							char* path = (char*) malloc (50);
							sprintf(path, "%d_%s", getpid(), filename.c_str()); /* numele noului fisier */

							int fd = open(path, O_WRONLY|O_CREAT| O_APPEND, 0644); /* deschid fisierul pentru scriere*/
							down_descriptor[filename] = fd; /* mapez numele fisierului cu fd-ul lui*/

							/* dau acceptul serverului pentru a incepe trimiterea*/
							memset(buffer, 0, BUFLEN);
    						sprintf(buffer, "start %s %s", user.c_str(), filename.c_str());
    						send(sockfd, buffer, sizeof(buffer), 0);

						}
					}
					else if (strcmp(buffer, "quit\n") == 0){ // serverul se inchide
						/* inchid toata resursele folosite */
						fclose(g);
						close(sockfd); 
						FD_CLR(sockfd, &read_fds);
						FD_CLR(sockfd, &write_fds);
    					exit(0);
					}
				}
				else if (i == STDIN_FILENO){ /* am primit mesaj de la tastatura*/
					memset (recv_buffer, 0, BUFLEN);
    				fgets(recv_buffer, BUFLEN-1, stdin);
    				fprintf(g, "%s", recv_buffer ); /* afisez comanda primita in log*/

					string backup = string(recv_buffer);
    				vector<string> v = split (backup, ' '); /* splituiesc comanda dupa ' '*/
    				/* Trimit comanda primita de la tastatura catre server*/
    				if (v.at(0) == "login"){
    					if (v.size() != 3){
    						printf("Numar incomplet de parametrii! \n");
    						fprintf(g, "Numar incomplet de parametrii! \n");
    					}
    					if (is_LOGGED == true){
    						cout<<"-2 Sesiune deja deschisa" << endl;
    						fprintf(g, "-2 Sesiune deja deschisa\n");
    						break;
    					}
    					LOGGED_USER = v.at(1);
    					n = send(sockfd,recv_buffer, sizeof(recv_buffer), 0);

    				}
    				else if (strcmp (recv_buffer, "logout\n") == 0){
    					if (is_LOGGED == false){
    						cout << "-1 Clientul nu e autentificat!" << endl;
    						fprintf(g, "-1 Clientul nu e autentificat!\n");
    					}
    					else {
    						LOGGED_USER = "";
    						is_LOGGED = false;
    						memset(buffer, 0, BUFLEN);
    						sprintf(buffer, "logout");
    						send(sockfd, buffer, sizeof(buffer), 0);
    					}
    				}
    				else if (strcmp (recv_buffer, "getuserlist\n") == 0){
    					if (is_LOGGED == false){
    						cout << "-1 Clientul nu e autentificat!" <<endl;
    						fprintf(g, "-1 Clientul nu e autentificat!\n");
    					}
    					else {
    						memset(buffer, 0, BUFLEN);
    						sprintf(buffer, "getuserlist");
    						send(sockfd, buffer, sizeof(buffer), 0);
    					}
    				}
    				else if (v.at(0) == "getfilelist"){
    					if (is_LOGGED == false){
    						cout << "-1 Clientul nu este autentificat! " << endl;
    						fprintf(g, "-1 Clientul nu e autentificat!\n");
    					}
    					else {
    						memset(buffer, 0, BUFLEN);
    						sprintf(buffer, "%s", recv_buffer);
    						send(sockfd, buffer, sizeof(buffer), 0);
    					}
    				}
    				else if (v.at(0) == "share"){
    					if (is_LOGGED == false){
    						cout << "-1 Clientul nu este autentificat! " << endl;
    						fprintf(g, "-1 Clientul nu e autentificat!\n");
    					}
    					else {
    						memset(buffer, 0, BUFLEN);
    						sprintf(buffer, "%s", recv_buffer);
    						send(sockfd, buffer, sizeof(buffer), 0);
    					}
    				}
    				else if (v.at(0) == "unshare") {
    					if (is_LOGGED == false){
    						cout << "-1 Clientul nu este autentificat! " << endl;
    						fprintf(g, "-1 Clientul nu e autentificat!\n");
    					}
    					else {
    						memset(buffer, 0, BUFLEN);
    						sprintf(buffer, "%s", recv_buffer);
    						send(sockfd, buffer, sizeof(buffer), 0);
    					}
    				}
    				else if (v.at(0) == "upload"){
    					if (is_LOGGED == false){
    						cout << "-1 Clientul nu este autentificat! " << endl;
    						fprintf(g, "-1 Clientul nu e autentificat!\n");
    					}
    					else {
    						string file = string (recv_buffer + 7); /*determin numele fisierului*/
    						file.erase(remove(file.begin(), file.end(), '\n'), file.end());
    						if (isFile (file) == 0){
    							cout << "-4 Fisier inexistent!" << endl;
    							fprintf(g, "-4 Fisier inexistent! \n");

    							printf("%s> ", LOGGED_USER.c_str());
								fflush(stdout);

								fprintf(g, "%s> ", LOGGED_USER.c_str());
								fflush(g);
    							break;
    						}
    						else {
    							memset(buffer, 0, BUFLEN);
	    						sprintf(buffer, "%s", recv_buffer);
	    						send(sockfd, buffer, sizeof(buffer), 0);
    						}
    					}
    				}
    				else if (v.at(0) == "delete") {
    					if (is_LOGGED == false){
    						cout << "-1 Clientul nu este autentificat!" << endl;
    						fprintf(g, "-1 Clientul nu e autentificat!\n");
    					}
    					else {
    						memset(buffer, 0, BUFLEN);
    						sprintf(buffer, "%s", recv_buffer);
    						send(sockfd, buffer, sizeof(buffer), 0);
    					}
    				}
    				else if (v.at(0) == "download") {
    					if (is_LOGGED == false){
    						cout << "-1 Clientul nu este autentificat! " << endl;
    						fprintf(g, "-1 Clientul nu e autentificat!\n");

    						printf("%s> ", LOGGED_USER.c_str());
							fflush(stdout);
							fprintf(g, "%s> ", LOGGED_USER.c_str());
							fflush(g);
    					}
    					else {
    						memset(buffer, 0, BUFLEN);
	    					sprintf(buffer, "%s", recv_buffer);

	    					vector<string> pcs = split(buffer, ' ');
	    					if (pcs[1] == "@") {
	    						pcs[1] = LOGGED_USER;

	    						string file = string (buffer + 11);
								sprintf(buffer, "download %s %s", pcs[1].c_str(), file.c_str());
	    					}


	    					send(sockfd, buffer, sizeof(buffer), 0);
	    					printf("%s> ", LOGGED_USER.c_str());
							fflush(stdout);

							fprintf(g, "%s> ", LOGGED_USER.c_str());
							fflush(g);
    					}
    				}
    				else if (strcmp(recv_buffer, "quit\n") == 0){
    					if (is_LOGGED == false){
    						cout << "-1 Clientul nu este autentificat!" << endl;
    						fprintf(g, "-1 Clientul nu e autentificat!\n");
    					}
    					else {
    						if (sending.size() == 0 && receving.size() == 0){
    							memset(buffer, 0, BUFLEN);
    							sprintf(buffer, "quit");
    							send(sockfd, buffer, sizeof(buffer), 0);
    							close(sockfd); 
    							fclose(g);
								FD_CLR(sockfd, &read_fds);
								FD_CLR(sockfd, &write_fds);
    							exit(0);
    						}
    						else {
    							TO_QUIT = true;
    						}
    					}
					}
				}
			}
			else if (tv.tv_usec == 0 && FD_ISSET(i, &tmp_wr_fds)){ 
				if (i == sockfd) { /* a venit timpul sa scriu un mesaj catre server => actiunea send()*/
					if (sending.size() != 0){
						string file = get<0> (sending.at(count)); /* determin fisierul ce trebuie trimis pe baza round robin*/

						string bkf = file; /* copie de backup, pentru a crea numele cu Padding*/


						int dim_antet = 49; // primii 5 -> piece, urmatorii 24 nume fisier, urmatorii
						int fd = descriptor[file];

						int m = read (fd, upload_buf, BUFLEN - dim_antet); /* citesc din fisier BUFLEN - dim_antet bytes*/

						if (m <= 0){ /* daca am terminat de citit, trimit un mesaj de anuntare*/
							char antet[BUFLEN];

							char* name = (char*) calloc (41, sizeof(char));
							name = strcpy(name , file.c_str()); 

							char* padding = (char*) calloc (40 - file.size(), sizeof(char));
							for (int k = 0; k < 40 - file.size(); k++){
								padding[k] = '@';
							}
							strcat(name, padding);

							sprintf(antet, "piece%sdone", name); /*in loc de dimensiune, voi avea "done"*/

							int sss = send(sockfd, antet, BUFLEN, 0);		
							vector <pair<string, int>>::iterator result = find(sending.begin(), sending.end(), make_pair(file, 0));
							sending.erase(result); /*sterg fisierul din lista de trimiteri*/
							close(fd);
							count = 0; /* resetez contorul */
							/* daca s-a dat intre timp o comanda de quit si acesta a fost ultimul transfer*/
							if (TO_QUIT == true && sending.size() == 0 && receving.size() == 0){
								/* pregatesc clientul de quit*/
								memset(buffer, 0, BUFLEN);
    							sprintf(buffer, "quit");
    							send(sockfd, buffer, sizeof(buffer), 0);
    							close(sockfd); 
								FD_CLR(sockfd, &read_fds);
								FD_CLR(sockfd, &write_fds);
    							exit(0);
							}
							break;		
						}

						char antet[BUFLEN];

						string pad = bkf.append(40 - bkf.size(), '@'); // PADDING EFICIENT

						sprintf(antet, "piece%s%04d", pad.c_str(), m); // pun ca prefix: piece_numefisier_cattrimit

						char* toSend = (char*) malloc(BUFLEN);

						memcpy(toSend, antet, dim_antet);
						memcpy(toSend + dim_antet, upload_buf, m);
						toSend[dim_antet + m] = '\0';

						int len = BUFLEN;
						sendAll(sockfd, toSend, &len ); /* trimit datele prin TCP*/
						// send(sockfd, toSend, BUFLEN, 0);
						/* resetez bufferele*/
						memset(antet, 0, BUFLEN);
						memset(upload_buf, 0, BUFLEN);
						memset(toSend, 0, BUFLEN);
						/*incrementez contorul circular*/
						increment(count, sending);
					}
				}
				tv.tv_usec = 90; /* resetez timer-ul pentru select*/
			}
		}
    }
    return 0;
}


