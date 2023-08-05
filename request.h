
void create_common_HTTP_header(char *message, char *host, char *current_command,
  char *body_size) {
    char *endLiner = "\r\n";
    strcat_multiple_string(message, "http://34.254.242.81:8080/api/v1/tema/",current_command, endLiner);
    strcat_multiple_string(message + strlen(message) - 1, "Host: ", host, endLiner);  
    strcat_multiple_string(message + strlen(message) - 1,"Content-Type: application/json", endLiner, NULL);
    strcat_multiple_string(message + strlen(message) - 1, "Connection: keep-alive", endLiner, NULL);
    strcat_multiple_string(message + strlen(message) - 1, "Content-Length: ", body_size, endLiner);
  }


void create_POST_header(char *message, char *host, char *current_command,
    char* body_size, struct client* this_client) {
    memset(message, 0, 1000);
        memcpy(message, "POST ", 6);
        char *endLiner = "\r\n";
        create_common_HTTP_header(message, host, current_command, body_size);
        if (this_client->JWT_token != NULL) {
          strcat_multiple_string(message + strlen(message) - 1, "Authorization: Bearer ", 
            this_client->JWT_token, endLiner);
        }
        if (this_client->cookies != NULL) {
           strcat_multiple_string(message + strlen(message) - 1, "Cookie: ",
           this_client->cookies, endLiner);
        }
        strcat(message + strlen(message) - 1, endLiner);
}

void create_GET_header(char *message, char *host, char *current_command,
    char* body_size, struct client* this_client) {
    memset(message, 0, 1000);
        memcpy(message, "GET ", 5);
        char *endLiner = "\r\n";
        create_common_HTTP_header(message, host, current_command, body_size);
        if (this_client->JWT_token != NULL) {
          strcat_multiple_string(message + strlen(message) - 1, "Authorization: Bearer ", 
          this_client->JWT_token, endLiner);
        }
        if (this_client->cookies != NULL) {
           strcat_multiple_string(message + strlen(message) - 1, "Cookie: ",
           this_client->cookies, endLiner);
        }
        //de bagat aici cookie-urile
        strcat(message + strlen(message) - 1, endLiner);
}

void create_DELETE_header(char *message, char *host, char *current_command,
    char* body_size, struct client* this_client) {
         memset(message, 0, 1000);
        memcpy(message, "DELETE ", 8);
        char *endLiner = "\r\n";
        create_common_HTTP_header(message, host, current_command, body_size);
        if (this_client->JWT_token != NULL) {
          strcat_multiple_string(message + strlen(message) - 1, "Authorization: Bearer ", 
          this_client->JWT_token, endLiner);
        }
        if (this_client->cookies != NULL) {
           strcat_multiple_string(message + strlen(message) - 1, "Cookie: ",
           this_client->cookies, endLiner);
        }
        strcat(message + strlen(message) - 1, endLiner);
    }