#ifndef V_TYPES
#define V_TYPES

#define NONE 69420

/**
 * Structure for defining a point_t in a 2D system.
*/
typedef struct {
    double x;   //the x coordinate
    double y;   //the y coordinate
} point_t;

/**
 * Enumeration for different colors.
*/
typedef enum {
    NO_COLOR, BLACK, WHITE, GREEN, RED, BLUE
} colors;

/**
 * Enumeration for different kinds of objects.
*/
typedef enum {
    NO_OBSTACLE, WALL, HILL, CLIFF, SMALL_ROCK, BIG_ROCK
} obs_types;

/**
 * Collection of data representing an obstacle in a 2D system.
*/
typedef struct {
    double x;
    double y;      //position where obstacle is detected
    colors color;   //color of the obstacle 
    obs_types type; //type of detected obstacle
} obstacle_t;

/**
 * Enumeration for possible robot stata.
*/
typedef enum {
    IDLE, MOVING, SCANNED, COLLIDING
} states;

/**
 * Collection of data representing a robot in a 2D system
*/
typedef struct {
    double x;
    double y;     //position where robot is located
    states status;  //current state of the robot
} robot_t;

/**
 * Enumeration for compass orientations
*/
typedef enum {
    NORTH, SOUTH, EAST, WEST
} orientations;

/**
 * Enumerations for directions in a 2D space
*/
typedef enum {
    LEFT, RIGHT
} directions;

/**
 * Inline function that sets the position and status of a robot_t
 * 
 * @param rob the object to update in terms of position
 * @param new_x the new x coordinate
 * @param new_y the new Y coordinate
 * @param new_status the new robot_t status
 * @param @code symbol == operator "->" or "."
*/
#define set_robot_data(rob, new_x, new_y, new_status, symbol)\
    if ((new_x) != NONE) {\
        (rob) symbol x = (new_x);\
    }\
    if ((new_y) != NONE) {\
        (rob) symbol y = (new_y);\
    }\
    if ((new_status) != NONE) {\
        (rob) symbol status = (new_status);\
    }

/**
 * Inline function that sets the position and status of a robot_t
 * 
 * @param obs the object to update in terms of position
 * @param new_x the new x coordinate
 * @param new_y the new Y coordinate
 * @param new_type the new obstacle type
 * @param new_color the new obstacle color
 * @param @code symbol == operator "->" or "."
*/
#define set_obstacle_data(obs, new_x, new_y, new_type, new_color, symbol)\
    if ((new_x) != NONE) {\
        (obs) symbol x = (new_x);\
    }\
    if ((new_y) != NONE) {\
        (obs) symbol y = (new_y);\
    }\
    if ((new_type) != NONE) {\
        (obs) symbol type = (new_type);\
    }\
    if ((new_color) != NONE) {\
        (obs) symbol color = (new_color);\
    }

#endif