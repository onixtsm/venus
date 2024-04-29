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
#ifndef STEPPER_H
#define STEPPER_H
#include <stdbool.h>
#include <stdint.h>

/**
 * @defgroup STEPPER STEPPER library
 *
 * stepper library only for balancing robot.
 *
 *
 *  requires 0.3.0
 * @{
 */

/**
 * Initialize the stepper driver.
 */
extern void stepper_init(void);

/**
 * Cleanup the stepper driver.
 */
extern void stepper_destroy(void);

/**
 * Enable the steppers. This toggles the enable pin
 * on the physical drivers.
 * If enabled the wheels be locked into position.
 */
extern void stepper_enable(void);
/**
 * Disable the steppers. This toggles the enable pin
 * on the physical drivers.
 * If disabled, the wheels freely spin.
 */
extern void stepper_disable(void);

/**
 * Emergency reset the whole stepper module.
 * This will disable the drivers and discard any command
 * currently being queued or processed.
 */
extern void stepper_reset(void);

/**
 * @param left The speed for the left wheel
 * @param right The speed for the right wheel
 *
 * The value set indicates the time between pulses.
 * The shorter the value, the faster the stepper steps.
 */
extern void stepper_set_speed(uint16_t left, uint16_t right);

/**
 * @param left The number of steps for the left wheel
 * @param right The number of steps for the right wheel
 *
 * This sets the number of steps to take for each wheel.
 * The start is always synchronized.
 *
 * Once it starts to process this, it stores the speed and number of
 * steps. You can prepare a next command before it finishes processing.
 * It only starts on the next command if both left/right steps are taken.
 * To make sure that both wheels rotate the full command adjust the speed
 * accordingly. For example to turn nicely: left speed X, steps Y,
 * then right steps Y/2, speed X*2  (so goes 1/2 as fast)
 */
extern void stepper_steps(int16_t left, int16_t right);

/**
 * @param left The number of steps for the left wheel
 * @param right The number of steps for the right wheel
 *
 * The steps remaining to take in the current command.
 */
extern void stepper_get_steps(int16_t *left, int16_t *right);
/**
 * @returns true if stepper is idle, false when processing a command.
 */
extern bool stepper_steps_done(void);

/**
 * @}
 */

#endif  // STEPPER_H
