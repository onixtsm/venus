#ifndef OBSTACLES_H_
#define OBSTACLES_H_
#include "TCS3472.h"
#include "VL53L0X.h"
#include "navigation.h"

#define MAX_DISTANCE_TO_OBSTACLE 4000  // mm (To be tested)
#define MIN_DISTANCE_TO_OBSTACLE 50    // mm (To be tested)
#define DISTANCE_FOR_COLOR 30          // mm (To be tested)

#define OBST_SCAN_STEPS 2   // Steps turned to each side (To be tested)
#define OBST_SCAN_ANGLE 15  // Degrees turned in each step (To be tested)

typedef enum { TURN_LEFT, TURN_RIGHT } turnDirection_t;

typedef enum { none, border, crater, farObstacle, smallRock, bigRock, hill } obstacle_type_t;

typedef struct _obstacle_t {
  obstacle_type_t type;
  color_t color;
  uint16_t distance;
} obstacle_t;

obstacle_t scanForFarObstacles(int inSteps, int inAngle, vl53l0x_t **distance_sensors, tcs3472_t *front_looking);

obstacle_t checkForBorderOrCrater(tcs3472_t *down_looking);

obstacle_t scanForFarObstacles(int inSteps, int inAngle, vl53l0x_t **distance_sensors, tcs3472_t *front_looking);

bool scanForObstacles(obstacle_t ioObstacleArray[], int inSteps, int inAngle, vl53l0x_t **distance_sensors,
                      tcs3472_t *down_looking);

bool anyObstacle(obstacle_t inObstacleArray[], int inSize);

void reportObstacle(obstacle_t inObstacle, coordinates_t inPosition);

void reportObstacles(obstacle_t inObstacleArray[], int inSize, coordinates_t inPosition);

void moveForward(void);
void turn90(turnDirection_t inTurnDirection);
#endif
