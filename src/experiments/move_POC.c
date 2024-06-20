#include <libpynq.h>
#include <stepper.h>

int main(void) {

	pynq_init();
  stepper_init();
  stepper_enable();
  int steps = 1000;
    stepper_steps(steps, steps);
}
