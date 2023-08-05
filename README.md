Tema4 Pcom

## Compilare + Rulare:
Codul se compileaza ruland comanda:
    make 
Iar pentru executie:
    make run

## Biblioteca JSON
Am folosit parson ca biblioteca pentru a parsa string-urile de tip json. 
O data ce am facut tema in C iar asta era varianta recomandata si in enunt,
am decis ca aceasta este cea mai buna, altfel trebuia sa fac eu parsarea string-urilor venite de la tastatura, si sa am grija la spatii si la dimensiune 
ar fi fost horror.
Fisierele parson.c, parson.h si folderul parson tin de biblioteca json folosita si NU SUNT IMPLEMENTATE DE MINE: acesta este link-ul de la repository-ul de github:
https://github.com/kgabis/parson  

## Fisierul client.c
In client.c se afla logica programului, am o structura de tip struct client pe care
o aloc si ii initializez campurile cu NULL, pe baza acestor campuri stiu ce comenzi s-au realizat sau nu. Dupa folosesc un while loop si execut independent fiecare 
comanda in functie de input-ul pe care il primesc de la tastatura. In momentul in care rezolvam bugg-urile verificam ca programul sa stie cum sa se ocupe de comenzile gresite, de cele duplicate si de cele date in ordine gresite, ca de exemplu get_book inainte de login sau enter_library. Nu am folosit poll, de fiecare data cand execut o
comanda deschid un socket, trimit request-ul, primesc raspunsul si inchid socket-ul,
in ciuda overhead-ului creat consider ca este o varianta mai buna, deoarece cererile trimise catre server sunt gestionate in mod independent, iar serverul nu este fortat sa procesese dummy request pentru a extinde durata de viata a conexiunii, plus ca era mult mai usor de inteles si de implementat asa.

## Fisierele header: 
common.h --> destuld e usor de inteles, sunt functii de baza pe care le folosesc atat in client.c cat si in celelalte fisiere header,  acolo structura client:
struct client {
    char *username;
    char *password;
    char *JWT_token; 
    char *cookies;
};
char *cookies -- un string pe care il primesc in momentul in care fac login
char *JWT_token -- il primesc in momentul in care accesez biblioteca cu functia
de enter_library, 
Cele 2 campuri le voi da ca parametrii la fiecare cerere care va urma din momentul in care le primesc.
int open_connection(const char *host_ip, int portno, int ip_type, int socket_type, int flag) -- aceasta deschide socket-ul prin care trimit informatii catre server
Am reimplementat functiile de reverse si de itoa, dintr-un motiv sau altul nu functionau cele de biblioteca, probabil greseam ceva in apelarea lor sau nu mi le vedea vscode-ul, ca pur si simplu nu imi compila.

request.h --> aici sunt create headerele pentru fiecare request in parte
response.h --> aici am creat struct response, mi s-a parut o modalitate mult
mai ordonata pentru a gestiona si a accesa usor raspunsul de la server 
struct response
{
  char *header;
  int body_size;
  char *body; 
};

char *header-> va contine header-ul raspunsului de la server, practic, intreg continul pana "\r\n\r\n".
struct response* receive_from_server(int sockfd) - functia aceasta imi creeaza
raspunsul venit de server, citind toata informatia care se afla pe sockfd, 

## Conventii comenzi
Am considerat ca un utilizator nu poate da login in momentul in care este deja
logat. Daca vrea sa se conecteze cu alt utilizator, trebuie prima data sa dea logout
si dupa sa introduca username-ul si password-ul de la alt utilizator. 
La add_book, singurul lucru pe care il verific este numarul de pagini, ma asigur sa fie un numar si sa nu aiba mai mult de 5 cifre. Am considerat ca titlurile pot fi prea variate si compplexe pentru a gasi un criteriu, plus ca am vazut in user-ul test ca sunt date toate titlurile posbile. Deci cu atat mai greu ii in lipsa unui format prestabilit.
restul comenzilor ar trebui, cu exceptia lui register sa le poti da o singura data fara sa obtii eroare. In cazul lui delete si get, am lasat serverul sa intoarca mesajul de eroare, iar in cazul lui delete daca nu intoarce nimic iar codul nu are o valoare de eroare, inseamna ca stergerea s-a indeplinit cu succes. 

