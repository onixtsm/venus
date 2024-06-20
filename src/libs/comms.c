#include "comms.h"

#include <cJSON.h>
#include <libpynq.h>
#include <stdio.h>

// json variable
char* json;

// number of items in JSON objects
#define JSON_SIZE 7

// enumeration for easily identifying the labels for json items
typedef enum { ROBOT_X, ROBOT_Y, ROBOT_STATUS, OBS_X, OBS_Y, OBS_TYPE, OBS_COLOR } identifiers;

// constant for JSON identifying labels
const char* JSON_LABELS[JSON_SIZE] = {
    "robot_x",        // id = ROBOT_X
    "robot_y",        // id = ROBOT_Y
    "robot_status",   // id = ROBOT_STATUS
    "obstacle_x",     // id = OBS_X
    "obstacle_y",     // id = OBS_Y
    "obstacle_type",  // id = OBS_TYPE
    "obstacle_color"  // id = OBS_COLOR
};

// Helper inline functions

/**
 * Inline function that adds robot information to a json object
 *
 * @param rob the robot
 * @param root the json object
 */
#define encode_robot(root, robot)                        \
  cJSON_AddNumberToObject((root), "robot_x", (robot).x); \
  cJSON_AddNumberToObject((root), "robot_y", (robot).y); \
  cJSON_AddNumberToObject((root), "robot_status", (robot).status);

/**
 * Inline function that adds obstacle information to a json object
 *
 * @param obs the obstacle
 * @param root the json object
 */
#define encode_obstacle(root, obstacle)                              \
  cJSON_AddNumberToObject((root), "obstacle_x", (obstacle).x);       \
  cJSON_AddNumberToObject((root), "obstacle_y", (obstacle).y);       \
  cJSON_AddNumberToObject((root), "obstacle_type", (obstacle).type); \
  cJSON_AddNumberToObject((root), "obstacle_color", (obstacle).color);

/**
 * Inline function for extracting one number item from a json object
 *
 * @param root the json object
 * @param temp temporary variable for storing the json item
 * @param item an individual number item from @code root
 * @param label the identifying string of the json item
 *
 * @return exit code 1 if the item is not a number
 */
#define json_extract(root, temp, item, label)       \
  (temp) = cJSON_GetObjectItem((root), (label));    \
  if (!cJSON_IsNumber((temp))) {                    \
    fprintf(stderr, "JSON item is not a number\n"); \
    return 1;                                       \
  } else if (temp == NULL) {                        \
    fprintf(stderr, "JSON variable NULL\n");        \
    return 1;                                       \
  } else {                                          \
    (item) = (temp)->valuedouble;                   \
  }

// private function definitions
/**
 * Encodes information regarding robot status and detected obstacles in a JSON format.
 *
 * @param obs the detected obstacle
 * @param rob the robot that detected
 * @return pointer to a JSON string which encodes the given information.
 */
char* encode_json(obstacle_t obstacle, robot_t robot) {
  // initialize json object
  cJSON* root = cJSON_CreateObject();

  // add all information to the object
  encode_robot(root, robot);
  encode_obstacle(root, obstacle);

  // create the json string
  char* json_string = cJSON_PrintUnformatted(root);

  // release the memory for the json object
  cJSON_Delete(root);
  return json_string;
}

char* encode_string(char* string) {
  cJSON* root = cJSON_CreateObject();
  cJSON_AddStringToObject(root, "name", string);

  char* json_string = cJSON_PrintUnformatted(root);

  // release the memory for the json object
  cJSON_Delete(root);
  return json_string;
}

/**
 * Decodes a JSON string for retrieving information regarding
 * robot status and detected obstacles.
 *
 * @param obs the detected obstacle
 * @param rob the robot that detected
 * @param json_string the JSON string to decode
 * @return exit code 0 if subroutine executes successfully,
 * 1 in case of an error.
 */
int decode_json(obstacle_t* obstacle, robot_t* rob, char* json_string) {
  // temporary json object
  cJSON* temp = NULL;

  // initialize data for the JSON number items
  int data[JSON_SIZE] = {-1};

  // create the json object
  cJSON* root = cJSON_Parse(json_string);

  // fill the json data and handle the case where the item is not a number
  for (uint8_t i = 0; i < JSON_SIZE; i++) {
    json_extract(root, temp, data[i], JSON_LABELS[i]);
  }
  // update the robot and obstacle information based on the received json data
  set_robot_data(rob, data[ROBOT_X], data[ROBOT_Y], data[ROBOT_STATUS], ->);
  set_obstacle_data(obstacle, data[OBS_X], data[OBS_Y], data[OBS_COLOR], data[OBS_TYPE], ->);

  // delete the json object
  cJSON_Delete(root);
  return 0;
};

// Function to send message(Idea: color is one of six colors(?) so its 1-6 interger)(object- 0- nothing, 1- cliff, 2-hill, 3-
// small block, 4 big block)
void send_json(char message[]) {
  uint32_t length = strlen(message);
  uint8_t* len_bytes = (uint8_t*)&length;

  for (int i = 0; i < 4; i++) {
    uart_send(UART0, len_bytes[i]);
  }

  for (uint32_t i = 0; i < length; i++) {
    uart_send(UART0, message[i]);
  }
}

// function to receive message
char* receive_json() {
  uint8_t read_len[4];
  for (int i = 0; i < 4; i++) {
    read_len[i] = uart_recv(UART0);
  }
  uint32_t length = *((uint32_t*)read_len);

  char* json_string = (char*)malloc(length + 1);
  for (uint32_t i = 0; i < length; i++) {
    json_string[i] = uart_recv(UART0);
  }

  json_string[length] = '\0';
  return json_string;
}

// public function definitions
void send_msg(obstacle_t obstacle, robot_t robot) {
  char* json = encode_json(obstacle, robot);
  send_json(json);

  free(json);
}

void recv_msg(obstacle_t* obstacle, robot_t* robot) {
  char* json = receive_json();
  decode_json(obstacle, robot, json);

  free(json);
}

void send_ready_message(char* name) {
  char* json = encode_string(name);
  send_json(json);

  free(json);
}

void send_ready_status() {
  robot_t robot = {NONE, NONE, READY};
  obstacle_t obstacle = {NONE, NONE, NONE, NONE};
  send_msg(robot, obstacle);
}

bool recv_start_status() {
  robot_t* robot;
  obstacle_t* obstacle;
  if (uart_has_data(UART0))
    rcv_msg(robot, obstacle);

  return robot->status == ACKNOWLEDGED;
}

bool recv_start_message(void) {
  char *string = receive_json();
  bool returnable = string[0] != 0;
  free(string);
  return returnable;
}
