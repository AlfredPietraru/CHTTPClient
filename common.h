#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define HEADER_LEN 1024
#define BUFFER_LEN 4096

struct client {
    char *username;
    char *password;
    char *JWT_token;
    char *cookies;
};

struct client *init_client() {
    struct client* my_client = malloc(sizeof(struct client));
    my_client->cookies = NULL;
    my_client->JWT_token = NULL;
    my_client->password = NULL;
    my_client->username = NULL;
    return my_client; 
}

void free_client(struct client *my_client) {
  if (my_client->cookies != NULL) {
    free(my_client->cookies);
  }
  if (my_client->JWT_token != NULL) {
    free(my_client->JWT_token);
  }
  if (my_client->username != NULL) {
    free(my_client->username);
  }
  if (my_client->password != NULL) {
    free(my_client->password);
  }
  free(my_client);
}


int recv_all(int sockfd, char *buffer, int bytes_to_receive) {
    int current_bytes_received = 0;
    int total_bytes_received = 0;
    while(total_bytes_received < bytes_to_receive) {
        current_bytes_received = recv(sockfd, buffer + total_bytes_received,
         bytes_to_receive - total_bytes_received, 0);
         if (current_bytes_received == 0) {
            printf("s-a inchis serverul inainte sa dea tot\n");
            return 0;
         }
         if (current_bytes_received == -1) {
            printf("exista o eroare la trimitere\n");
            return -1;
            break;
         }
            total_bytes_received = total_bytes_received + current_bytes_received;
    }    
    return total_bytes_received;
}

void error_handler(int condition, char* message) {
    if (condition) {
        fprintf(stderr, "%s\n", message);
        exit(1);
    }
}

int open_connection(const char *host_ip, int portno,
    int ip_type, int socket_type, int flag)
{
    struct sockaddr_in serv_addr;
    int sockfd = socket(ip_type, socket_type, flag);
      error_handler(sockfd < 0, "ERROR opening socket");

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = ip_type;
    serv_addr.sin_port = htons(portno);
    int rc = inet_aton(host_ip, &serv_addr.sin_addr);
    error_handler(rc == 0,  "nu merge inet_aton");

    /* connect the socket */
    rc = connect(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr));
    error_handler(rc < 0, "nu merge sa se conecteze");

    return sockfd;
}

void send_to_server(int sockfd, char* message)
{
    int bytes, sent = 0;
    int total = strlen(message);
    while ( sent <= total) {
        bytes = write(sockfd, message + sent, total - sent);
        if (bytes == 0) {
          return;
        }
        error_handler(bytes < -1, "nu se trimite la server");
        sent = sent + bytes; 
    }

}

void reverse(char s[])
 {
     int i, j;
     char c;

     for (i = 0, j = strlen(s)-1; i<j; i++, j--) {
         c = s[i];
         s[i] = s[j];
         s[j] = c;
     }
}  

void itoa(int n, char s[])
 {
     int i, sign;

     if ((sign = n) < 0)  /* record sign */
         n = -n;          /* make n positive */
     i = 0;
     do {       /* generate digits in reverse order */
         s[i++] = n % 10 + '0';   /* get next digit */
     } while ((n /= 10) > 0);     /* delete it */
     if (sign < 0)
         s[i++] = '-';
     s[i] = '\0';
     reverse(s);
} 

void strcat_multiple_string(char * str1, char *str2, char *str3, char *str4) {
    strcat (str1, str2);
    if (str3 != NULL) {
      strcat(str1, str3);
    }
     if (str4 != NULL) {
      strcat(str1, str4);
    }
}

char *receive_input(char *to_print_before) {
    printf("%s=", to_print_before);
    char buffer[100];
    memset(buffer, 0, 100);
    fgets(buffer, 99, stdin);
    buffer[strlen(buffer) - 1] = '\0';
    char *to_return = calloc(strlen(buffer) + 1, 1);
    memcpy(to_return, buffer, strlen(buffer));
    return to_return;
}

int right_command(char *command, char *supposed_to_be) {
    if (strncmp(command, supposed_to_be, strlen(supposed_to_be)) == 0) {
       if (strlen(command) - 1== strlen(supposed_to_be)) {
            return 1;
       } 
    }
    return 0;
}

