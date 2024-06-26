#include "navigation.h"

#include <libpynq.h>
#include <math.h>
#include <stepper.h>
#include <stdint.h>

#include "TCS3472.h"
#include "measurements.h"
#include "movement.h"
#include "VL53L0X.h"
#include "comms.h"
#include "src/libs/vtypes.h"

// Initialization of global variables for the movement
static coordinates_t g_final_position = {0.0, 0.0};
static float g_final_heading = 0.0;

static navig_movement_t g_current_movement = NAVIG_NONE;

/**
 * Implementation of the functions
 */

double direction(double *di, double ddi) {  // updates direction properly
  double direction = *di + ddi;
  if (direction >= 360) {
    direction -= 360;
  } else if (direction < 0) {
    direction += 360;
  }
  return direction;
}

void navig_init(float heading) {
  g_final_heading = heading;

  stepper_init();
  stepper_enable();
}

void navig_destroy(void) { stepper_destroy(); }

void navig_stop(void) {
  stepper_reset();

  // TO CHECK IF THE REMAINING STEPS ARE ALSO RESET BY stepper_reset();
  g_final_position = navig_get_current_position();
  g_final_heading = navig_get_current_heading();

  // Couldn't be updated before navig_get_current_XXX() were called
  g_current_movement = NAVIG_NONE;

  stepper_enable();
}

void navig_start_turning(float angle) {
  if (angle > 360.0) angle = 360.0;
  if (angle < -360.0) angle = -360.0;

  LOG("Starting turning %f degrees\n", angle);

  if (navig_still_moving()) {
    navig_stop();
  }

  float steps = angle * (2463.0f / 360);

  // We update the static global variables
  g_final_heading += angle;  // We update already to the final heading
  g_current_movement = NAVIG_TURNING;

  stepper_steps(steps, -steps);
}

void navig_start_moving(float distance) {
  LOG("Starting moving %f mm\n", distance);

  if (navig_still_moving()) {
    navig_stop();
  }

  float steps = distance * (1600.0f / 25.13274);

  // We update the static global variables
  g_final_position.x_coord += distance * sin(g_final_heading);  // We update already to the final position
  g_final_position.y_coord += distance * cos(g_final_heading);  // We update already to the final position
  g_current_movement = NAVIG_MOVING;

  stepper_steps(steps, steps);
}

bool navig_still_moving(void) { return !stepper_steps_done(); }

coordinates_t navig_get_current_position(void) {
  coordinates_t current_position;

  switch (g_current_movement) {
    case NAVIG_NONE:
    case NAVIG_TURNING:
      current_position = g_final_position;
      break;
    case NAVIG_MOVING:;
      int16_t remaining_left_steps, remaining_right_steps;
      stepper_get_steps(&remaining_left_steps, &remaining_right_steps);

      float remaining_distance = remaining_left_steps / (1600.0f / 25.13274);

      current_position.x_coord = g_final_position.x_coord - remaining_distance * sin(g_final_heading);
      current_position.y_coord = g_final_position.y_coord - remaining_distance * cos(g_final_heading);
      break;
  }

  LOG("navig_get_current_position() => %f, %f\n", current_position.x_coord, current_position.y_coord);

  return current_position;
}

float navig_get_current_heading(void) {
  float current_heading;

  switch (g_current_movement) {
    case NAVIG_NONE:
    case NAVIG_MOVING:
      current_heading = g_final_heading;
      break;
    case NAVIG_TURNING:;
      int16_t remaining_left_steps, remaining_right_steps;
      stepper_get_steps(&remaining_left_steps, &remaining_right_steps);

      float remaining_angle = remaining_left_steps / (2463.0f / 360);

      current_heading = g_final_heading - remaining_angle;

      break;
  }

  LOG("navig_get_current_heading() => %f\n", current_heading);

  return current_heading;
}

bool killSwitchScan(position_t *pos, position_t *tPos, tcs3472_t *down_looking) {
  printf("Killswitch\n");


  // if (!stepper_steps_done()) {
  //   printf("Scanning color\n");
  //   if (tcs3472_determine_color(down_looking) == BLACK) {
  //     int16_t stepsL = 0, stepsR = 0;
  //     stepper_get_steps(&stepsL, &stepsR);
  //     m_stop();
  //     float rads = pos->di * pi / 180;
  //     float distance = stepsL * (25.13274 / 1600.0);
  //     pos->x = tPos->x - distance * cos(rads);
  //     pos->y = tPos->y - distance * sin(rads);

  //     tPos->x = 9000;
  //     tPos->y = 9000;
    

  //   return;
  // } else {
  //   killSwitchScan(pos, tPos, down_looking);
  // }
  while (!stepper_steps_done()) {
    printf("Scanning color\n");
    if (tcs3472_determine_color(down_looking) == BLACK) {
      LOG("BLACK ON THE BOTTOM");
      int16_t stepsL = 0, stepsR = 0;
      stepper_get_steps(&stepsL, &stepsR);
      m_stop();
      float rads = pos->di * pi / 180;
      float distance = stepsL * (25.13274 / 1600.0);
      pos->x = tPos->x - distance * cos(rads);
      pos->y = tPos->y - distance * sin(rads);

      tPos->x = 9000;
      tPos->y = 9000;
      m_forward_or(10, backward);
      pos->x -= 10 * cos(pos->x);
      pos->y -= 10 * sin(pos->y);
      while(!stepper_steps_done());
      m_turn_degrees(90, left);
      return true;
    }
  }

  pos->x = tPos->x;
  pos->y = tPos->y;
  return false;
}


obstacle_t avoidBorderOrCrater(position_t *pos, tcs3472_t *forward_looking) {
  obstacle_t  obstacle = {pos->x, pos->y, COLOR_COUNT, NONE};
  obstacle = scanBorderCrater(pos, forward_looking);

  m_turn_degrees(60, left);  // while the robot is still on the border
  direction(&pos->di, 60);
  return obstacle;
}

obstacle_t scanBorderCrater(position_t *pos, tcs3472_t *forward_looking) {
  obstacle_t obstacle;

  float rads = pos->di * pi / 180;
  obstacle.x = pos->x + 6 * cos(rads);
  obstacle.y = pos->y + 6 * sin(rads);

  obstacle.type = WALL;
  obstacle.color = tcs3472_determine_color(forward_looking);
  return obstacle;
}

obstacle_t scanScope(position_t *pos, vl53l0x_t **distance_sensors, tcs3472_t *forward_looking, tcs3472_t *down){

  obstacle_t obstacle = {pos->x, pos->y, COLOR_COUNT, NONE};  

  uint16_t distance[14] = {0};
  uint16_t distance_low;

  m_turn_degrees(60, left);                              //turn 30 deg left
  pos->di = direction(&pos->di, 60.0);    //update orientation
  while(!stepper_steps_done()){
    sleep_msec(100);
  }
  for(int i = 0; i < 12; i++){                           //repreats previous process
    vl53l0x_read_mean_range(distance_sensors[VL53L0X_LOW], &distance_low);
    distance[i] = distance_low;
    position_t tPos = {pos->x, pos->y, 0.0};
    m_turn_degrees(10, right);
    killSwitchScan(pos, &tPos, down);   
    direction(&pos->di, -10.0);
    if(distance[i] < 500){
      robot_t robot = {pos->x, pos->y, IDLE};
      obstacle.x = pos->x + (distance[i]+7) / 10 * cos(pos->di);
      obstacle.y = pos->y + (distance[i]+7) / 10 * sin(pos->di);
      obstacle.type = NONE;
      obstacle.color = NONE;
      send_msg(obstacle, robot);
      printf("x: %f\n", obstacle.x);
      printf("y: %f\n", obstacle.y);
    }
  }

  vl53l0x_read_mean_range(distance_sensors[VL53L0X_LOW], &distance_low);
  distance[12] = distance_low;

  m_turn_degrees(60, left);                              //repeat process to original position
  pos->di = direction(&pos->di, 60.0);

  int index = 13;    //the index of the smallest distance
  distance[index] = 150;
  for(int i = 0; i<13; i++){
    if(distance[i] < DISTANCE_FOR_SCOPE && distance[i] < distance[index]){
      index = i;
    }
  }
  if(index!=13){
    m_turn_degrees(60 - index * 10, left);
    obstacle = scanHillOrRock(pos, distance_sensors, forward_looking, down);
  }
  return obstacle;
}

obstacle_t scanHillOrRock(position_t *pos, vl53l0x_t **distance_sensors, tcs3472_t *forward_looking, tcs3472_t *down){
  printf("Moving towards hill\n");
  obstacle_t obstacle = {pos->x, pos->y, COLOR_COUNT, NONE};

  uint16_t distance_low = 8910, distance_middle = 8910, distance_high = 8910;
  vl53l0x_read_mean_range(distance_sensors[VL53L0X_LOW], &distance_low);
  vl53l0x_read_mean_range(distance_sensors[VL53L0X_MIDDLE], &distance_middle);
  vl53l0x_read_mean_range(distance_sensors[VL53L0X_HIGH], &distance_high);

  LOG("Distance to high obstacle: %d \n", distance_high);
  LOG("Distance to middle obstacle: %d \n", distance_middle);
  LOG("Distance to low obstacle: %d \n", distance_low);


  if (distance_low < DISTANCE_FOR_SCOPE){
    
    while (distance_middle > DISTANCE_FOR_COLOR && distance_low > DISTANCE_FOR_COLOR && distance_high > DISTANCE_FOR_COLOR){             //while the distance to the object is too large
      if (should_die()) {
        break;
      }
      position_t tPos = {0.0, 0.0, 90.0};
      m_forward_or(0.5, forward);                            //move forward 1cm
      tPos.x += cos(pos->di) * 0.5;                             //update its position
      tPos.y += sin(pos->di) * 0.5;
      printf("moving forward, should do killswitch\n");
      bool kill = false;
      while(!stepper_steps_done()){
        kill = killSwitchScan(pos, &tPos, down);
      }
      if(kill) {
        LOG("kill: %d", kill);
        return obstacle;
      }
      sleep_msec(100);
      
      pos->x=  tPos.x;
      pos->y = tPos.y;
      vl53l0x_read_mean_range(distance_sensors[VL53L0X_LOW], &distance_low);
      vl53l0x_read_mean_range(distance_sensors[VL53L0X_MIDDLE], &distance_middle);
      vl53l0x_read_mean_range(distance_sensors[VL53L0X_HIGH], &distance_high);

      LOG("Distance to high obstacle: %d \n", distance_high);
      LOG("Distance to middle obstacle: %d \n", distance_middle);
      LOG("Distance to low obstacle: %d \n", distance_low);

    }
    // vl53l0x_read_mean_range(distance_sensors[VL53L0X_LOW], &distance_low);
    // vl53l0x_read_mean_range(distance_sensors[VL53L0X_MIDDLE], &distance_middle);
    // vl53l0x_read_mean_range(distance_sensors[VL53L0X_HIGH], &distance_high);
    if (distance_low >= distance_high - 20 && distance_low <= distance_high + 20){    //readings of distance sensors must be within 20mm of each other to be considered the same
      obstacle.type = HILL;
      LOG("hill\n");
    } else if (distance_low >= distance_middle - 20 && distance_low <= distance_middle + 20){
      obstacle.type = BIG_ROCK;
      LOG("big rock\n");
    } else{
      obstacle.type = SMALL_ROCK;
      LOG("small rock\n");
    }
    float rads = pos->di * pi / 180;
    obstacle.x = pos->x + 6 * cos(rads);
    obstacle.y = pos->y + 6 * sin(rads);       //set the coordinates of the obstacle
    color_t c = tcs3472_determine_color(forward_looking);
    LOG("front color: %d", c);
    obstacle.color = c;
  }
  return obstacle;
}
