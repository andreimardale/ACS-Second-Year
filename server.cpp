// Nume: MARDALE ANDREI
// Grupa: 324 CB
// Tema 2 Sistem de Backup si Partajare a Fisierelor
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

#define MAX_CLIENTS	10
#define BUFLEN 4096

using namespace std;

/* clasa care retine informatiile importante despre un user */
class Info {
  public:
  	string password; /* parola userului*/
    vector<string> sharedFiles; /*lista de fisiere share-uite*/
    vector<string> privateFiles; /*lista de fisiere private*/
    

  	Info (string p){
  		password = p;
  	}
  	Info (string p, vector<string> f){
  		password = p;
  		sharedFiles = f;
  	}

  	/* functie care returneaza true daca un fisier este Shared*/
    bool findInShared (string file){
    	for (int i = 0; i < sharedFiles.size(); i++){
    		if (sharedFiles[i] == file){
    			return true;
    		}
    	}
    	return false;
    }
	/* functie care returneaza true daca un fisier este Private*/
    bool findInPrivate (string file){
    	for (int i = 0; i < privateFiles.size(); i++){
    		if (privateFiles[i] == file){
    			return true;
    		}
    	}
    	return false;
    }
    /* functie care seteaza un fisier ca Shared*/
    void set_Shared (string file){
    	sharedFiles.push_back(file); /* introduc elementul in lista de Shared*/
    	vector<string>::iterator result = find(privateFiles.begin(), privateFiles.end(), file);
    	if (result == privateFiles.end()){
    		cout <<"That file is not in there" << endl;
    	}
    	else {
    		privateFiles.erase(result); /* scot elementul din lista de Private*/
    	}
    }
    /* functie care seteaza un fisier ca Private*/
    void unset_Shared (string file){
    	vector<string>::iterator result = find(sharedFiles.begin(), sharedFiles.end(), file);
    	if (result == sharedFiles.end())
        	cout << "That file is not in there!" << endl;
    	else{
        	sharedFiles.erase(result); /* scot elementul din lista de elemente shared*/
        	privateFiles.push_back(file); /* il introduc in lista de elemente private*/
    	}
    }
};

unordered_map <int, string> logged_users; /* mapez socket-ul unei conexiuni cu user-ul logat*/

bool TO_QUIT = false; /* retine daca s-a dat comanda QUIT (in timpul unui transfer)*/
bool BLOCK_CMD = false; /* daca este true, atunci nu se mai pot accepta comenzi*/

/* verifica daca un caracter este alfanumeric */
bool is_not_alnum_space(char c) {
    return !(isalpha(c) || isdigit(c) || (c == ' '));
}

/* verifica corectitudinea unui nume*/
bool checkName (string name){
	if (name.size() > 24){
		return false; /* dimensiune mai mica decat 24*/
	}
	return find_if(name.begin(), name.end(), is_not_alnum_space) == name.end(); /* sa aiba doar caractere alfanumerice*/
}

/* Functie care imparte un string dupa un separator*/
vector<string> split(string &text, char sep) {
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

/* Functie care populeaza lista de useri*/
void populateUsers (char* users_config_file, unordered_map<string, Info*> *data, vector<string> *usr){
    int n, i;
    string user, password;
    ifstream f (users_config_file);
    f >> n;
    for (i = 0; i < n; i++){
    	f >> user >> password;
    	if (checkName (user) == false){ /* daca username-ul este valid */
    		cout << "Username " << user<<" invalid!" << endl;
    		continue;
    	}
    	if (checkName(password) == false){ /* daca parola este valida */
    		cout << "Password " << password<<" invalid!" << endl;
    		continue;	
    	}
    	if ( (*data).find(user) != (*data).end() ) { /* daca numele nu este duplicat */
    		cout << "Nume duplicat " << user <<endl;
    		continue;
    	}

    	(*data).insert (make_pair (user, new Info(password))); /* atunci insereaza o pereche de tipul (user, Info)*/
    	(*usr).push_back(user); /* insereaza user-ul si in lista de useri*/
    }
    f.close(); /*inchide fisierul */
}

/* Functie care populeaza lista de fisiere*/
void populateFiles (char* shared_files, unordered_map<string, Info*> *data){
	int n, i;
	string line;
	string user, file;
	ifstream f (shared_files);
	f >> n;
	getline(f,line); // un fel de flush
	for (i = 0; i < n; i++){
		getline(f, line);
		vector<string> v = split (line, ':'); /*fac split dupa caracterul :*/
		user = v.at(0); /*primul element este username-ul*/
		file = v.at(1); /*al doilea caracter este fisierul partajat*/
		if ((*data).find(user) == (*data).end()){ /*daca userul exista, continui*/
			cout << "User inexistent! "<< endl;
			continue;
		}
		string path = "./" + user + "/" + file; /* construiesc path-ul catre fisier*/

		if (access (path.c_str(), F_OK) != -1){ /* verifica daca fisierul exista*/
			Info* my = (*data)[user];
			(my->sharedFiles).push_back(file); /*populez lista de fisiere Share-uite din cadrul clasei Info*/
			(*data).insert (make_pair (user, my));
		}
		else {
			cout << "Fisier inexistent! "<< endl;
			continue;
		}
	}
	f.close();
}

/*Functie care populeaza lista de fisiere private*/
void populatePrivateFiles (unordered_map<string, Info*> *data, vector<string> usr){
	for (int i = 0; i < usr.size(); i++){ /* pentru toti userii, verific directoarele home*/
		string user = usr.at(i);
		DIR *dp = NULL;
		struct dirent *dptr = NULL;
		string path = user + "/";
		string result;
		if (NULL == (dp = opendir(path.c_str()))) {
	    	perror("Nu pot deschide directorul");
		}
		else { /* si orice fisier gasit care nu este Share-uit, devine private*/
			while(NULL != (dptr = readdir(dp)) ){
				Info* my = (*data)[user];
				if (dptr -> d_type == DT_REG) {
					if (my->findInShared(dptr->d_name) == false)
						(my->privateFiles).push_back(dptr->d_name);
				}
				(*data).insert(make_pair(user,my));
			}
		}
	}	
}

/* 1-> nu exista user ; 2-> Parola gresita; 0 -> conexiune acceptata */
int checkLogin (string user, string pwd, unordered_map <string, Info*> data, int socket){
	if (data.find(user) == data.end()){
		cout<<"User inexistent!" << endl;
		return 2;
	}
	else {
		Info* info = data[user];
		if (pwd == info->password){
			cout<<"Conexiune acceptata!" << endl;
			// LOGGED_USER = user;
			logged_users[socket] = user;
			return 0;
		}
		else {
			cout << "parola gresita" << endl;
			return 1;
		}
	}
}

/*Functie care creeaza directoarele home, la inceput, daca acestea nu exista*/
void createFolders(vector<string> usr){
	for(vector<string>::iterator it = usr.begin(); it != usr.end(); ++it) {
		mkdir((*it).c_str(), 0777);		
	}
}

/*Functie care determina dimensiunea unui fisier*/
unsigned long fsize(const char *filename) {
    struct stat st; 

    if (stat(filename, &st) == 0)
        return st.st_size;

    return -1; 
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

/*Functie care determina informatii despre fisierele dintr-un director (dimensiunea + starea)*/
string fileInfo (string user, unordered_map <string, Info*> data){
	DIR *dp = NULL;
	struct dirent *dptr = NULL;
	string path = user + "/"; /* construiesc calea catre directorul home*/
	string result;
	if (NULL == (dp = opendir(path.c_str()))) {
	    perror("Nu pot deschide directorul"); /* Eroare daca nu se poate deschide directorul*/
	}
	else {
		while(NULL != (dptr = readdir(dp)) ){
			if (dptr -> d_type == DT_REG) { /* daca este de tipul fisier*/
				result = result + dptr->d_name; 
				string filePath = "./" + path + dptr->d_name; /*construiesc calea catre fisier*/
				result = result + "\t" + prettyFormat(fsize(filePath.c_str())) + " bytes"; /*afisez dimensiunea*/
				Info* my = data[user];
				if (my -> findInShared(dptr->d_name) == true){
					result = result + "\t SHARED \n"; /*afisez daca e privat sau shared*/
				}
								  
				else {
					result = result + "\t PRIVATE \n";
				}
			}
		}
		closedir(dp); /*inchid directorul*/
	}
	result = result.substr(0, result.size()-1); /* scap de ultimul \n */

	return result;	
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

/*Incrementeaza contorul pentru politica de round-robin*/
void increment (int &count, vector <tuple<string,int, int>> sending){
	if (count == sending.size() - 1){ /* incrementearea este circulara-> daca am ajuns la dim. listei => 0*/
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
        // printf("%d \n", n );
        if (n == -1){ 
	    	printf("%s\n", strerror(errno));

            break;
        }
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


int main(int argc, char *argv[])
{	
    int sockfd, newsockfd, portno; /* elemente necesare pentru setarea conexiunii TCP*/
    unsigned int clilen;

    char buffer[BUFLEN]; /* buffer pentru date receptionate*/
    char sendBuffer[BUFLEN]; /* buffer pentru date trimise*/
    char* antet = (char*) calloc(BUFLEN, sizeof(char)); /* buffer pentru antet pentru download*/
	char* toSend = (char*) calloc(BUFLEN, sizeof(char)); /* buffer pentru datele trimise prin download */

    struct sockaddr_in serv_addr, cli_addr; /*structurile pentru socket*/
    int n, i; /* contori */
    int count = 0; /* contor pentru politica round robin*/

    char recv_buffer[BUFLEN]; /*buffer pentru comenzi receptionate */
    char upload_buf[BUFLEN]; /* buffer pentru pachetele trimise prin download*/

   
    unordered_map <string, Info*> data; /*map-ul care mapeaza un username cu o instanta a clasei Info*/
    vector<string> usr; /* lista de useri */
    vector<string> receving; /* lista de fisiere ce sunt in primire */
    unordered_map <string, int> descriptor; /* mapeaza un nume de fisier cu un descriptor -> pt upload*/
    unordered_map <int, int> download_socket; /* mapeaza un socket cu un fd */
	vector<tuple<string, int, int >> for_dld; /*vectoru cu tuplutir de tipul fileName, socket, fileDescriptor -> pt download*/
	vector<int> clients; /*lista cu socket-urile clientilor -> pentru a trimite mesaj global, tuturor clientilor*/

    fd_set read_fds;	//multimea de citire folosita in select()
    fd_set tmp_fds;	//multime folosita temporar
    fd_set write_fds; // multime de scriere folosita in select()
    fd_set tmp_wr_fds; // multime folosita temporar pentru scriere

    int fdmax;		//valoare maxima file descriptor din multimea read_fds
    if (argc != 4) {
       fprintf(stdout,"Usage : ./server <port_server> <users_config_file> <static_shares_config_file>  \n");
       exit(1);
    }

    populateUsers(argv[2], &data, &usr); /* populeaza users*/
    createFolders(usr); /*creaza folderele la inceput, daca nu exista deja*/
    populateFiles(argv[3], &data); /* populeaza shared_files*/
    populatePrivateFiles(&data, usr); /*populeaza fisierele private*/

    /* golim multimea de descriptori */
    FD_ZERO(&read_fds);
    FD_ZERO(&tmp_fds);
    FD_ZERO(&write_fds);
    FD_ZERO(&tmp_wr_fds);
    
    /*creaza un socket pe care se vor primi conexiuni*/ 
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
    	fprintf(stderr,"ERROR opening socket");
       exit(1);
    }
     
    portno = atoi(argv[1]);

    /*seteaza structurile pentru conexiunea TCP*/
    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;	// foloseste adresa IP a masinii
    serv_addr.sin_port = htons(portno);
     
    /*fac bind pe socket-ul anterior deschis*/ 
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(struct sockaddr)) < 0) {
    	fprintf(stderr,"ERROR on binding");
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
    int bforce[fdmax]; /*vector care numara cate incercari de logare esuate au existat*/

     // main loop
	while (1) {
		tmp_fds = read_fds; 
		tmp_wr_fds = write_fds;
		if (select(fdmax + 1, &tmp_fds, &tmp_wr_fds , NULL, &tv) == -1) {
			fprintf(stderr,"ERROR in select");
        	exit(1);
		}
	
		for(i = 0; i <= fdmax; i++) {
			if (FD_ISSET(i, &tmp_fds)) {
				if (i == sockfd) {
					// a venit ceva pe socketul inactiv(cel cu listen) = o noua conexiune
					// actiunea serverului: accept()
					clilen = sizeof(cli_addr);
					if ((newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen)) == -1) {
						fprintf(stderr,"ERROR in accept");
        				exit(1);	
					} 
					else {
						//adaug noul socket intors de accept() la multimea descriptorilor de citire
						FD_SET(newsockfd, &write_fds);
						FD_SET(newsockfd, &read_fds);
						clients.push_back(newsockfd); // adaug socket-ul in lista de clienti
						if (newsockfd > fdmax) { 
							fdmax = newsockfd;
						}
					}
					bforce[newsockfd] = 0;
					printf("Noua conexiune de la %s, port %d, socket_client %d\n ", inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port), newsockfd);
				}
				else if (i == STDIN_FILENO){ /* a venit un mesaj de la tastatura */
					memset (recv_buffer, 0, BUFLEN); /*curat buffer-ul*/
    				fgets(recv_buffer, BUFLEN-1, stdin); /* preiau mesajul */
    				if( strcmp(recv_buffer, "quit\n") == 0 ){ /*daca e quit*/
    					if (for_dld.size() == 0 && receving.size() == 0){ /* daca nu exista transferuri in curs*/
    						for (int k = 0; k < clients.size(); k++){ /*atentioneaza toti clientii ca se inchide serv.*/
	    						int skt = clients.at(k);
	    						send(skt, recv_buffer, sizeof(buffer), 0);
	    					}
    						exit(0); /* se inchide serverul */
    					}
    					else { /*daca exista transferuri in curs de desfasurare*/
    						BLOCK_CMD = true; /*eventuale comenzi sunt blocate */
    						TO_QUIT = true; /*se seteaza variabila care anunta ca exista o cerere de inchidere a serverului*/
    					}
    				}
				}					
				else {
					// am primit date pe unul din socketii cu care vorbesc cu clientii
					//actiunea serverului: recv()
					memset(buffer, 0, sizeof(buffer)); /*golesc bufferul*/
					if ((n = recvAll(i, buffer, sizeof(buffer))) <= 0) {
						if (n == 0) {
							//conexiunea s-a inchis
							printf("selectserver: socket %d hung up\n", i);
						} else {
							fprintf(stderr,"ERROR in recv");
        					exit(1);
						}
						close(i); 
						FD_CLR(i, &read_fds); // scoatem din multimea de citire socketul pe care 
						FD_CLR(i, &write_fds); // scoatem din multimea de scriere socketul deconectat
					} 
					else { //recv intoarce >0
						if (strncmp(buffer, "piece", 5) == 0){ /* daca este o bucata de fisier dintr-un upload*/
							char cmd[6];
							char file[41];
							char dim[5];

							strncpy(cmd, buffer, 5); /*refac comanda -> "piece"*/
							cmd[5] = '\0';
							
							strncpy(file, buffer+5, 40);
							file[40] = '\0';
							string fisier = unwrapName(file); /* refac numele fisierului ce este transferat*/
							
							strncpy(dim, buffer+45, 4); /*determin numarul de octeti utili din acest pachet primit*/
							dim[5] = '\0';
							
							int sz;
							sscanf(dim, "%d", &sz); /*convertesc dimensiunea din char in int*/

							char* continut = (char*) malloc (sz+1); 
							memcpy(continut, buffer+49, sz); /*obtin continutul efectiv al fisierului*/


							int fd = descriptor[string(fisier)]; /*obtin filedescriptorul fisierul primit*/

							if (strcmp(dim, "done") == 0){ /* ultimul pachet trimit e de atentionare -> dimensiunea e "done"*/
								vector<string >::iterator result = find(receving.begin(), receving.end(), string(fisier));
								receving.erase(result); /*scot fisierul din lista de fisiere in curs de receptionare*/
								close(fd); /* inchid fisierul nou creat pe server*/

								string path = "./" + logged_users[i] + "/" + fisier.c_str(); /*obtin calea catre fisier*/
								memset(sendBuffer, 0, BUFLEN);
								sendBuffer[0] = 'L'; /* trimit clientului mesaj de finis impreuna cu numele fisierului si dimensiunea lui*/
								sprintf(sendBuffer+1, "Upload finished: %s - %s bytes ", fisier.c_str(), prettyFormat(fsize(path.c_str())).c_str());
								send(i, sendBuffer, sizeof(sendBuffer), 0);
								
								if (TO_QUIT == true && receving.size() == 0 && for_dld.size() == 0){ /* daca a aparut intre timp o comanda QUIT*/
									for (int k = 0; k < clients.size(); k++){ /* dupa ce s-a terminat si ultimul transfer de facut => se inchide serverul*/
	    								int skt = clients.at(k); /* se atentioneaza toti clientii de intentia de inchidere*/
	    								send(skt, recv_buffer, sizeof(buffer), 0);
	    							}
	    							exit(0); /* se inchdie serverul */
								}
								break;
							}

							write (fd, continut, sz); /* se scrie in fisier payload-ul primit */
							memset(continut, 0, sz+1); /*resetez buffer-ul de continut*/
						}
						else if (BLOCK_CMD == false) { // daca nu blochez comenzi
							string income = string(buffer);
							income.erase(remove(income.begin(), income.end(), '\n'), income.end()); /* elimin eventuale \n*/
							vector<string> splitted = split(income, ' '); /* fac split dupa spatii libere*/
							
							if (splitted.at(0) == "login") { /* daca am o comanda login*/
								if (splitted.size() != 3){ /* daca nu am toate elementele -> login user parola => fail*/
									cout << "wrong input " << endl;
									break;
								}
								string u = splitted.at(1); /* determin usernameul*/
								string pwd = splitted.at(2); /* determin parola */
								int res = checkLogin(u, pwd, data, i); /* verific datele primite*/
								if (res == 1 || res == 2){ /* daca am cod 1 sau 2 => incercare esuata*/
									bforce[i]++; /*cresc contorul de incercari esuate aferent socket-ului i*/
									if (bforce[i] == 3){ /* daca s-a ajuns la 3 incercari -> Eroare*/
										sprintf(sendBuffer, "8"); /* trimit clientului codul 8*/
										send (i, sendBuffer, sizeof(buffer), 0);
										close(i); /* si inchid conexiunea*/
										FD_CLR(i, &read_fds); /* scoatem din multimea de citire socketul deconectat*/
										FD_CLR(i, &write_fds); // scoatem din multimea de scriere socketul deconectat
									}
									else {
										sprintf(sendBuffer, "3"); /* daca inca nu s-a ajuns la 3 incercari gresite*/
										send (i, sendBuffer, sizeof(buffer), 0); /* trimit mesaj cu user/parola gresita */
									}
								}
								else if (res == 0) { /* daca login-ul a fost in regula*/
									sprintf(sendBuffer, "0");
									send (i, sendBuffer, sizeof(buffer), 0); /* instiintez clientul ca s-a logat cu succes*/
									bforce[i] = 0;
								}	
							}
							else if (splitted.at(0) == "logout"){ /* daca am comanda logout*/
								logged_users.erase(i); /* sterg userer-ul asociat socket-ului curent din lista de useri logati*/
							}
							else if (splitted.at(0) == "getuserlist"){ /* comanda getuserlist*/
								string v;
								/* parcurg vectorul de useri si obtinu rezultatul*/
								for(vector<string>::iterator it = usr.begin(); it != usr.end(); ++it) {
		  							v = v + (*it);
		  							v = v + "\n";
								} 
								v = v.substr(0, v.size()-1); /* determin fisierul */

								memset(sendBuffer, 0, BUFLEN);
								sendBuffer[0] = 'l'; /*daca prima litera e 'l' -> clientul va sti ca e vorba de rezultat la comanda getuserlist*/
								sprintf(sendBuffer+1, "%s", v.c_str());
								send (i, sendBuffer, sizeof(sendBuffer), 0);
							}
							else if (splitted.at(0) == "getfilelist"){ /* comanda getfilelist*/
								if (data.find(splitted.at(1)) == data.end()){ /* daca nu exista utilizatorul*/
									memset(sendBuffer, 0, BUFLEN);
									sendBuffer[0] = 'F'; /* mini-antet pentru rezultatul comenzii getfilelist */
									sprintf(sendBuffer+1, "-11 Utilizator inexistent! "); /* trimit mesaj de eroare*/
									send(i, sendBuffer, sizeof(sendBuffer), 0);
								}
								else {
									string toSend = fileInfo(splitted.at(1), data); /* obtin date relevante despre fisierele userului solicitat*/
									memset(sendBuffer, 0, BUFLEN);
									sendBuffer[0] = 'F';
									sprintf(sendBuffer+1, "%s" , toSend.c_str());
									send(i, sendBuffer, sizeof(sendBuffer), 0);
								}		
							}
							else if (splitted.at(0) == "share"){ /* comanda SHARE*/
								string file1 = string (buffer + 6); /* determin fisierul share-uit*/
								string file = file1.substr(0, file1.size()-1); /* offset 6-> nr de litere S-H-A-R-E*/

								Info* my = data[logged_users[i]]; 
								string path = "./" + logged_users[i] + "/" + file; /* obtin calea catre fisierul share-uit*/
								if (access (path.c_str(), F_OK) == -1){ /* daca fisierul nu exista */
									memset(sendBuffer, 0, BUFLEN);
									sendBuffer[0] = 'S'; /* mini antet pentru comanda (S)hare*/
									sprintf(sendBuffer+1, "-4 Fisier inexistent! ");  /*trimit mesaj de eroare*/
									send(i, sendBuffer, sizeof(sendBuffer), 0);
								}
								else if (my->findInShared(file) == true){ /* daca fisierul exista dar e partajat deja*/
									memset(sendBuffer, 0, BUFLEN);
									sendBuffer[0] = 'S';
									sprintf(sendBuffer+1, "-6 Fisierul este deja partajat! "); /*eroare aferenta*/
									send(i, sendBuffer, sizeof(sendBuffer), 0);
								}
								else {
									my->set_Shared(file); /* daca fisierul nu era partajat deja, il partajez acum*/
									data.insert (make_pair (logged_users[i], my));
									memset(sendBuffer, 0, BUFLEN);
									sendBuffer[0] = 'S';
									sprintf(sendBuffer+1, "200 Fisierul a fost partajat! "); /* mesaj de confirmare*/
									send(i, sendBuffer, sizeof(sendBuffer), 0);
								}
							}
							else if (splitted.at(0) == "unshare"){ /* comanda UNSHARE*/
								string file1 = string (buffer + 8); 
								string file = file1.substr(0, file1.size()-1); /* determin fisierul */

								Info* my = data[logged_users[i]];
								string path = "./" + logged_users[i] + "/" + file; /* compun calea catre fisier*/
								if (access (path.c_str(), F_OK) == -1){ /* determin daca exista acel fisier*/
									memset(sendBuffer, 0, BUFLEN);
									sendBuffer[0] = 'U'; /*mini antet pentru comanda (U)nshare*/
									sprintf(sendBuffer+1, "-4 Fisier inexistent! ");
									send(i, sendBuffer, sizeof(sendBuffer), 0);
								}
								else if (my->findInShared(file) == false){ /*daca fisierul este deja privat*/
									memset(sendBuffer, 0, BUFLEN);
									sendBuffer[0] = 'U';
									sprintf(sendBuffer+1, "-7 Fisierul este deja privat! "); /*trimit direct mesaj de eroare*/
									send(i, sendBuffer, sizeof(sendBuffer), 0);
								}
								else {
									my->unset_Shared(file); /*setez fisierul ca fiind privat*/
									data.insert (make_pair (logged_users[i], my));
									memset(sendBuffer, 0, BUFLEN);
									sendBuffer[0] = 'U';
									sprintf(sendBuffer+1, "200 Fisierul a fost setat ca PRIVATE! "); /*trimit mesaj de confirmare*/
									send(i, sendBuffer, sizeof(sendBuffer), 0);
								}
							}
							else if (splitted.at(0) == "upload"){ /* comanda UPLOAD*/
								string file1 = string (buffer + 7); /*determin numele fisierului*/
								string file = file1.substr(0, file1.size()-1); /* elimin ultium \n*/

								Info* my = data[logged_users[i]];
								string path = "./" + logged_users[i] + "/" + file; /* determin calea catre fisier*/
								
								if (access (path.c_str(), F_OK) != -1){ // fisierul exista deja
									memset(sendBuffer, 0, BUFLEN);
									sendBuffer[0] = 'u'; // prima litera u => mesaj despre upload
									sendBuffer[1] = '0';
									sprintf(sendBuffer+2, "-9 Fisier existent pe server!");
									send(i, sendBuffer, sizeof(sendBuffer), 0);
								}
								else {
									memset(sendBuffer, 0, BUFLEN);
									sendBuffer[0] = 'u'; /* mesaj care incepe cu u1 */
									sendBuffer[1] = '1'; /* inseamna ca upload-ul se poate face cu succes*/


									int fd = open(path.c_str(), O_WRONLY|O_CREAT| O_APPEND, 0644); /* deschid fisierul pentru scriere*/
									descriptor[file] = fd; /* mapez numele fisierului cu un file descriptor */

									sprintf(sendBuffer + 2, "%s", file.c_str());
									send(i, sendBuffer, sizeof(sendBuffer), 0); /* instiintez clientul ca poate incepe sa trimita data*/
		    						receving.push_back(file); /* adauga fisierul in lista de upload*/
								}
							}
							else if (splitted.at(0) == "delete"){ /* comanda DELETE*/
								string file1 = string (buffer + 7); /* determin numele fisierului */
								string file = file1.substr(0, file1.size()-1); /* elimin \n*/

								int k;
								string path = "./" + logged_users[i] + "/" + file; /* determin calea catre fisier */
								bool found = false; 
								for (k = 0; k < receving.size(); k++){ /* verific daca fisierul este in curs de upload*/
									if (receving.at(k) == file){
										memset(sendBuffer, 0, BUFLEN);
										sendBuffer[0] = 'd';
										sprintf(sendBuffer+1, "-10 File is busy! "); /* trimit eroare catre client*/
										send(i, sendBuffer, sizeof(sendBuffer), 0);
										found = true;
									}
								}
								for (k = 0; k < for_dld.size(); k++){ /* verific daca fisierul este in curs de download*/
									if ((get<0> (for_dld.at(k))) == file){
										memset(sendBuffer, 0, BUFLEN);
										sendBuffer[0] = 'd';
										sprintf(sendBuffer+1, "-10 File is busy! "); 
										send(i, sendBuffer, sizeof(sendBuffer), 0);
										found = true;
									}
								}
								if (found == false) { /* daca fisierul nu e in transfer*/
									unlink(path.c_str()); /* sterg fisierul de pe disc*/
									memset(sendBuffer, 0, BUFLEN);
									sendBuffer[0] = 'd';
									sprintf(sendBuffer+1, "200 Fisier sters! "); /* instiintez clientul */
									send(i, sendBuffer, sizeof(sendBuffer), 0);
								}
							}
							else if (splitted.at(0) == "download"){ /* comanda DOWNLOAD*/

								string user = splitted.at(1); /* determin userul ce detine acel fisier */
								int dim_user = user.size(); 

								string file1 = string (buffer + 10 + dim_user); /* determin numele fisierului */
								string file = file1.substr(0, file1.size()-1); /* elimin \n de la sfarsit */

								Info* my = data[user];

								string path = "./" + user + "/" + file; /* determin calea catre fisier */

								if (access(path.c_str(), F_OK) == -1){ // fisierul nu exista
									memset(sendBuffer, 0, BUFLEN);
									sendBuffer[0] = 'D'; // prima litera D => mesaj despre download
									sendBuffer[1] = '0';
									sprintf(sendBuffer+2, "-4 Fisier inexistent!");
									send(i, sendBuffer, sizeof(sendBuffer), 0);
								}
								else {
									if (my->findInPrivate(file) == true && user != logged_users[i]){ // fisierul e privat
										memset(sendBuffer, 0, BUFLEN);
										sendBuffer[0] = 'D'; // prima litera D => mesaj despre download
										sendBuffer[1] = '1';
										sprintf(sendBuffer+2, "-5 Descarcare interzisa!");
										send(i, sendBuffer, sizeof(sendBuffer), 0);
									}
									else {
										memset(sendBuffer, 0, BUFLEN);
										sendBuffer[0] = 'D'; /* D2 -> cod pentru download acceptat */
										sendBuffer[1] = '2';
										
										sprintf(sendBuffer + 2, "%s %s", user.c_str() , file.c_str());
										send(i, sendBuffer, sizeof(sendBuffer), 0);
									}
								}
							}
							else if (splitted.at(0) == "start"){ /* cod pentru START DOWNLOAD*/
								string user = splitted.at(1); /* determin posesorul fisierului*/
								int dim_user = user.size();

								if (user == "@") /* substitui '@' cu numele user-ului*/
									user = logged_users[i];

								string file = string (buffer + 7 + dim_user); /* determin fisierul ce se vrea downloadat*/

								Info* my = data[user];
								string path = "./" + user + "/" + file; /* obtin calea catre fisier */

								int fd = open(path.c_str(), O_RDONLY); /* deschid fisierul pentru citire*/

								/* introduc un tuplu de forma : (nume_fisier, socket, fd) in lista de download */
								for_dld.push_back(make_tuple(file, i, fd)); 
							}
							else if (splitted.at(0) == "quit"){ /* comanda QUIT */
								vector<int>::iterator result = find(clients.begin(), clients.end(), i);
								clients.erase(result); /* se sterge clientul respectiv din lista de clienti*/
								close(i);  /* se inchide conexiunea catre el*/
								FD_CLR(i, &read_fds); // scoatem din multimea de citire socketul pe care 
								FD_CLR(i, &write_fds);
							}
						}
					}
				} 
			}
			else if (tv.tv_usec == 0 && FD_ISSET(i, &tmp_wr_fds)){ /* a venit timpul sa trimit o bucata de fisier catre un client */
				if (i != sockfd){  
					if (for_dld.size() != 0){ /* daca exista ceva de trimis in lista de trimiteri */
						string file = get<0> (for_dld.at(count)); /* determin numele fisierului de trimis*/
						int skt = get<1> (for_dld.at(count)); /* socketul pe care se doreste transmisia*/
						int fd = get<2> (for_dld.at(count)); /* file-descriptorul fisierului din care se citesc date*/

						string bkf = file; /* back-up pentru numele fisierului */

						/* dimensiunea antet e standard: 5 bytes pt 'piece' , 40 pt numele fisierului si 4 pentru dim.*/
						int dim_antet = 49; 

						int m = read (fd, upload_buf, BUFLEN - dim_antet); /* citesc din fisier date*/

						if (m <= 0) { /* daca s-a terminat de citit din fisier*/
							char antet[BUFLEN]; /*pregatesc un mesaj special*/
							string pad = bkf.append(40 - bkf.size(), '@'); // PADDING EFICIENT

							sprintf(antet, "piece%sdone", pad.c_str()); /* in loc de dimensiune, voi avea cuvantul DONE*/

							int len = BUFLEN;
							sendAll(skt, antet, &len); /* folosesc wrapper-ul pentru a trimite pachetul*/
							// send(skt, antet, BUFLEN, 0);

							vector <tuple<string, int, int>>::iterator result = find(for_dld.begin(), for_dld.end(), make_tuple(file, skt, fd));
							for_dld.erase(result); /* elimin tuplul respectiv din lista de download*/

							close(fd); /* inchid fisierul sursa */
							count = 0; /* resetez contorul pentru round robin*/

							if (TO_QUIT == true && receving.size() == 0 && for_dld.size() == 0){ /* verific daca s-a dat o comanda de QUIT intre timp*/
								for (int k = 0; k < clients.size(); k++){ /* atentionez toti clientii ca se va inchide serverul*/
	    							int skt = clients.at(k);
	    							send(skt, recv_buffer, sizeof(buffer), 0);
	    						}
	    						exit(0); /* inchid serverul*/
							}
						}
						else { /* daca mai sunt date de citit din fisier*/
							string pad = file.append(40 - file.size(), '@'); // PADDING EFICIENT

							sprintf(antet, "piece%s%04d", pad.c_str(), m); // pun ca prefix: piece_numefisier_cattrimit

							memcpy(toSend, antet, dim_antet); /* pregatesc antetul in buffer-ul de trimitere */
							memcpy(toSend + dim_antet, upload_buf, m); /* adau si informatia din fisier */
							toSend[dim_antet + m] = '\0'; /* pun terminatorul*/

							int len = BUFLEN;

							sendAll(skt, toSend, &len); /* trimit pachetul folosind wrapperul, pentru siguranta*/

							/*resetez bufferele*/
							memset(antet, 0, BUFLEN);
							memset(upload_buf, 0, BUFLEN);
							memset(toSend, 0, BUFLEN);

							/*incrementez contorul pentru politica de round-robin*/
							increment(count, for_dld);
						}
					}
				}
				tv.tv_usec = 90; /* resetez timerul pentru select */
			}
		}
    }
    close(sockfd); /* inchid socket-ul principal*/
    return 0; 
}


