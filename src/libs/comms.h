#ifndef COMMS_H
#define COMMS_H
#include "vtypes.h"

/**
 * Sends information regarding robot status and detected obstacles
 * from a robot to the server.
 * 
 * @param obstacle the detected obstacle
 * @param robot the robot that detected
*/

void send_msg(obstacle_t obstacle, robot_t robot);

/**
 * Retrieves information regarding robot status and detected obstacles
 * from the server and stores it in the robot.
 * 
 * @param obs the detected obstacle
 * @param rob the robot that detected
*/
void recv_msg(obstacle_t* obstacle, robot_t* robot);

/* Sends reasy message */
void send_ready_message(char *name);

bool recv_start_message(void);
#endif
