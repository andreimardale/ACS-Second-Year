== README-Tema 3 -Protocoale de Comunicatii ==
== MARDALE ANDREI == 324 CB
== Web Crawler ==

===== Aplicatia SERVER =====
Pentru a implementa SERVER-ul, ideea de baza a fost ca acesta va multiplexa mereu, va alege intre a citi ceva sau a trimite ceva. In cazul in care se selecteaza citirea, atunci aceasta se poate face de la tastatura, pentru a primi comenzi de la utilizator sau se poate face pe socketul principal, cel inactiv, pe care se receptioneaza cererile de conectare sau pe unul din socketii activi, caz in care se primeste un mesaj / o bucata de site / o bucata de fisier de la unul din clienti. In cazul in care se selecteaza sa se trimita ceva, doar in cazul in care am cel putin 5 clienti conectati si am clientul care urmeaza sa "munceasca" e liber si am elemente de trimis (coada nu e goala), doar in acel caz, se pregateste si se trimite un link catre client.

MODUL NORMAL: serverul primeste o cerere de download de la tastatura, apoi obtine numele paginii si calea catre pagina, mapeaza link-ul ca fiind descarcat (il insereaza in opened cu true) si ataseaza linkului un numar. Pentru prima pagina e 1, deoarece nivelul de recursivitate implicit e 1. Apoi insereaza link-ul in coada de trimiteri. Cand se indeplinesc conditiile de trimitere, creeaza fisierul pe disc, trimite link-ul catre client si il insereaza intr-un map care retine in functie de socket-ul trimis, numele linkului si fd-ul lui. Bucatile de site care se intorc de la client, au ca antet "site" si astfel serverul stie ca e vorba de site-ul principal. Acum e momentul ca se scapa de header-ul de http, daca exista. Pentru verificare, cau "\r\n\r\n" si il skipuiesc. Dupa acest pas scriu pe disc fisierul html. La ultimul pachet, primit, mai fac un recv() suplimentar cu restul de octeti pana la dimensiunea maxima a bufferului, pt ca altfel, TCP mi-ar uni mesajul urmator cu acesta. (Analog, pe client trimit un mesaj gol, ca un padding pana la dimensiunea totala).

MODUL RECURSIV: Daca e modul recursiv pornit, pe server nu sunt multe diferente, el primeste de la client si o lista cu site-urile ce trebuie descarcate in continuare, lista cu antet "list". Pe baza acestei liste updateaza coada de linkuri.

MODUL EVERYTHING: In acest mod, deoarece am implementat tema inainte de a se posta pe forum informatia cu privire la acest mod, eu downloadez cu clientul caruia i-a fost asignat site-ul si toate fisierele si le trimit catre server. Astfel, dupa download-ul site-ului, se trimite o lista de fisire (pdf, zip etc) care vor fi downloadate. Aceasta lista are antet "elst" si in acest moment, serverul creeaza directoare si fisiere pentru toate elementele din lista. Apoi bucatile din fisier sunt precedate de antetul "evth". Serverul obtine pentru un anume fisier fd-ul asociat si scrie continutul la acel fd.

Functiile folosite pentru rezolvarea temei au fost:
1) vector<string> split(const string &text, char sep)  => functie ce imparte string-ul text in tokeni, dupa caracterul sep si intoarce un vector de tokeni. Aceasta functie e utila mai ales in procesarea path-urilor de genul /dir1/dir2/... .

2) int createFolders(char* numePag, char* calePag) => functie care creeaza structura de directoare pentru site-ul principal, cel descarcat initial si returneaza file descriptorul fisierului aferent acestuia de pe disc. In cadrul functiei, sparg calea catre resursa in tokeni, dupa "/" si cat timp in token nu apare ".html", practic, nu am ajuns la sfarsitul caii, creez directoare, intru in ele si tot asa. Cand am gasit fisierul html, il creez pe disc si ies din bucla. Daca nu s-a gasit un fisier html, am o solutie de rezerva, anume creez automat un fisier "index.html".

string unwrapName (char* name) => Functie care recupereaza numele dintr-un "header" de pachet trimis. Ideea e ca pentru a trimite fisiere (pdf, zip etc), eu pun un header de 100 octeti. Primii 4, sunt pentru a identifica tipul mesajului, "evth", urmatorii 96 sunt pentru a identifica resursa : "info.pdf", iar daca numele resursei e mai mic de 96 de caractere, se face padding pana la valoarea dorita. Aceasta functie obtine numele din acest antet.
void increment (int &count, vector<int> socket_clients) => Functie care incrementeaza contorul circular, pentru politica de round-robin. Cand ajung la dimensiunea listei de clienti, o iau de la 0, altfel cresc cu 1.
int sendAll(int s, char* buf, int* len) => Wrapper peste send(). Asigura trimitrea totala a datelor, deoarece am observat ca TCP nu garanteaza ca daca eu trimit n octeti, toti acestia vor ajunge exact asa la destinatie. Acest wrapper, forteaza trimiterea de send() pana numarul real de octeti trimisi este cel dorit.
int recvAll (int sockfd, char* buf, int len ) => Analog pentru recv().
void decompose(const char* link, char** nume_pagina, char** cale_pagina) => Functie care obtine pe baza unui link complet, numele site-ului si calea catre resursa.
string combinePaths(string path1, string path2) => Functie care combina doua path-uri care se pot suprapune. 
string removeBacks (string toQuerry) => Functie care aplica .. in cadrul unui path, adica sterge efectiv token-ul .. si tokenul de dinainte.
vector<string> unpackLinks(char* buffer) => Functie care obtine lista de link-uri din formatul codificat: link.html|link2.html|... , asa cum au fost ele trimise prin TCP.
int completeFolderPath(string download_file_path) => Functie care creeaza foldere pana ajunge la fisierul efectiv ce trebuie creat (e folosita in cazul everything).
void prepareFolders(vector<string> wantedLinks, char* numePag, char* path, unordered_map <string, int> *desc) => pe baza unei liste de linkuri cu fisiere ce vor fi descarcate, creeaza folderele si fisierele pe disc si mapeaza fd-urile intr-un map.
void updateQueue(queue<string> *links, vector<string> wantedLinks, char* numePag, char* path, unordered_map<string, bool> *opened) => updateaza coada de linkuri pe baza listei de linkuri primite in cazul recursive; pentru fiecare link, il corecteaza si il adauga cu nivel de recursivitate +1, de asemenea il marcheaza ca fiind prelucrat, pentru a evita descarcarea de mai multe ori a aceluiasi fisier.
void printErrMsg (int other, const char* message, FILE* g_error) => Functie auxiliara care afiseaza mesaje la stderr sau in fisier in functie de parametrul other.
void printInfoMsg(int other, const char* message, FILE* g_info) => Analog pentru stdout sau fisier.


===== Aplicatia CLIENT =====
Pentru aplicatia client nu am folosit multiplexare deoarece nu am considerat ca e necesar (un client nu poate primi alte comenzi in timp ce descarca ceva). Astfel, modul de gandire a fost urmatorul: Dupa stabilirea conexiunilor cu serverul, clientul poate primi doua tipuri de mesaje: mesaj "exit", cand serverul il anunta ca trebuie sa se inchida si mesaj cu un link. Daca e link, atunci obtine modul de lucru dorit (acest lucru a fost trimis de catre server) si nivelul de recursivitate al paginii. Dupa care obtin IP-ul serverului ce tine acel site, ma conectez la serverul HTTP si descarc site-ul. Daca e modul Everything pornit, atunci descarc si toate fisierele existene in acea pagina, dupa care trimit serverului un mesaj "exit", pentru ca acesta sa stie ca acest client si-a terminat treaba.

MODUL RECURSIVE: Ideea pentru a implementa acest mod a fost sa atasez fiecarui link, la inceput un numar, care indica nivelul de recursivitate al site-ului respectiv. Astfel, la fiecare site, atunci cand se retrimite lista de linkuri din acel site, este incrementat acel contor. Cand se ajunge la nivel maxim, nu se mai cauta linkuri in cadrul paginii respective. 

MODUL EVERYTHING: In acest mod de lucru, pe langa site-ul propriu zis, se va trimite si o lista de linkuri catre server cu elementele gasite. Dupa care se incepe trimiterea tuturor elementelor.  In implementearea mea, un client va descarca toate elementele din acea pagina gasita. (in enunt nu este restrictionat acest lucru si am vazut forumul prea tarziu).

Functii folosite: (multe din ele se repeta de la server)
int connectToHTTP(char* server_ip) => Realizeaza conexiunea la serverul de HTTP cu ip-ul dat. Returneaza socket-ul pe care se va face comunicarea.
vector<string> getLinks(char* resource) => Cauta in fisierul temporar resource, toate linkurile de genul href. Am ales sa salvez un fisier temporar pentru ca as fi putut avea probleme cu un buffer; se putea intampla sa am un link la sfarsit sa continue in cealalta bucata date.
void downloadEverything(vector<string> wantedLinks, char* numePag, char* path, char* ip_address, int serv_socket) => Functie care downlodeaza toate fisierele gasite din lista wantedLinks si le trimite catre server.
void sendList(vector<string> wantedLinks, int sockfd) => functie care trimite in cazul Everything lista de fisiere gasite in cadrul un site.
void sendRecursiveList(vector<string> wantedLinks, int sockfd, char* path, char* numePag) => Functie care trimite in cazul Recursive toate linkurile gasite intr-un site.
vector<string> fetchSite(char* numePag, char* resource, int sockfd, int serv_socket) => Aici are loc descarcarea efectiva a site-ului.
