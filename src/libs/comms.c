#include <libpynq.h>
#include <cJSON.h>
#include <stdio.h>

char *json;

//Function to code the message into JSON
char* encodeJson(double x_coord, double y_coord, int obj_found, int color, int statuscheck){

  cJSON *root = cJSON_CreateObject();
  cJSON_AddNumberToObject(root, "x", x_coord);
  cJSON_AddNumberToObject(root, "y", y_coord);
  cJSON_AddNumberToObject(root, "obj_found", obj_found);
  cJSON_AddNumberToObject(root, "color", color);
  cJSON_AddNumberToObject(root, "status", statuscheck);
  char *json_string = cJSON_PrintUnformatted(root);
  cJSON_Delete(root);
  return json_string;
}


//function to decode the json
void decodeJson(double *x_coord, double *y_coord, int *obj_found, int *color, int *statuscheck, char *json_string){
  cJSON *root = cJSON_Parse(json_string);
  cJSON *x = cJSON_GetObjectItem(root, "x");
  cJSON *y = cJSON_GetObjectItem(root, "y");
  cJSON *obj= cJSON_GetObjectItem(root, "obj_found");
  cJSON *colorjson = cJSON_GetObjectItem(root, "color");
  cJSON *status = cJSON_GetObjectItem(root, "status");
    
  if (cJSON_IsNumber(x)) {
      *x_coord = x->valuedouble;
  }
  if (cJSON_IsNumber(y)) {
      *y_coord = y->valuedouble;
  }
  if (cJSON_IsNumber(obj)) {
      *obj_found = obj->valueint;
  }
  if (cJSON_IsNumber(colorjson)) {
      *color = colorjson->valueint;
  }
  if (cJSON_IsNumber(status)) {
      *statuscheck = status->valueint;
  }

  cJSON_Delete(root);
}

//Function to send message(Idea: color is one of six colors(?) so its 1-6 interger)(object- 0- nothing, 1- cliff, 2-hill, 3- small block, 4 big block)
void send_json(char message[]){

  uint32_t length = strlen(message);
  uint8_t* len_bytes = (uint8_t*)&length;

  for(int i = 0; i <4 ; i++){
    uart_send(UART0, len_bytes[i]);
  }

  for(uint32_t i=0; i<length ;i++){
    uart_send(UART0, message[i]);
  }
}

//function to receive message
char* receive_json(){
  uint8_t read_len[4];
  for(int i =0; i<4; i++){
    read_len[i] = uart_recv(UART0);
  }
  uint32_t length = *((uint32_t*)read_len);

  char* json_string = (char*)malloc(length+1);
  for(uint32_t i=0; i<length; i++){
    json_string[i] = uart_recv(UART0);
  }

  json_string[length] = '\0';
  return json_string;
}


void send_msg(double x_coord, double y_coord, int obj_found, int color, int status){
  char *json = encodeJson(x_coord, y_coord, obj_found, color, status);
  send_json(json);
}

void recv_msg(double *x_coord, double *y_coord, int *obj_found, int *color, int *status){
  char *json = receive_json();
  decodeJson(x_coord, y_coord, obj_found, color, status, json);

  free(json);
}


