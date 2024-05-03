#include <stdint.h>
#include <sys/time.h>
#include <libpynq.h>
#include "measurements.h"


uint32_t get_period(uint8_t pin, uint8_t level) {

  struct timeval end, start;
  gettimeofday(&start, NULL);

  uint8_t curr_level;
  do {
    curr_level = gpio_get_level(pin);
  } while (curr_level != level);
  gettimeofday(&start, NULL);
  do {
  curr_level = gpio_get_level(pin);
  } while (curr_level == level);

  gettimeofday(&end, NULL);

  return (end.tv_sec - start.tv_sec) * 1000 * 1000 + (end.tv_usec - start.tv_usec);
}
