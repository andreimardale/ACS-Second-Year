// Nume: MARDALE ANDREI
// Grupa: 324 CB
// Tema 3 Web Crawler
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
#include <netdb.h>
#include <arpa/inet.h>
#include <iostream>
#include <fstream>
#include <map>
#include <algorithm>
#include <vector>
#include <fcntl.h>
#include <unordered_map>

#define BUFLEN 4096
#define HTTP_PORT 80

using namespace std;

int globalCounter = 0;
int everything;
int recursiv;
int other;
FILE* g_error;
FILE* g_info;
int recLevel = -1;

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
/* Functia de la lab, pentru a citi o linie dintr-un mesaj primit de pe sockd*/
ssize_t Readline(int sockd, char *vptr, size_t maxlen) {
    ssize_t n, rc;
    char    c, *buffer;

    buffer = vptr;

    for ( n = 1; n < maxlen; n++ ) {	
	if ( (rc = read(sockd, &c, 1)) == 1 ) {
	    *buffer++ = c;
	    if ( c == '\n' )
		break;
	}
	else if ( rc == 0 ) {
	    if ( n == 1 )
		return 0;
	    else
		break;
	}
	else {
	    if ( errno == EINTR )
		continue;
	    return -1;
	}
    }

    *buffer = 0;
    return n;
}

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

/*Incrementeaza contorul pentru politica de round-robin*/
void increment (int &count, vector <pair<string, int>> sending){
	if (count == sending.size() - 1){
		count = 0;
	}
	else {
		count++;
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
		if (recvd == 0){
			break;
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

/*Realizeaza conexiunea la serverul HTTP cu ip-ul dat, returneaza socket-ul*/
int connectToHTTP(char* server_ip){
	int sockfd;
  	int port = HTTP_PORT;
  	struct sockaddr_in servaddr;


  	if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
		printf("Eroare la  creare socket.\n");
		exit(-1);
  	}  

  	/* formarea adresei serverului */
 	memset(&servaddr, 0, sizeof(servaddr));
  	servaddr.sin_family = AF_INET;
  	servaddr.sin_port = htons(port);

  	if (inet_aton(server_ip, &servaddr.sin_addr) <= 0 ) {
    	printf("Adresa IP invalida.\n");
    	exit(-1);
  	}
    
  	/*  conectare la server  */
  	if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0 ) {
    	printf("Eroare la conectare\n");
    	exit(-1);
  	}

  	return sockfd;
}

/* Obtine link-urile dintr-o pagina web*/
vector<string> getLinks(char* resource){
	int fd = open(resource, O_RDONLY);
	char line[BUFLEN];
	int i = 0;
	char* tok;
	vector<string> wantedLinks;

	while (1){
		memset(line, 0, BUFLEN);
		int nrbytes = Readline(fd, line, BUFLEN - 1);
		line[nrbytes] = 0;
		
		if (strstr(line, "href=")){

			tok = strtok (line,"\"\'");
			while (tok != NULL){
				if (strstr(tok, "href=")){
					tok = strtok(NULL, "\"\'");
					wantedLinks.push_back(string(strdup(tok)));
					break;
				}
				else {
					tok = strtok (NULL, "\"\'");
				}
			}
		}

		if (nrbytes == 0){
			close(fd);
			break;
		}
	}
	return wantedLinks;
}

/* Creeaza foldere pana ajunge la fisierul efectiv ce trebuie creat*/
int completeFolderPath(string download_file_path, string rep){
	vector<string> splitted = split(download_file_path, '/');
	int fd;
	if (splitted.at(splitted.size() - 1).find(".") == std::string::npos){
		for (int i = 0; i < splitted.size(); i++){
			if (chdir(splitted.at(i).c_str()) != 0){
				mkdir(splitted.at(i).c_str(), 0777);
				chdir(splitted.at(i).c_str());
			}
		}
		fd = open(rep.c_str(), O_WRONLY|O_CREAT|O_TRUNC, 0644);
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

/* Functie care combina doua path-uri care se pot suprapune*/
string combinePaths(string path1, string path2){
	vector<string> splitted1 = split(path1, '/');
	vector<string> splitted2 = split(path2, '/');
	string result;
	int i = 0;
	int j = 0;
	while (i < splitted1.size() && splitted1.at(i) != splitted2.at(j)){
		result = result + "/" + splitted1.at(i);
		i++;
	}
	while (i < splitted1.size() && j < splitted2.size() && splitted1.at(i) == splitted2.at(j)){
		result = result+ "/" + splitted1.at(i);
		i++;
		j++;
	}
	while ( j < splitted2.size()){
		result = result+ "/" + splitted2.at(j);
		j++;
	}

	vector<string> splitted = split(result, '/');
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
		if (splitted.at(i) == ".."){
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

/* Functie care downloadeaza toate fisierele gasite din lista wantedLinks*/
void downloadEverything(vector<string> wantedLinks, char* numePag, char* path, char* ip_address, int serv_socket){
	int i;
	char* msg = (char*) malloc(BUFLEN);
	string toSplit = string(path); // calea catre pagina html sursa
	vector<string> splitted = split(toSplit, '/');
	string newPath, toQuerry;
	for (i = 0; i < splitted.size() - 1; i++){
		newPath = newPath+ "/" + splitted.at(i); // calea catre directorul unde trebuie salvate chestii
	}
	unordered_map <string, bool> downloaded;

	for (i = 0; i < wantedLinks.size(); i++){ /* acum pastrez doar ce are extensie si nu e http si nu e #*/
		if (wantedLinks.at(i).find("http") != std::string::npos || wantedLinks.at(i).find("#") != std::string::npos) {
			continue;
		}
		else {
			string bkf = wantedLinks.at(i);
			string toQuerry1 = combinePaths(newPath, wantedLinks.at(i));
			string toQuerry = removeBacks(toQuerry1); /* obtine calea corecta catre resursa */

			if (downloaded[toQuerry] == true){ /* daca a fost deja downloadat acel fisier in skipuiesc*/
				continue;
			}
			else {
				downloaded[toQuerry] = true; /* altfel il marchez*/
			}
		    int sockfd = connectToHTTP(ip_address); /* ma conectez la sever HTTP*/

			char sendbuf[BUFLEN]; 
			char recvbuf[BUFLEN];
			char serv_buffer[BUFLEN];

			int dim_header = 100; /*header de dim 100 -> primii 4 pt "evth", urmatorii 96 pt numele resursei + padding*/

			memset(msg, 0, BUFLEN);
			sprintf(msg, "Se descarca resursa: %s", toQuerry.c_str());
			printInfoMsg(other, msg, g_info);


			memset(sendbuf, 0, BUFLEN);
			sprintf(sendbuf, "GET %s HTTP/1.0\n\n", toQuerry.c_str());
			write(sockfd, sendbuf, strlen(sendbuf));

			int count = 0;
			int dim_antet = 0;
			char* httpbody;
			
			char antet[BUFLEN];
			string pad = bkf.append(96 - bkf.size(), '@');
			sprintf(antet, "evth%s", pad.c_str()); 
			char* toSend = (char*) malloc (BUFLEN); /* construiesc antetul*/

			while (1){
				memset(recvbuf, 0, BUFLEN);
				memset(toSend, 0, BUFLEN);

				int n = recvAll(sockfd, recvbuf, BUFLEN - dim_header); /* preiau info de la serverul HTTP*/

				if(count == 0) { /* daca e primul pachet primit, am grija la header HTTP*/
					httpbody = strstr(recvbuf, "\r\n\r\n");
				    httpbody += 4; /* move ahead 4 chars */
				    dim_antet = httpbody - recvbuf ;
				}
				if (count == 0 && httpbody){
					memcpy(toSend, antet, dim_header);
					memcpy(toSend+dim_header, httpbody, n - dim_antet);
					toSend[dim_header + n - dim_antet] = '\0';

					int len = dim_header + n - dim_antet;
					sendAll(serv_socket, toSend, &len);
				}
				else {
					memcpy(toSend, antet, dim_header);
					memcpy(toSend+dim_header, recvbuf, n);
					toSend[dim_header + n] = '\0';

					int len = n + dim_header;
					sendAll(serv_socket, toSend, &len);
				}
				count ++ ;
				dim_antet = 0;

				if (n < BUFLEN - dim_header){

					memset(antet, 0, BUFLEN);

					break;
				}
			}
		}
	}
}

/* Selecteaza din lista de linkuri gasite intr-o pagina pe cele care sunt valie pt Everything si le trimite*/
void sendList(vector<string> wantedLinks, int sockfd){
	char sendbuf[BUFLEN]; 
	memset(sendbuf, 0, BUFLEN);
	string unified = "elst"; // elist

	for (int i = 0; i < wantedLinks.size(); i++){
		if (wantedLinks.at(i).find("http") != std::string::npos 
			|| wantedLinks.at(i).find("#") != std::string::npos){
			// || wantedLinks.at(i).find("html") != std::string::npos
			// || wantedLinks.at(i).find("htm") != std::string::npos) {
			continue;
		}
		else {
			unified += wantedLinks.at(i) + "|";
		}
	}
	memcpy(sendbuf, unified.c_str(), unified.size());
	sendbuf[unified.size()] = '\0';
	int len = BUFLEN;
	int nrsent = sendAll(sockfd, sendbuf, &len);
}

/* Selecteaza din lista de linkuri gasite intr-o pagina pe cele care sunt valie pt Recursive si le trimite*/
void sendRecursiveList(vector<string> wantedLinks, int sockfd, char* path, char* numePag){
	char sendbuf[BUFLEN]; 
	memset(sendbuf, 0, BUFLEN);
	string unified = "list";
	for (int i = 0; i < wantedLinks.size(); i++){
		if (wantedLinks.at(i).find("http") != std::string::npos || wantedLinks.at(i).find("#") != std::string::npos) {
			continue;
		}
		else {
			vector<string> splitted = split(wantedLinks.at(i), '/');
			if (splitted.at(splitted.size() - 1).find(".html") != std::string::npos){
				// cout << std::to_string(recLevel+1) + wantedLinks.at(i) << endl;
				unified += std::to_string(recLevel+1) + wantedLinks.at(i) + "|"; /* incrementez si nivelul de recursivitate*/
			}
		}
	}
	memcpy(sendbuf, unified.c_str(), unified.size());
	sendbuf[unified.size()] = '\0';
	int len = BUFLEN;
	int nrsent = sendAll(sockfd, sendbuf, &len);
}

/* Descarca site-ul si il trimite catre server*/
vector<string> fetchSite(char* numePag, char* resource, int sockfd, int serv_socket){
	char sendbuf[BUFLEN]; 
	char recvbuf[BUFLEN];
	char toSend[BUFLEN];
	char* antet = (char*)"site";
	memset(sendbuf, 0, BUFLEN);
	sprintf(sendbuf, "GET %s HTTP/1.0\n\n", resource);
	write(sockfd, sendbuf, strlen(sendbuf));
	
	char* tempFile = (char*) malloc (100);
	sprintf(tempFile, "%d_%d_temp_file.html",getpid(), globalCounter); /* fisier temporar pentru a cauta linkuri*/
	globalCounter++;

	int fd = open(tempFile, O_WRONLY|O_CREAT|O_TRUNC, 0644);
	vector<string> wantedLinks;
	while (1){
		memset(recvbuf, 0, BUFLEN);
		memset(toSend, 0, BUFLEN);
		int n = recvAll(sockfd, recvbuf, BUFLEN - 4);

		int nrwritten = write(fd, recvbuf, n); /* scriu datele in fisierul temporar*/

		memcpy(toSend, antet, 4);
		memcpy(toSend+4, recvbuf, n);
		toSend[n+4] = '\0';

		int len = n+4;
		int nrsent = sendAll(serv_socket, toSend, &len); /* trimit datele catre server */

		if (n < BUFLEN - 4){ /* la ultimul pachet trimis*/
			int remain = BUFLEN - len;
			char* padded = (char*) calloc (remain, sizeof(char));
			sendAll(serv_socket, padded, &remain);

			wantedLinks = getLinks(tempFile); /* caut linkurile*/
			if (everything == 1){ 
		    	sendList(wantedLinks, serv_socket); // doar daca everything ON
			}
			if (recursiv == 1 && recLevel <= 4){ // atentie -> nivel 2 - <= 1; nivel 5 -- <= 4
				sendRecursiveList(wantedLinks, serv_socket, resource, numePag);
			}

			unlink(tempFile); /* sterg fisierul temporar */
			break;
		}
	}
	close(fd);
	return wantedLinks;
}

int main(int argc, char *argv[])
{
    int sockfd, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char recv_buffer[BUFLEN];

    char buffer[BUFLEN];
    if (argc > 7) {
       fprintf(stderr,"./client [-o <fisier_log>] -a <adresa ip server> -p <port>\n");
       exit(0);
    }

    string logFile;
    char* port = (char*) malloc(100);
    char* adresa_ip = (char*) malloc(100);
    
    char* msg = (char*) malloc(BUFLEN);

    for (int i = 0; i < argc ; i++){ /* verific parametrii primiti */
   		if (strcmp(argv[i], "-o") == 0){
   			other = 1;
			
   			logFile = string(argv[i+1]);
   			int pid = getpid();
   			char* c_pid = (char*) malloc (100);
   			sprintf(c_pid, "%d", pid);
   			string info = logFile + "_" + c_pid + ".stdout";
   			string eror = logFile + "_" + c_pid + ".stderr";
			g_info = fopen (info.c_str(), "wt");
			g_error = fopen(eror.c_str(), "wt");
   		}
   		if (strcmp(argv[i], "-p") == 0){
   			port = argv[i+1];
   		}
   		if (strcmp(argv[i], "-a") == 0){
   			adresa_ip = argv[i+1];
   		}
   	}

    /*creaza un socket pe care se va comunica cu serverul*/ 
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        printErrMsg(other, (char*)"Error opening socket", g_error);
        exit(0);
    }

    /* pregateste structurile pentru adrese*/
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(port));
    inet_aton(adresa_ip, &serv_addr.sin_addr);
    
    /*conectare la server*/
    if (connect(sockfd,(struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0) {
        printErrMsg(other, (char*)"Error connecting", g_error);

        exit(0);
    }
    else {
    	memset(msg, 0, BUFLEN);
    	sprintf(msg, "M-am conectat la serverul cu ip %s, pe portul, %s", adresa_ip, port);
    	printInfoMsg(other, msg, g_info);
    }
    
    while(1){
    	
    	if (n = recv(sockfd, recv_buffer, sizeof(recv_buffer), 0) < 0){
        	printErrMsg(other, (char*)"ERROR receving from socket", g_error);
    		exit(0);
        }
        else {
        	if (strcmp(recv_buffer, "exit\n") == 0){ /* serverul s-a inchis*/
        		close(sockfd);
        		printInfoMsg(other, "Serverul s-a inchis! Se inchide clientul!", g_info);
        		break;
        	}
        	else { /* am primit o noua comanda*/
				char* numePag;
				char* calePag;
				recursiv = (recv_buffer[0] == '0') ? 0 : 1; /* determin daca e mod recursiv pornit*/
				everything = (recv_buffer[1] == '0') ? 0 : 1; /* determin daca e mod everything pornit*/
				recLevel = (int)recv_buffer[2] - 48; /* aflu nivelul de recursivitate al paginii*/

				decompose(recv_buffer+2, &numePag, &calePag); /* obtin numele site-ului si calea catre resursa*/

				memset(msg, 0, BUFLEN);
				sprintf(msg, "Am primit comanda sa downloadez site-ul: %s%s", numePag, calePag);
				printInfoMsg(other, msg, g_info);

				struct hostent *ghbn;
				struct in_addr **addr_list;
				/* Obtin adresa IP a serverului HTTP*/
				if (numePag[0] == 'w' && numePag[1] == 'w' && numePag[2] == 'w'){
					ghbn = gethostbyname(numePag+4); //obtin ip
    				addr_list = (struct in_addr **)ghbn->h_addr_list;
				}
				else {
					ghbn = gethostbyname(numePag); //obtin ip
    				addr_list = (struct in_addr **)ghbn->h_addr_list;
				}
				
    			char* ip_address = inet_ntoa(*addr_list[0]); /* convertesc la char* adresa ip*/

		    	int http_socket = connectToHTTP(ip_address); /* ma conectez la server HTTP*/

		    	vector<string> wantedLinks = fetchSite( numePag, calePag, http_socket, sockfd); /* obtin si trimit site + iau lista de linkuri*/

		    	memset(msg, 0, BUFLEN);
				sprintf(msg, "Am terminat de descarcat si trimis site-ul: %s%s", numePag, calePag);
				printInfoMsg(other, msg, g_info);

		    	if (everything == 1){ /* daca e Everything on, atunci trimit si toate celelate fisiere*/
					downloadEverything(wantedLinks, numePag, calePag, ip_address, sockfd);
		    	}

				send(sockfd, "exit", strlen("exit"), 0); /* anunt serverul ca acest client si-a terminat treaba*/
        	}
        }
    }
    return 0;
}


