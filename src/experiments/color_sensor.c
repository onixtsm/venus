#include <libpynq.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>

#define IN IO_AR9
#define ADC_IN ADC0
#define S0 IO_AR11
#define S1 IO_AR10
#define S2 IO_AR12
#define S3 IO_AR13
#define CHECK IO_A1

int main(void) {
  pynq_init();
  gpio_init();
  adc_init();
  buttons_init();
  switches_init();

  gpio_set_direction(S0, GPIO_DIR_OUTPUT);
  gpio_set_direction(S1, GPIO_DIR_OUTPUT);
  gpio_set_direction(S2, GPIO_DIR_OUTPUT);
  gpio_set_direction(S3, GPIO_DIR_OUTPUT);
  gpio_set_direction(CHECK, GPIO_DIR_OUTPUT);

  gpio_set_level(S0, GPIO_LEVEL_LOW);
  gpio_set_level(S1, GPIO_LEVEL_HIGH);

  gpio_set_level(S2, GPIO_LEVEL_HIGH);
  gpio_set_level(S3, GPIO_LEVEL_HIGH);

  struct timeval check, start;
  int level, level_prev = 0;
  float v;
  gettimeofday(&start, NULL);
  int32_t start_us = start.tv_usec;

  while (1) {
    if (get_button_state(BUTTON0)) {
      break;
    }

    level = gpio_get_level(IN);
    v = adc_read_channel(ADC_IN);

    gettimeofday(&check, NULL);
    uint32_t check_us = check.tv_usec;

    if (level != level_prev && level == GPIO_LEVEL_HIGH) {
      uint32_t dt = (check_us - start_us);
      if (get_switch_state(SWITCH0)) {
        fprintf(stderr, "%d\n", dt);
      }
      gettimeofday(&start, NULL);
      start_us = start.tv_usec;
    }

    level_prev = level;

    gpio_set_level(CHECK, level);
  }

  gpio_reset();

  pynq_destroy();
  return 0;
}
