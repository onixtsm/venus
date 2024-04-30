#include <libpynq.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>

#include "adc.h"
#include "buttons.h"
#include "gpio.h"
#include "util.h"

#define IN IO_A0
#define S0 IO_A5
#define S1 IO_A4
#define S2 IO_A3
#define S3 IO_A2
#define CHECK IO_A1

int main(void) {
  pynq_init();
  gpio_init();
  adc_init();
  buttons_init();

  gpio_set_direction(S0, GPIO_DIR_OUTPUT);
  gpio_set_direction(S1, GPIO_DIR_OUTPUT);
  gpio_set_direction(S2, GPIO_DIR_OUTPUT);
  gpio_set_direction(S3, GPIO_DIR_OUTPUT);
  gpio_set_direction(CHECK, GPIO_DIR_OUTPUT);

  printf("GPIO init\n");

  gpio_set_level(S0, GPIO_LEVEL_HIGH);
  gpio_set_level(S1, GPIO_LEVEL_LOW);

  gpio_set_level(S2, GPIO_LEVEL_HIGH);
  gpio_set_level(S3, GPIO_LEVEL_LOW);
  printf("Levels set\n");
  struct timeval check, start;
  int v;
  gettimeofday(&start, NULL);

  while (1) {
    if (get_button_state(BUTTON0)) {
      break;
    }
    
    v = gpio_get_level(IN);

    gettimeofday(&check, NULL);
    int32_t start_us = start.tv_usec;
    int32_t check_us = check.tv_usec;
    if (check_us - start_us >= 5) {
      fprintf(stderr, "%d,%d\n", check_us, v);
      gettimeofday(&start, NULL);
    }

  }


  gpio_reset();

  pynq_destroy();
  printf("DONE!\n");
  return 0;
}
