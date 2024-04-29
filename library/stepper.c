/*
Copyright (c) 2023 Eindhoven University of Technology

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#include "stepper.h"

#include <platform.h>
#include <stdio.h>
#include <stdlib.h>

#include "arm_shared_memory_system.h"
#include "log.h"

static arm_shared stepper_handles;
static volatile uint32_t *stepper_ptrs = NULL;

#define STEPPER_REG_CONFIG 0

#define STEPPER_REG_STEPS 1
#define STEPPER_REG_CUR_STEPS 1

#define STEPPER_REG_PERIOD 2
#define STEPPER_REG_CUR_PERIOD 2

#define STEPPER_REG_DUTY 3
#define STEPPER_REG_CUR_DUTY 3

#define STEPPER_REG_NXT_STEPS 4
#define STEPPER_REG_NXT_PERIOD 5
#define STEPPER_REG_NXT_DUTY 6
#define STEPPER_REG_COUNT 7

#define MIN_PULSE 0x10
#define MIN_PERIOD (0x30 * 64)

typedef union __attribute__((packed)) {
  struct {
    uint16_t step_l : 15;
    uint8_t dir_l : 1;
    uint16_t step_r : 15;
    uint8_t dir_r : 1;
  };
  uint32_t val;
} steps;

typedef union {
  struct {
    uint16_t left;
    uint16_t right;
  };
  uint32_t val;
} pwm_set;

void stepper_init(void) {
  if (stepper_ptrs != NULL) {
    pynq_error("Stepper library is already initialized\n");
  }
  stepper_ptrs = arm_shared_init(&(stepper_handles), axi_stepper_0, 4096);

  // pulse length. Currently 160 ns
  // TODO lookup datasheet to see minimum
  pwm_set st = {.left = MIN_PULSE, .right = MIN_PULSE};
  stepper_ptrs[STEPPER_REG_DUTY] = st.val;

  st.left = st.right = 0x1000;
  stepper_ptrs[STEPPER_REG_PERIOD] = st.val;
}

void stepper_enable(void) {
  if (stepper_ptrs == NULL) {
    pynq_error("STEPPER has not been initialized.\n");
  }
  // Set reset and lower enable pin
  stepper_ptrs[STEPPER_REG_CONFIG] = 0x1;
}
void stepper_disable(void) {
  if (stepper_ptrs == NULL) {
    pynq_error("STEPPER has not been initialized.\n");
  }
  // Set reset and lower enable pin
  stepper_ptrs[STEPPER_REG_CONFIG] = 0x0;
}

void stepper_destroy(void) {
  if (stepper_ptrs == NULL) {
    pynq_error("STEPPER has not been initialized.\n");
  }

  stepper_disable();
  arm_shared_close(&(stepper_handles));
  stepper_ptrs = NULL;
}

void stepper_reset() {
  if (stepper_ptrs == NULL) {
    pynq_error("STEPPER has not been initialized.\n");
  }
  // Set reset and lower enable pin
  stepper_ptrs[STEPPER_REG_CONFIG] = 0x2;
}

bool stepper_steps_done(void) {
  if (stepper_ptrs == NULL) {
    pynq_error("STEPPER has not been initialized.\n");
  }
  volatile steps *stp = (volatile steps *)&(stepper_ptrs[STEPPER_REG_CUR_STEPS]);
  steps now;
  now.val = stp->val;

  if (now.step_l == 0 && now.step_r == 0) {
    return true;
  }
  return false;
}

void stepper_steps(int16_t left, int16_t right) {
  if (stepper_ptrs == NULL) {
    pynq_error("STEPPER has not been initialized.\n");
  }
  volatile steps *stp = (volatile steps *)&(stepper_ptrs[STEPPER_REG_CUR_STEPS]);
  steps now;
  now.dir_r = (right < 0) ? 0 : 1;
  now.dir_l = (left < 0) ? 0 : 1;
  now.step_r = abs(right);
  now.step_l = abs(left);

  stp->val = now.val;
}

void stepper_set_speed(uint16_t left, uint16_t right) {
  if (stepper_ptrs == NULL) {
    pynq_error("STEPPER has not been initialized.\n");
  }
  if (left < (MIN_PERIOD) && right < (MIN_PERIOD)) {
    pynq_error("STEPPER speed is invalid. Should be atleast %u ticks", MIN_PERIOD);
  }
  volatile pwm_set *stp = (volatile pwm_set *)&(stepper_ptrs[STEPPER_REG_PERIOD]);
  pwm_set n;
  n.left = left;
  n.right = right;
  stp->val = n.val;
}

void stepper_get_steps(int16_t *left, int16_t *right) {
  if (stepper_ptrs == NULL) {
    pynq_error("STEPPER has not been initialized.\n");
  }
  volatile steps *stp = (volatile steps *)&(stepper_ptrs[STEPPER_REG_CUR_STEPS]);
  volatile steps now;
  now.val = stp->val;

  if (now.dir_l == 0) {
    *left = now.step_l;
  } else {
    *left = -now.step_l;
  }
  if (now.dir_r == 0) {
    *right = -now.step_r;
  } else {
    *right = now.step_r;
  }
}
