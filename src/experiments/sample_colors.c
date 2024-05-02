#include <libpynq.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include "util.h"

#define IN IO_AR13
#define ADC_IN ADC0
#define S0 IO_AR7
#define S1 IO_AR6
#define S2 IO_AR4
#define S3 IO_AR5
#define CHECK IO_A1

#define SAMPLE_SIZE 1000000

typedef struct _data_t {
  uint8_t level;
  struct timeval time;
} data_t;

void print_data(const data_t* data, struct timeval start, const char* path) {
  FILE* f = fopen(path, "wb");
  long int dt;
  for (size_t i = 0; i < SAMPLE_SIZE; ++i) {
    dt = (data[i].time.tv_sec - start.tv_sec) * 1000 * 1000 + (data[i].time.tv_usec - start.tv_usec);
    fprintf(f, "%ld,%d\n", dt, data[i].level);
  }
}

void get_data(uint8_t s, char *name) {
  data_t* data = malloc(SAMPLE_SIZE * sizeof(*data));

  gpio_set_level(S2, (s >> 1) & 1);
  gpio_set_level(S3, s & 1);
  struct timeval check, start;
  gettimeofday(&start, NULL);
  for (size_t i = 0; i < SAMPLE_SIZE; ++i) {
    data[i].level = gpio_get_level(IN);
    gettimeofday(&check, NULL);
    data[i].time = check;
  }
  print_data(data, start, name);
  free(data);
}

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

  // No filter
  get_data(0, "red.csv");
  get_data(0b01, "blue.csv");
  get_data(0b10, "white.csv");
  get_data(0b11, "green.csv");

  pynq_destroy();
}
