#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "parson.h"
#include "common.h"
#include "response.h"
#include "request.h"
#define HEADER_LEN 1024
#define BUFFER_LEN 4096

int page_count_contains_only_numbers(char *page_count) {
  unsigned char cifra_zero = 48;
  unsigned char cifra_noua = 57;
  for(unsigned int i = 0; i < strlen(page_count); i++) {
     if (page_count[i] > cifra_noua || page_count[i] < cifra_zero) {
        return 0;
     }
  }
  return 1; 
}

int check_page_count(char *page_count) {
  if (strlen(page_count) > 5) {
    printf("Nu are sens sa aiba asa de multe de pagini.\n");
    return 0;
  }
  return page_count_contains_only_numbers(page_count);
}

char *create_login_json_body(char *username, char *password) {
    JSON_Value *root_value = json_value_init_object();
    JSON_Object *root_object = json_value_get_object(root_value);
    json_object_set_string(root_object, "username", username);
    json_object_set_string(root_object, "password", password);
    char *serialized_string = json_serialize_to_string_pretty(root_value);
    json_value_free(root_value); 
    return serialized_string;
}

int main() {
    char* host = "34.254.242.81";
    uint16_t port = 8080;
    char command[40];
    char message[1000];
    struct client* my_client = init_client();
    while(1) {
      memset(command, 0, 40);
      fgets(command, 39, stdin);
      if (right_command(command, "exit")) {
        free_client(my_client);
        return 0;
      }
      if (right_command(command, "register")) {
        int sfd = open_connection(host, port, AF_INET, SOCK_STREAM, 0); 
        //inregsitratre
        char *username = receive_input("username");
        char *password = receive_input("password");
        
        //aici facem body-ul
        char *serialized_string = create_login_json_body(username, password);
        int num = strlen(serialized_string);
         char snum[5];
        itoa(num, snum);
        //aici facem antetul
        create_POST_header(message, host, "auth/register", snum, my_client);
        // add body
        strcat(message + strlen(message) - 1,  serialized_string);
        
         send_to_server(sfd, message);
         struct response* mess;
         mess = receive_from_server(sfd);
        //display_message(mess);
        if (mess->body == NULL) {
          printf("Utilizatorul %s a fost creat cu succes\n", username);
        } else {
          printf("%s\n", mess->body);
        }

        free(username);
        free(password);
        free_message(mess);
        json_free_serialized_string(serialized_string);
        close(sfd);
        continue;
      }
      if (right_command(command, "login")) {
        if (my_client->username != NULL && my_client->password != NULL) {
            printf("Esti deja logat avand username-ul: %s.\n", my_client->username);
            continue;
        }
        int sfd = open_connection(host, port, AF_INET, SOCK_STREAM, 0); 
        char *username = receive_input("username");
        char *password = receive_input("password");
        //aici facem body-ul
        char *serialized_string = create_login_json_body(username, password);
        int num = strlen(serialized_string);
         char snum[5];
        itoa(num, snum);

        create_POST_header(message, host, "auth/login", snum, my_client);
        strcat(message + strlen(message) - 1,  serialized_string);

        send_to_server(sfd, message);
        struct response* mess;
        mess = receive_from_server(sfd);
        my_client->cookies = extract_cookies(mess);

        //display_message(mess);
        int return_code = return_error_code(mess);
        if (return_code >= 400 && return_code <=499) {
          printf("%s\n", mess->body);
        } else {
          printf("Utilizatorul %s s-a logat cu succes\n", username);
          my_client->username = username;
          my_client->password = password;
        }

        json_free_serialized_string(serialized_string);
        free_message(mess);
        close(sfd);
        continue;
      }
      if (right_command(command, "enter_library")) {
        if (my_client->cookies == NULL) {
          printf("Nu esti logat.\n");
          printf("Executa comanda 'login' inainte\n");
          continue;
        }
        if (my_client->JWT_token != NULL) {
          printf("Deja ai cerut accesul la biblioteca si esti conectat.\n");
          continue;
        }
         int sfd = open_connection(host, port, AF_INET, SOCK_STREAM, 0);
         memset(message, 0, 1000);
        create_GET_header(message, host, "library/access", "0", my_client);
        send_to_server(sfd, message);
        struct response* mess;
        mess = receive_from_server(sfd);

        JSON_Value *schema = json_parse_string(mess->body);
        char *to_copy_token = (char*)json_object_get_string(json_object(schema),
           "token");
        my_client->JWT_token = calloc(strlen(to_copy_token) + 1, 1);
        memcpy(my_client->JWT_token, to_copy_token, strlen(to_copy_token));

        free_message(mess);
        json_value_free(schema);
        close(sfd);
        continue;
      }
       if (right_command(command, "get_books")) {
        if (my_client->cookies == NULL) {
          printf("Nu esti logat.\n");
          printf("Executa comanda 'login' inainte\n");
          continue;
        }
        if (my_client->JWT_token == NULL) {
            printf("Nu ai acces la biblioteca.\n");
            printf("Executa comanda 'enter_library' inainte.\n");
            continue;
        }
        int sfd = open_connection(host, port, AF_INET, SOCK_STREAM, 0);
        memset(message, 0, 1000);
        create_GET_header(message, host, "library/books", "0", my_client);
        send_to_server(sfd, message);
        struct response* mess;
        mess = receive_from_server(sfd);
        //TODO: properly display message
        printf("%s\n", mess->body);
        JSON_Value *schema = json_parse_string(mess->body);
        JSON_Array *book_array = json_value_get_array(schema);
        for (unsigned int i = 0; i < json_array_get_count(book_array); i++) {
          JSON_Object *book = json_array_get_object(book_array, i);
          printf("Book number %d:\n", i);
          double id = json_object_get_number(book, "id");
          printf("%d\n", (int)id);
          printf("%s\n\n", json_object_get_string(book, "title"));
        }

        json_value_free(schema);        
        free_message(mess);
        close(sfd);
        continue;
      }
      // mai trebuie de rezolvat calea la get_book //de facut corect;
      if (right_command(command, "add_book")) {
        if (my_client->cookies == NULL) {
          printf("Nu esti logat.\n");
          printf("Executa comanda 'login' inainte\n");
          continue;
        }
        if (my_client->JWT_token == NULL) {
            printf("Nu ai acces la biblioteca.\n");
            printf("Executa comanda 'enter_library' inainte.\n");
            continue;
        }
        int sfd = open_connection(host, port, AF_INET, SOCK_STREAM, 0); 
        memset(message, 0, 1000);
        char *title = receive_input("title");
        char *author = receive_input("author");
        char *genre = receive_input("genre");
        char *publisher = receive_input("publisher");
        char *page_count = receive_input("page_count");
        while(check_page_count(page_count) == 0) {
            free(page_count);
            printf("Introdu un page count valid:\n");
            page_count = receive_input("page_count");
        }
        
        //TODO: de validat ce a introdus utilizatorul;

        JSON_Value *root_value = json_value_init_object();
        JSON_Object *root_object = json_value_get_object(root_value);
        json_object_set_string(root_object, "title", title);
        json_object_set_string(root_object, "author", author);
        json_object_set_string(root_object, "genre", genre);
        json_object_set_string(root_object, "publisher", publisher);
        json_object_set_string(root_object, "page_count", page_count);
        char *serialized_string = json_serialize_to_string_pretty(root_value);
        int body_size = strlen(serialized_string);
        char snum[5];
        itoa(body_size, snum);

        create_POST_header(message, host, "library/books", snum, my_client);
        strcat(message + strlen(message) - 1,  serialized_string);
        send_to_server(sfd, message);
        struct response* mess;
        mess = receive_from_server(sfd);
        int return_code = return_error_code(mess);
        if (return_code >= 400 && return_code <= 499) {
          printf("Exista o eroare avand codul %d:\n", return_code);
          printf("%s\n", mess->body);
        } else {
          printf("Cartea '%s' scrisa de '%s' a fost adaugata cu succes.\n", 
            title, author);
        }

        free_message(mess);
        json_value_free(root_value);
        json_free_serialized_string(serialized_string);
        free(title);
        free(author);
        free(genre);
        free(publisher);
        free(page_count);
        close(sfd);
        continue;
      }
      if (right_command(command, "get_book")) { 
        if (my_client->cookies == NULL) {
            printf("Nu ai acces la biblioteca.\n");
            printf("Executa comanda 'enter_library' inainte.\n");
            continue;
        }
        if (my_client->JWT_token == NULL) {
          printf("Nu esti logat.\n");
          printf("Executa comanda 'login' inainte\n");
          continue;
        }
        int sfd = open_connection(host, port, AF_INET, SOCK_STREAM, 0);
        memset(message, 0, 1000);
        char *id = receive_input("id");
        JSON_Value *root_value = json_value_init_object();
        JSON_Object *root_object = json_value_get_object(root_value);
        json_object_set_string(root_object, "id", id);
        char *serialized_string = json_serialize_to_string_pretty(root_value);
        int num = strlen(serialized_string);
        char snum[5];
        itoa(num, snum);
        int initial_command_size = strlen("library/books/"); 
        char *complete_command = calloc(initial_command_size + strlen(id) + 1, 1);
        memcpy(complete_command, "library/books/", initial_command_size);
        strcat(complete_command, id); 

        create_GET_header(message, host, complete_command, snum, my_client);
        strcat(message + strlen(message) - 1,  serialized_string);
        send_to_server(sfd, message);
        struct response* mess;
        mess = receive_from_server(sfd);
        printf("%s\n", mess->body);

        free(complete_command);
        free_message(mess);
        json_free_serialized_string(serialized_string);
        json_value_free(root_value);
        free(id);
        close(sfd);
        continue;
      }
      if (right_command(command, "delete_book")) {
        if (my_client->JWT_token == NULL) {
          printf("Nu esti logat.\n");
          printf("Executa comanda 'login' inainte\n");
          continue;
        }
        if (my_client->cookies == NULL) {
            printf("Nu ai acces la biblioteca.\n");
            printf("Executa comanda 'enter_library' inainte.\n");
            continue;
        }
        int sfd = open_connection(host, port, AF_INET, SOCK_STREAM, 0);
        memset(message, 0, 1000);
        char *id = receive_input("id");
        JSON_Value *root_value = json_value_init_object();
        JSON_Object *root_object = json_value_get_object(root_value);
        json_object_set_string(root_object, "id", id);
        char *serialized_string = json_serialize_to_string_pretty(root_value);
        int num = strlen(serialized_string);
        char snum[5];
        itoa(num, snum);
        int initial_command_size = strlen("library/books/"); 
        char *complete_command = calloc(initial_command_size + strlen(id) + 1, 1);
        memcpy(complete_command, "library/books/", initial_command_size);
        strcat(complete_command, id); 

        create_DELETE_header(message, host, complete_command, snum, my_client);
        strcat(message + strlen(message) - 1,  serialized_string);
        send_to_server(sfd, message);
        struct response* mess;
        mess = receive_from_server(sfd);
        int operation_code = return_error_code(mess);
        if (operation_code >= 400 && operation_code <= 499) {
          printf("Exista o eroare:\n");
          printf("%s\n", mess->body);
        } else {
          printf("Cartea cu id-ul %s a fost stearsa din biblioteca.\n", id);
        }

        json_free_serialized_string(serialized_string);
        json_value_free(root_value);
        free_message(mess);
        free(complete_command);
        free(id);
        close(sfd);
        continue;
      }
      if(right_command(command, "logout")) {
        int sfd = open_connection(host, port, AF_INET, SOCK_STREAM, 0);
        memset(message, 0, 1000);
        create_GET_header(message, host, "auth/logout", "0", my_client);
        send_to_server(sfd, message);
        struct response* mess;
        mess = receive_from_server(sfd);
        int operation_code = return_error_code(mess);
        if (operation_code >= 400 && operation_code <= 499) {
          printf("Exista o eroare:\n");
          printf("%s\n", mess->body);
        } else {
          printf("Utilizatorul %s a fost deconectat cu succes.\n", my_client->username);
          free(my_client->cookies);
          free(my_client->JWT_token);
          free(my_client->password);
          free(my_client->username);
          my_client->password = NULL;
          my_client->username = NULL;
          my_client->cookies = NULL;
          my_client->JWT_token = NULL;
        }

        free_message(mess);
        close(sfd);
        continue;
      }
      printf("Nu exista aceasta comanda\n");
    }
    return 0;
}