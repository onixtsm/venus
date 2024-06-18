#include "obstacles.h"
#include "navigation.h"
#include <libpynq.h>

#include "TCS3472.h"
#include "VL53L0X.h"
#include "measurements.h"

obstacle_t checkForBorderOrCrater(tcs3472_t *down_looking) {
  // LOG("-------- checkForBorderOrCrater ---------\n");
  LOG("b?");

  obstacle_t obstacle = {.type = none, .color = COLOR_COUNT};

  if (tcs3472_determine_color(down_looking) == BLACK) {
    obstacle.type = border;  // Or crater, but we cannot differentiate easily...
    LOG("Crater or Border!!!\n");
  }

  return obstacle;
}

#define VL53L0X_LOW 0
#define VL53L0X_MIDDLE 1
#define VL53L0X_HIGH 2

// ***********************************************************
// * FUNCTION: checkForHillOrRock
// ***********************************************************
obstacle_t checkForHillOrRock(vl53l0x_t **vl53l0x_sensors, tcs3472_t *front_looking) {
  // Measure the distance to the possible obstacle
  LOG("-------- checkForHillOrRock ---------\n");

  // Sensor at low level
  // vl53l0x_read_calib_range(vl53l0x_sensors[VL53L0X_LOW], &distance_low);
  vl53l0x_read_range(vl53l0x_sensors[VL53L0X_LOW]);
  LOG("Low level sensor - distance: %d mm\n", vl53l0x_sensors[VL53L0X_LOW]->range);

  // Sensor at middle level
  vl53l0x_read_range(vl53l0x_sensors[VL53L0X_MIDDLE]);
  LOG("Middle level sensor - distance: %d mm\n", vl53l0x_sensors[VL53L0X_MIDDLE]->range);

  // Sensor at high level
  vl53l0x_read_range(vl53l0x_sensors[VL53L0X_HIGH]);
  LOG("Hight level sensor - distance: %d mm\n", vl53l0x_sensors[VL53L0X_HIGH]->range);

  LOG("-------------------------------------\n");

  obstacle_t obstacle = {.type = none, .color = COLOR_COUNT, .distance = 0};

  // Now we update the 3  attributes, starting by the distance
  obstacle.distance = vl53l0x_sensors[VL53L0X_LOW]->range;

  // Determine the type of obstacle
  if (vl53l0x_sensors[VL53L0X_LOW]->range > MAX_DISTANCE_TO_OBSTACLE) {
    obstacle.type = none;
  } else if (vl53l0x_sensors[VL53L0X_LOW]->range > MIN_DISTANCE_TO_OBSTACLE) {
    obstacle.type = farObstacle;
  } else {  // Close obstacle detected at low level
    if (vl53l0x_sensors[VL53L0X_MIDDLE]->range > MIN_DISTANCE_TO_OBSTACLE) {
      obstacle.type = smallRock;
    } else {  // Obstacle detected also at middle level
      if (vl53l0x_sensors[VL53L0X_HIGH]->range > MIN_DISTANCE_TO_OBSTACLE) {
        obstacle.type = bigRock;
      } else {  // Obstacle detected also at high level
        obstacle.type = hill;
      }
    }
    // Determine the color of the obstacle

    // We get closer to the obstacle to measure the color
    navig_start_moving(DISTANCE_FOR_COLOR);
    while (navig_still_moving());  // We wait until reaches the distance

    obstacle.color = tcs3472_determine_color(front_looking);  // We measure the color of the obstacle

    // And then we get back to the original position
    navig_start_moving(-DISTANCE_FOR_COLOR);
    while (navig_still_moving());  // We wait until reaches the distance
  }

  return obstacle;
}

// ***********************************************************
// * FUNCTION: checkForObstacle
// ***********************************************************
obstacle_t checkForObstacle(vl53l0x_t **distance_sensors, tcs3472_t *down_looking) {
  obstacle_t obstacle = checkForBorderOrCrater(down_looking);

  if (obstacle.type != none) {
    return obstacle;
  }

  obstacle = checkForHillOrRock(distance_sensors, down_looking);
  return obstacle;
}

// ***********************************************************
// * FUNCTION: scanForFarObstacles
// ***********************************************************
obstacle_t scanForFarObstacles(int inSteps, int inAngle, vl53l0x_t **distance_sensors, tcs3472_t *front_looking) {
  // Check for obstacle at the front
  obstacle_t obstacle = checkForHillOrRock(distance_sensors, front_looking);  // We check with the distance sensors

  // Check for obstacles towards the right
  for (int i = 0; (i < inSteps) && (obstacle.type != none); i++) {
    navig_start_turning(inAngle);
    while (navig_still_moving());  // We wait until reaches the angle

    obstacle = checkForHillOrRock(distance_sensors, front_looking);  // We check with the distance sensors
  }

  return obstacle;
}

// ***********************************************************
// * FUNCTION: scanForObstacles
// ***********************************************************
bool scanForObstacles(obstacle_t ioObstacleArray[], int inSteps, int inAngle, vl53l0x_t **distance_sensors, tcs3472_t *down_looking) {
  // Check for obstacle at the front
  ioObstacleArray[inSteps] = checkForObstacle(distance_sensors, down_looking);

  // Check for obstacles at the left
  for (int i = 0; i < inSteps; i++) {
    navig_start_turning(-inAngle);
    while (navig_still_moving());  // We wait until reaches the angle

    ioObstacleArray[inSteps - 1 - i] = checkForObstacle(distance_sensors, down_looking);
  }

  // Return to central position
  navig_start_turning(inSteps * inAngle);
  while (navig_still_moving());  // We wait until reaches the angle

  // Check for obstacles at the right
  for (int i = 0; i < inSteps; i++) {
    navig_start_turning(inAngle);
    while (navig_still_moving());  // We wait until reaches the angle

    ioObstacleArray[inSteps + 1 + i] = checkForObstacle(distance_sensors, down_looking);
  }

  // Return to central position
  navig_start_turning(-inSteps * inAngle);
  while (navig_still_moving());  // We wait until reaches the angle

  return false;
}

// ***********************************************************
// * FUNCTION: anyObstacle
// ***********************************************************
bool anyObstacle(obstacle_t inObstacleArray[], int inSize) {
  for (int i = 0; i < inSize; i++) {
    if (inObstacleArray[i].type != none) {
      return true;
    }
  }

  return false;
}

// ***********************************************************
// * FUNCTION: moveForward
// ***********************************************************
void moveForward(void) {
  LOG(">>>>>>>>>>>>>>>>>>>>>>>>>>  Moving Forward in direction %f\n", navig_get_current_heading());
  // 1. Drive motor forward
  sleep_msec(1000);  // We simulate the time to move to the next position

  navig_start_moving(MIN_DISTANCE_TO_OBSTACLE);
  while (navig_still_moving());  // We wait until reaches the angle

  coordinates_t position = navig_get_current_position();
  LOG(">>>>>>>>>>>>>>>>>>>>>>>>>>  Reached position %f, %f\n", position.x_coord, position.y_coord);
}

// ***********************************************************
// * FUNCTION: turn90
// ***********************************************************
void turn90(turnDirection_t inTurnDirection) {
  LOG(">>>>>>>>>>>>>>>>>>>>>>>>>>  Turning 90º to the %s\n", inTurnDirection == TURN_LEFT ? "left" : "right");

  // 1. Drive motor 90º in inTurnDirection
  if (inTurnDirection == TURN_LEFT) {
    navig_start_turning(-90);
    while (navig_still_moving());  // We wait until reaches the angle
  } else {
    navig_start_turning(90);
    while (navig_still_moving());  // We wait until reaches the angle
  }

  LOG(">>>>>>>>>>>>>>>>>>>>>>>>>>  Heading now to %fº\n", navig_get_current_heading());
}

// ***********************************************************
// * FUNCTION: reportObstacle
// ***********************************************************
void reportObstacle(obstacle_t inObstacle, coordinates_t inPosition) {
  LOG("***** OBSTACLE *****\n");

  LOG("* Type: %s\n", inObstacle.type == none        ? "none"
                         : inObstacle.type == border    ? "border"
                         : inObstacle.type == crater    ? "crater"
                         : inObstacle.type == hill      ? "hill"
                         : inObstacle.type == smallRock ? "smallRock"
                         : inObstacle.type == bigRock   ? "bigRock"
                                                        : "default");

  /* printf("* Color: %s\n", inObstacle.color == TCS3200_BLACK   ? "black"
                          : inObstacle.color == TCS3200_RED   ? "red"
                          : inObstacle.color == TCS3200_GREEN ? "green"
                          : inObstacle.color == TCS3200_BLUE  ? "blue"
                                                              : "undefined"); */
  LOG("Color %s", COLOR_NAME(inObstacle.color));

  LOG("* Position: %f, %f\n", inPosition.x_coord, inPosition.y_coord);
}

// ***********************************************************
// * FUNCTION: reportObstacles
// ***********************************************************
void reportObstacles(obstacle_t inObstacleArray[], int inSize, coordinates_t inPosition) {
  for (int i = 0; i < inSize; i++) {
    if (inObstacleArray[i].type != none) {
      reportObstacle(inObstacleArray[i], inPosition);
    }
  }
}
