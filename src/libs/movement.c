#include "movement.h"

#include <libpynq.h>
#include <stepper.h>

void m_turn_degrees(float degrees, directionLR d)  // direction input should be left/right
{
  float steps = degrees * (2463.0f / 360);
  if (d == right)  // turns right
  {
    stepper_steps(-steps, steps);
  } else  // turns left
  {
    stepper_steps(steps, -steps);
  }
}

void m_forward_or(float distance, directionFB d) {
  float steps = distance * (1600.0f / 25.13274);
  if (d == forward) {
    stepper_steps(steps, steps);
  } else {
    stepper_steps(-steps, -steps);
  }
}

void m_waggle(int step_distance, int distance) {                   // i is step distance and d is total distance
  const int predefined_steps[7] = {0, 33, 66, 99, 133, 166, 340};  // [6] = 10 cm;
  for (int i = 0; i < distance; i += step_distance) {
    sleep_msec(1000);
    stepper_steps(predefined_steps[step_distance], 0);  // wiggle right
    //<scan>                       //scanning surroundings
    sleep_msec(1000);
    stepper_steps(0, predefined_steps[step_distance]);  // wiggling left double the distance
    //<scan>
    sleep_msec(1000);
    stepper_steps(0, predefined_steps[step_distance]);
    //<scan>
    sleep_msec(1000);                                   // scanning surroundings
    stepper_steps(predefined_steps[step_distance], 0);  // wiggling right to finish the movement
  }
}

void m_stop(void) {
  stepper_reset();
  stepper_enable();
}

void m_init(void) {
  stepper_init();
  stepper_enable();
}
