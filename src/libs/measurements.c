#include "measurements.h"

#include <libpynq.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/time.h>

char name[10];

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

// https://math.stackexchange.com/questions/106700/incremental-averaging
double incremental_mean(double new_value, double running_mean, size_t count) {
  if (count < 2) {
    return new_value;
  }
  return running_mean + (new_value - running_mean) / count;
}

// https://www.arduino.cc/reference/en/language/functions/math/map/
int map(int x, int in_min, int in_max, int out_min, int out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

float generateRandomFloat(float min, float max) {  // yes, this function was c+p from chatGPT, fuck off
  // Generate a random float in the range [0, 1]
  float scale = rand() / (float)RAND_MAX;
  // Scale and shift the result to the desired range
  return min + scale * (max - min);
}
inline static float sqr(float x) { return x * x; }

void linear_regression(size_t n, const float *x, const float *y, float *a, float *b) {
  float sumx = 0;
  float sumx2 = 0;
  float sumxy = 0;
  float sumy = 0;

  for (size_t i = 0; i < n; ++i) {
    sumx += x[i];
    sumx2 += sqr(x[i]);
    sumxy += x[i] * y[i];
    sumy += y[i];
  }
  float denom = (n * sumx2 - sqr(sumx));
  if (denom == 0) {
    // singular matrix. can't solve the problem.
    *a = 0;
    *b = 0;
  }

  *a = (n * sumxy - sumx * sumy) / denom;
  *b = (sumy * sumx2 - sumx * sumxy) / denom;
}

bool should_die(void) {
  return get_button_state(BUTTON0) || get_switch_state(SWITCH0);
}
