#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define HEADER_LEN 1024
#define BUFFER_LEN 4096

struct response
{
  char *header;
  int body_size;
  char *body; 
};

char* extract_cookies(struct response* mess) {
    char *where_cookies_start = strstr(mess->header, "Set-Cookie:");
    if (where_cookies_start == NULL) {
      return NULL; 
    }
    where_cookies_start = where_cookies_start + strlen("Set-Cookie: ");
    
    char *all_cookies;
    char *limit = strstr(where_cookies_start, "; ");
    int cookies_size = limit - where_cookies_start;
    all_cookies = calloc(cookies_size + 1, 1);
    memcpy(all_cookies, where_cookies_start, cookies_size);
    return all_cookies;
}

int get_body_size(char *message) {
    char *how_big_is_body = strstr(message, "Content-Length:");
    if (how_big_is_body == NULL) {
      //la register ii bine asa
      return 0;
    }
    how_big_is_body = how_big_is_body + strlen("Content-Length:") + 1;
    char *number_ends_here = strstr(how_big_is_body, "\r\n");
    int how_much_for_atoi = number_ends_here - how_big_is_body;
    char nr_buff[how_much_for_atoi + 1];
    memset(nr_buff, 0, how_much_for_atoi + 1);
    memcpy(nr_buff, how_big_is_body, how_much_for_atoi);
    return atoi(nr_buff);
}


struct response* receive_from_server(int sockfd) {
  char buffer[BUFFER_LEN + 1];
  memset(buffer, 0, BUFFER_LEN + 1);
  int total_size = 0;
  int current_size_received = 0;
  while(strstr(buffer, "\r\n\r\n") == NULL) {
    current_size_received = recv(sockfd, buffer + total_size, BUFFER_LEN, 0);
    error_handler(current_size_received < 0, "nu primeste mesajul de server");
    total_size = total_size + current_size_received;
  }
  char *limit = strstr(buffer, "\r\n\r\n");
  struct response* mess = malloc(sizeof(struct response));
  mess->header = calloc(limit - buffer + 1, 1);
  memcpy(mess->header, buffer, limit - buffer);

  int body_size = get_body_size(buffer);
  mess->body_size = body_size;
  if (body_size == 0) {
    mess->body = NULL;
    return mess;
  } 

  int endHeaderSize = strlen("\r\n\r\n");
  int header_size = strlen(mess->header);
  int buffer_size = strlen(buffer);
  int currently_captured = buffer_size - header_size - endHeaderSize;

  if (currently_captured >= body_size) {
    mess->body = calloc(body_size + 1, 1);
    memcpy(mess->body, limit + endHeaderSize,  body_size);
    return mess;
  } else {
    if (body_size < BUFFER_LEN) {
      current_size_received = recv(sockfd, buffer + total_size, 
        body_size - currently_captured, 0);
      mess->body = calloc(body_size + current_size_received + 1, 1);
      memcpy(mess->body, limit + endHeaderSize,  body_size + current_size_received);
      return mess;
    } else {
      int body_fraction_captured = BUFFER_LEN - header_size - endHeaderSize;
      int bytes_left_to_capture = body_size - body_fraction_captured; 
      mess->body = calloc(body_size + 1, 1);
      memcpy(mess->body, buffer + header_size + endHeaderSize, body_fraction_captured);
      current_size_received = recv_all(sockfd, mess->body + body_fraction_captured, bytes_left_to_capture);
      return mess;
    }
  }
  return mess;
}


int return_error_code(struct response *mess) {
    char *first_row = strstr(mess->header, "\r\n");
    char buffer[strlen(first_row) + 1];
    buffer[strlen(first_row)] = '\0';
    memcpy(buffer, mess->header, strlen(first_row));
    strtok(buffer, " ");
    char *response_code = strtok(NULL, " ");
    char num_buffer[4];
    memset(num_buffer, 0, 4);
    memcpy(num_buffer, response_code, 3);
    return atoi(num_buffer);
}

void display_message(struct response* mess) {
    printf("%d\n", mess->body_size);
        printf("%s\n", mess->header);
        printf("%d\n", mess->body_size);
        printf("________\n");
        if (mess->body != NULL) {
          printf("%s\n", mess->body);
        }
}

void free_message(struct response* mess) {
   if (mess->header != NULL) {
    free(mess->header);
   }
   if (mess->body != NULL) {
    free(mess->body);
   }
   free(mess);
} 


