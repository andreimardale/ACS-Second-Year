// Nume: MARDALE ANDREI
// Grupa: 324 CB
// Tema 3 Web Crawler
// SERVER

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <algorithm>
#include <vector>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <tuple>
#include <errno.h>
#include <queue>

#define MAX_CLIENTS	10
#define BUFLEN 4096

using namespace std;

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

/*Creeaza structura de directoare a site-ului; returneaza fd-ul pentru fisierul de descarcat*/
int createFolders(char* numePag, char* calePag){
	mkdir(numePag, 0777);
	chdir(numePag);
	char* tok;
	int fd = -2;
	char* bkp = strdup(calePag);
	bkp[strlen(bkp)] = '\0';
	tok = strtok(bkp+1, "/"); 
	while(tok != NULL){
		if (strstr(tok, ".html") != NULL){ /* cand ajung la un token cu extensia html, ma opresc din a crea directoare*/
			fd = open(tok, O_WRONLY|O_CREAT| O_TRUNC, 0644);
			break;
		}
		mkdir(tok, 0777);
		chdir(tok);
		tok = strtok(NULL, "/");
	}
	if (fd == -2){
		fd = open("index.html", O_WRONLY|O_CREAT| O_TRUNC, 0644 ); /* daca cumva nu exista ultima parte, creez automat index.html*/
	}

	return fd;
}

/*Functie care recupereaza numele dintr-un header de pachet trimis/receptionat*/
string unwrapName (char* name){
	string fisier;
	for (int i = 0; i < 96; i++){ /*40 este dimensiunea maxima pe care o poate lua un nume de fisier*/
		if (name[i] != '@'){ /*elimina paddingul adaugat*/
			fisier = fisier + name[i];
		}
	}
	return fisier;
}

/*Incrementeaza contorul pentru politica de round-robin*/
void increment (int &count, vector<int> socket_clients){
	if (count == socket_clients.size() - 1){ /* incrementearea este circulara-> daca am ajuns la dim. listei => 0*/
		count = 0;
	}
	else {
		count++; /* altfel incrementez cu 1*/
	}
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
		if (recvd < len){
			return recvd;
		}
		n = n + recvd;
	}
	return n; 
}

/*Functie care obtine pe baza unui link complet, numele site-ului si calea catre resursa*/
void decompose(const char* link, char** nume_pagina, char** cale_pagina){
	char* bkp = strdup(link);
	char* tok = strtok(bkp + 8, "/");
	*nume_pagina = strdup(tok);
	*cale_pagina = strdup(link + 8 + strlen(tok));
}

/* Functie care combina doua path-uri care se pot suprapune*/
string combinePaths(string path1, string path2){
	vector<string> splitted1 = split(path1, '/');
	vector<string> splitted2 = split(path2, '/');
	string result;
	int i = 0;
	int j = 0;
	/*pana la primul element comun, iau din primul path*/
	while (i < splitted1.size() && splitted1.at(i) != splitted2.at(j)){
		result = result + "/" + splitted1.at(i);
		i++;
	}
	/* cat timp am elemente comune, le iau*/
	while (i < splitted1.size() && j < splitted2.size() && splitted1.at(i) == splitted2.at(j)){
		result = result+ "/" + splitted1.at(i);
		i++;
		j++;
	}
	/* cand nu mai am elemente comune iau din al doilea path*/
	while ( j < splitted2.size()){
		result = result+ "/" + splitted2.at(j);
		j++;
	}

	vector<string> splitted = split(result, '/');
	/* daca nu exista / , adaug*/
	if (splitted.at(splitted.size() - 1).find(".") == std::string::npos && result.at(result.size() - 1) != '/'){
		result = result + "/";
	}

	return result;
}

/* Functie care curata un path de .. (aplicandu-le)*/
string removeBacks (string toQuerry){
	vector<string> splitted = split(toQuerry, '/');
	int i;
	for (i = 0; i < splitted.size(); i++) {
		if (splitted.at(i) == ".."){ /* daca gasesc .., sterg ultimul token impreuna cu ..*/
			splitted.erase(splitted.begin() + i);
			splitted.erase(splitted.begin() + i - 1);
			i = i - 2;
		}
	}
	string clearPath;
	for (i = 0; i < splitted.size(); i++){
		clearPath = clearPath + "/" + splitted.at(i);
	}

	vector<string> splitted1 = split(clearPath, '/');

	if (splitted1.at(splitted1.size() - 1).find(".") == std::string::npos && clearPath.at(clearPath.size() - 1) != '/'){
		clearPath = clearPath + "/";
	}

	return clearPath;
}
/* Obtine un vector de linkuri din pachetul primit de la client*/
vector<string> unpackLinks(char* buffer){
	vector<string> wantedLinks;
	char* tok;
 	tok = strtok (buffer,"|");
	while (tok != NULL){
		wantedLinks.push_back(string(strdup(tok)));
		tok = strtok (NULL, "|");
	}

	return wantedLinks;
}
/* Creeaza foldere pana ajunge la fisierul efectiv ce trebuie creat*/
int completeFolderPath(string download_file_path){
	vector<string> splitted = split(download_file_path, '/');
	int fd;
	if (splitted.at(splitted.size() - 1).find(".") == std::string::npos){
		for (int i = 0; i < splitted.size(); i++){
			if (chdir(splitted.at(i).c_str()) != 0){
				mkdir(splitted.at(i).c_str(), 0777);
				chdir(splitted.at(i).c_str());
			}
		}
		fd = open("unnamed.html", O_WRONLY|O_CREAT|O_TRUNC, 0644);
	}
	else {
		for (int i = 0; i < splitted.size() - 1; i++){
			if (chdir(splitted.at(i).c_str()) != 0){
				mkdir(splitted.at(i).c_str(), 0777);
				chdir(splitted.at(i).c_str());
			}
		}
		fd = open(splitted.at(splitted.size()-1).c_str(), O_WRONLY|O_CREAT|O_TRUNC, 0644);
	}
	return fd;
}

/* Pe baza unei liste de linkuri cu fisiere ce vor fi descarcate, creeaza foldere si fisere pe disc si mapeaza fd-urile*/
void prepareFolders(vector<string> wantedLinks, char* numePag, char* path, unordered_map <string, int> *desc){
	int i;
	string toSplit = string(path); // calea catre pagina html sursa
	vector<string> splitted = split(toSplit, '/');
	string newPath, toQuerry;
	for (i = 0; i < splitted.size() - 1; i++){
		newPath = newPath+ "/" + splitted.at(i); // calea catre directorul unde trebuie salvate chestii
	}

	char homePath[100];
	getcwd(homePath, sizeof(homePath)); // determin calea pt home
	homePath[sizeof(homePath) - 1] = '\0';


	for (i = 0; i < wantedLinks.size(); i++){

		if(wantedLinks.at(i) == "/") /* sunt cazuri incare exista doar / */
			continue;

		string toQuerry = combinePaths(newPath, wantedLinks.at(i)); 
		string download_file_path = string(numePag) + toQuerry; /* calea corecta catre resursa */
		chdir(homePath); // ma intorc in directorul home al temei
		int fd = completeFolderPath(download_file_path); // construiesc folderele
		(*desc)[wantedLinks.at(i)] = fd; /* mapeaz descriptorul pentru fisierul nou creat*/
	}
}
/* Updateaza coada de linkuri pe baza listei de linkuri primite in cazul recursive*/
void updateQueue(queue<string> *links, vector<string> wantedLinks, char* numePag, char* path, unordered_map<string, bool> *opened){
	int i;
	string toSplit = string(path); // calea catre pagina html sursa
	vector<string> splitted = split(toSplit, '/');
	string newPath, toQuerry;
	for (i = 0; i < splitted.size() - 1; i++){
		newPath = newPath+ "/" + splitted.at(i); // calea catre directorul unde trebuie salvate chestii
	}

	for (i = 0; i < wantedLinks.size(); i++){

		if (wantedLinks.at(i) == "/")
			continue;
		char recLevel = wantedLinks.at(i).at(0);
		wantedLinks.at(i) = wantedLinks.at(i).substr(1, wantedLinks.at(i).size() - 1); /* elimin /n de la sfarsit*/
		string toQuerry1 = combinePaths(newPath, wantedLinks.at(i));
		string toQuerry = removeBacks(toQuerry1); /* obtin calea corecta*/

		if ((*opened)[toQuerry] == false){ /* daca nu a fost downloadat deja*/
			string download_file_path = string(1,recLevel) + "http://" + string(numePag) + toQuerry; 
			(*links).push(download_file_path); /* il inseres in coada (impreuna cu nivelul de recursivitate)*/
			(*opened)[toQuerry] = true; /* mapez linkul / fisierul in curs de descarcare*/
		}
	}
}
/* Afiseaza mesaje la stderr sau in fisier in functie de param. de input*/
void printErrMsg (int other, const char* message, FILE* g_error){
	if (other == 1){
		fprintf(g_error, "%s\n", message);
	}
	else {
		fprintf(stderr, "%s\n", message);
	}
}

/* Afiseaza mesaje la stdout sau in fisier in functie de param. de input*/
void printInfoMsg(int other, const char* message, FILE* g_info){
	if (other == 1){
		fprintf(g_info, "%s\n", message);
	}
	else {
		fprintf(stdout, "%s\n", message);
	}	
}


int main(int argc, char *argv[])
{	
	int no_clients = 0; /* numarul curent de clienti conectati*/
    int sockfd, newsockfd, portno; /* elemente necesare pentru setarea conexiunii TCP*/
    unsigned int clilen;
    int recursiv = 0, everything = 0, other = 0; /* parametrii de input */

    char buffer[BUFLEN]; /* buffer pentru date receptionate*/
    char sendBuffer[BUFLEN]; /* buffer pentru date trimise*/

    struct sockaddr_in serv_addr, cli_addr; /*structurile pentru socket*/
    int n, i; /* contori */
    int count = 0; /* contor pentru politica round robin*/

    char recv_buffer[BUFLEN]; /*buffer pentru comenzi receptionate */
    queue<string> links; /* coada de linkuri ce trebuie procesate */
    unordered_map <int, tuple<char*, char*, int>> sockToLink; /* mapeaza un socket cu un link -> pt fisiere*/
	int freeClients[1000]; /* retien clientii liberi*/    
	vector<tuple<char*, int >> clients; /*vector cu tupluride tipul IP, port -> pt status */
	vector<int> socket_clients;/* lista cu socket-urile clientilor -> pentru a trimite mesaj global, tuturor clientilor*/
	unordered_map<string, int> desc; /* mapeaza un fisier primit(everything) cu fd-ul lui*/
    char* continut = (char*) malloc(BUFLEN); /* continutul unui pachet din everything*/
	unordered_map<string, bool> opened; /* fisiere / link-uri descarcate deja*/
	char* msg = (char*) malloc(BUFLEN); /* pentru trimitere de mesaje catre functiile auxiliare de printare*/

    fd_set read_fds;	//multimea de citire folosita in select()
    fd_set tmp_fds;	//multime folosita temporar
    fd_set write_fds; // multime de scriere folosita in select()
    fd_set tmp_wr_fds; // multime folosita temporar pentru scriere

    int fdmax;		//valoare maxima file descriptor din multimea read_fds
    if (argc > 7) {
       fprintf(stdout,"./server [-r] [-e] [-o <fisier_log>] -p <port> \n");
       exit(1);
    }

    string logFile;
    char* port = (char*) malloc(100);
    FILE* g_error;
    FILE* g_info;

   	for (i = 0; i < argc ; i++){
   		if (strcmp(argv[i], "-r") == 0) { /* se doreste download recursiv */
   			recursiv = 1;
   		}
   		if (strcmp(argv[i], "-e") == 0){ /* se doreste Everything*/
   			everything = 1;
   		}
   		if (strcmp(argv[i], "-o") == 0){ /* fisierul de output*/
   			other = 1;
   			logFile = string(argv[i+1]);
   			string info = logFile + ".stdout";
   			string eror = logFile + ".stderr";
			g_info = fopen (info.c_str(), "wt");
			g_error = fopen(eror.c_str(), "wt");
   		}
   		if (strcmp(argv[i], "-p") == 0){ /* determin portul*/
   			port = argv[i+1];
   		}
   	}
  
    /* golim multimea de descriptori */
    FD_ZERO(&read_fds);
    FD_ZERO(&tmp_fds);
    FD_ZERO(&write_fds);
    FD_ZERO(&tmp_wr_fds);
    
    /*creaza un socket pe care se vor primi conexiuni*/ 
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
    	// fprintf(stderr,"ERROR opening socket");
    	printErrMsg(other, (char*)"ERROR opening socket", g_error);
       	exit(1);
    }
     
    portno = atoi(port);

    /*seteaza structurile pentru conexiunea TCP*/
    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;	// foloseste adresa IP a masinii
    serv_addr.sin_port = htons(portno);
     
    /*fac bind pe socket-ul anterior deschis*/ 
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(struct sockaddr)) < 0) {
    	// fprintf(stderr,"ERROR on binding");
    	printErrMsg(other, (char*)"ERROR on binding", g_error);

       	exit(1);
    }

    /* se asculta conexiuni pe acesti socket*/
    listen(sockfd, MAX_CLIENTS);

     //adaugam noul file descriptor (socketul pe care se asculta conexiuni) in multimea read_fds
    FD_SET(sockfd, &read_fds);
    FD_SET(STDIN_FILENO, &read_fds);

    /*structura de time-out pentru multiplexarea cu select()*/
    struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 90; /*timpul de time-out este de 90 microsecunde*/


    fdmax = sockfd;

    /* determin calea pentru home-ul serverului*/
    char homePath[100];
	getcwd(homePath, sizeof(homePath)); 
	homePath[sizeof(homePath) - 1] = '\0';

	int ccount = 0; /* contor pentru a retine cand e primul pachet sosit (ca sa elimin header de HTTP)*/
	int dim_antet = 0; /* dimensiunea header HTTP*/
     // main loop
	while (1) {
		tmp_fds = read_fds; 
		tmp_wr_fds = write_fds;
		if (select(fdmax + 1, &tmp_fds, &tmp_wr_fds , NULL, &tv) == -1) {
    		printErrMsg(other, (char*)"ERROR in select", g_error);
        	exit(1);
		}
	
		for(i = 0; i <= fdmax; i++) {
			if (FD_ISSET(i, &tmp_fds)) {
				if (i == sockfd) {
					// a venit ceva pe socketul inactiv(cel cu listen) = o noua conexiune
					// actiunea serverului: accept()
					clilen = sizeof(cli_addr);
					if ((newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen)) == -1) {
    					printErrMsg(other, (char*)"ERROR in accept", g_error);
        				exit(1);	
					} 
					else {
						//adaug noul socket intors de accept() la multimea descriptorilor de citire
						FD_SET(newsockfd, &write_fds);
						FD_SET(newsockfd, &read_fds);
						socket_clients.push_back(newsockfd); // adaug socket-ul in lista de clienti
						if (newsockfd > fdmax) { 
							fdmax = newsockfd;
						}
					}
					no_clients ++ ; /* actualizez numarul de clienti online*/
					freeClients[newsockfd] = 1; /* marchez clientul ca fiind liber*/
					clients.push_back(make_tuple(inet_ntoa(cli_addr.sin_addr),  ntohs(cli_addr.sin_port)));
					memset(msg, 0, BUFLEN);
					sprintf(msg, "Noua conexiune de la %s, port %d, socket_client %d\n ", inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port), newsockfd );
					printInfoMsg(other, msg, g_info);
				}
				else if (i == STDIN_FILENO){ /* a venit un mesaj de la tastatura */
					memset (recv_buffer, 0, BUFLEN); /*curat buffer-ul*/
    				fgets(recv_buffer, BUFLEN-1, stdin); /* preiau mesajul */
    				
    				if (strcmp(recv_buffer, "status\n") == 0){
    					for (int i = 0; i < clients.size(); i++){
    						memset(msg, 0, BUFLEN);
    						sprintf(msg, "%s %d \n", get<0> (clients.at(i)), get<1> (clients.at(i)));
    						printInfoMsg(other, msg, g_info);
    					}
    				}
    				if (strcmp(recv_buffer, "exit\n") == 0){
    					printInfoMsg(other, (char*)" Server-ul se va inchide! O zi buna", g_info);
    					for (int k = 0; k < socket_clients.size(); k++){ /* anunta toti clientii ca se inchide serverul*/
	    					int skt = socket_clients.at(k); /* se atentioneaza toti clientii de intentia de inchidere*/
	    					
	    					memset(msg, 0, BUFLEN);
	    					sprintf(msg, "Anunt clientul de pe socket-ul %d sa isi opreasca activitate! ", skt);
	    					printInfoMsg(other, msg, g_info);

	    					send(skt, recv_buffer, sizeof(buffer), 0);
	    					close(i); 
							FD_CLR(i, &read_fds); // scoatem din multimea de citire socketul pe care 
							FD_CLR(i, &write_fds); // scoatem din multimea de scriere socketul deconectat
	    				}
    					close(sockfd); /* inchid socket-ul principal*/
	    				return 0;
    				}
    				if (strncmp(recv_buffer, "download", 8) == 0){
    					strcpy(sendBuffer, recv_buffer + 9);
    					char* numePag;
						char* calePag;

						decompose(sendBuffer, &numePag, &calePag); /* obtin separat numele site-ului si calea catre fisier*/
						string cpag = string(calePag);
						cpag = cpag.substr(0, cpag.size() - 1); /* elimin /n de la sfarsit*/
						opened[cpag] = true; /* marchez link-ul ca deschis deja, pentru a nu il downloada de mai multe ori*/

						memset(msg, 0, BUFLEN);
						sprintf(msg, "Am primit comanda de download pentru site-ul: %s", sendBuffer);
						printInfoMsg(other, msg, g_info);

    					string numberedLink = "1" + string(sendBuffer); /* ii atasez nivel de recursivitate 1*/
    					numberedLink = numberedLink.substr(0, numberedLink.size() - 1);
    					links.push(numberedLink); /* il inserez in coada de descarcari*/
    				}
				}					
				else {
					// am primit date pe unul din socketii cu care vorbesc cu clientii
					//actiunea serverului: recv()
					memset(buffer, 0, sizeof(buffer)); /*golesc bufferul*/
					char* httpbody; /* corpul mesajului http, dupa eliminare header*/

					if ((n = recvAll(i, buffer, BUFLEN)) <= 0) {
						if (n == 0) {
							//conexiunea s-a inchis
							memset(msg, 0, BUFLEN);
							sprintf(msg, "selectserver: socket %d hung up", i);
							printInfoMsg(other, msg, g_info);
						} else {
							// fprintf(stderr,"ERROR in recv");
							printErrMsg(other, (char*)"ERROR in recv", g_error);
        					exit(1);
						}
						close(i); 
						FD_CLR(i, &read_fds); // scoatem din multimea de citire socketul pe care 
						FD_CLR(i, &write_fds); // scoatem din multimea de scriere socketul deconectat
					} 
					else { //recv intoarce >0
						if (strncmp(buffer, "site", 4) == 0){ /* se primeste site-ul initial cerut*/
							char* numePag = get<0> (sockToLink[i]); /* obtin numele site-ului pe baza maparii*/
							char* path = get<1> (sockToLink[i]); /* obtin calea catre resursa*/
							int file_d = get<2> (sockToLink[i]); /* obtin fd-ul pentru fisierul deschis pe disc*/
							
							if (ccount == 0){ /* daca e primul pachet primit, verific sa nu aiba header HTTP*/
								httpbody = strstr(buffer + 4, "\r\n\r\n"); /* header-ul HTTP se termina cu \r\n\r\n*/
								httpbody += 4; /* daca exista, il elimin*/
								dim_antet = httpbody - buffer; /* obtin dimensiunea antetului*/
							}

							if (ccount == 0 && httpbody){ /* daca s-a gasit un antet*/
								int nbytes = write(file_d, httpbody , n - dim_antet); /* scriu in fisier mai putini octeti*/
							}
							else {
								int nbytes = write(file_d, buffer+4, n - 4); /* daca nu s-a gasit, atunci scriu n - 4 (primii 4 fiind "site")*/
							}
							ccount ++ ; /* cresc contorul pentru pachete primite*/
							dim_antet = 0; /* reinitializez dimensiunea antetului*/
							
							if (n  < BUFLEN - 4){ /*daca e ultimul pachet primit*/
								ccount = 0; /* resetez contorul (pt urmatorul site)*/
								memset(msg, 0, BUFLEN);
								sprintf(msg, "Site-ul: %s%s a fost download cu succes!", numePag, path); /* afisez mesaj*/
								printInfoMsg(other, msg, g_info);

								memset(buffer, 0, BUFLEN); 
								int nrbyes = recv(i, buffer, BUFLEN - n, 0); // citesc informatia reduntanta de pe socket
								
								close(file_d);
								break;
							}
						}
						if (strncmp(buffer, "evth", 4) == 0){ /* se primeste o bucata de ceva in cadrul everything*/
							string wLink = unwrapName(buffer + 4); /* obtin numele fisierului*/
							int fd = desc[wLink]; /* obtin fd-ul*/
							memcpy(continut, buffer + 100, n - 100); /*obtin continutul, dupa eliminarea antetului*/
							continut[n-100] = '\0'; 
							int nrbytes = write(fd, continut, n-100); /* scriu continutul in fisier*/
							memset(continut, 0, BUFLEN); /* resetez bufferul*/
						}
						if (strncmp(buffer, "exit", 4) == 0){ /*daca a terminat un client de descarcat un site + fisiere*/
							int ok = 0;
							freeClients[i] = 1; /* il marchez ca liber*/

							for (int k = 0; k < 1000; k++){ 
								if (freeClients[k] == 1){ /* numar cati clienti liberi am*/
									ok ++;
								}
							}
							if (ok == no_clients && links.size() == 0){ /* daca toti clientii sunt liberi si coada e goala -> gata*/
								printInfoMsg(other, (char*)"Toti clientii si-au terminat treaba! Serverul se inchide!", g_info);
								for (int k = 0; k < socket_clients.size(); k++){ /* anunta toti clientii ca se inchide serverul*/
			    					int skt = socket_clients.at(k); /* se atentioneaza toti clientii de intentia de inchidere*/
			    					memset(buffer, 0, BUFLEN);
			    					sprintf(buffer, "exit\n");
			    					send(skt, buffer, BUFLEN, 0);
			    					close(skt); 
									FD_CLR(skt, &read_fds); // scoatem din multimea de citire socketul pe care 
									FD_CLR(skt, &write_fds); // scoatem din multimea de scriere socketul deconectat
			    				}
		    					close(sockfd);//  inchid socket-ul principal
		    					if (other == 1) {
		    						fclose(g_error);
		    						fclose(g_info);	
		    					}
			    				return 0;
							}
						}
						if (strncmp(buffer, "list", 4) == 0){ /* se primeste lista cu site-uri ce vor fi descarcate recursiv*/
							// printf("RECURSION:%s\n", buffer+ 4);
							char* numePag = get<0> (sockToLink[i]); 
							char* path = get<1> (sockToLink[i]);
							vector<string> wantedLinks = unpackLinks(buffer+4); /* despachetez buffer-ul*/
							updateQueue(&links, wantedLinks, numePag, path, &opened); /* adaug site-urile in coada*/
						}
						if (strncmp(buffer, "elst", 4) == 0){ /* se primeste lista cu fisiere de descarcat*/
							// printf("DOWNLOADS:%s\n", buffer + 4);
							char* numePag = get<0> (sockToLink[i]); 
							char* path = get<1> (sockToLink[i]);
							vector<string> wantedLinks = unpackLinks(buffer + 4); /* despachetez buffer-ul*/
							chdir(homePath); // ma intorc in directorul home al temei
							prepareFolders(wantedLinks, numePag, path, &desc); /* creez fisierele pe disc*/
						}
					}
				} 
			}
			else if (tv.tv_usec == 0 && FD_ISSET(i, &tmp_wr_fds)){  // verific periodic daca am ceva de trimis 
				if (i != sockfd){ 
					if (socket_clients.size() >= 5 && freeClients[socket_clients.at(count)] == 1 && links.size() != 0){ //  daca exista ceva de trimis in lista de trimiteri si am clienti conectati 
						string link = links.front(); /* determin numele fisierului de trimis */
						links.pop();

						int skt = socket_clients.at(count); /* aflu socket-ul clientului ce va face treaba*/
						freeClients[skt] = 0; /*marchez clientul ca ocupat*/

						char* numePag;
						char* calePag;

						decompose(link.c_str(), &numePag, &calePag); /* obtin numele paginii si numele resursei*/
						memset(msg, 0, BUFLEN);
						sprintf(msg, "Clientul de pe socket-ul %d , va downloada pagina: %s%s", skt, numePag, calePag);
						printInfoMsg(other, msg, g_info);

						chdir(homePath); // ma intorc in directorul home al temei
						int file_d = createFolders(numePag, calePag); /* creez fisierul pe disc (doar pt site-uri html)*/

						sockToLink[skt] = make_tuple(numePag, calePag, file_d);

						memset(sendBuffer, 0, BUFLEN);
						sprintf(sendBuffer, "%d%d%s", recursiv, everything, link.c_str());

						int nbytes = send(skt, sendBuffer, BUFLEN, 0); /* trimit link-ul ce trebuie descarcat*/
						memset(sendBuffer, 0, BUFLEN);

						if (nbytes < 0){
							printErrMsg(other, (char*)"Eroare la trimitere! :(", g_error );
							// fprintf(stderr, "Eroare la trimitere! :( ");
							exit(0);
						}


						/*incrementez contorul pentru politica de round-robin*/
						increment(count, socket_clients);
						
					}
					else if (freeClients[socket_clients.at(count)]  == 0){
						increment(count, socket_clients);
					}
				}
				tv.tv_usec = 90; /* resetez timerul pentru select */
			}
		}
    }

    close(sockfd); /* inchid socket-ul principal*/
    return 0; 
}
