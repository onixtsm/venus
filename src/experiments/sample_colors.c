#include <libpynq.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>

#include "../libs/measurements.h"

#define IN IO_AR9
#define S0 IO_AR10
#define S1 IO_AR11
#define S2 IO_AR12
#define S3 IO_AR13


typedef struct _data_t {
  uint32_t dtw;
  uint32_t dtr;
  uint32_t dtg;
  uint32_t dtb;
} data_t;


void print_data(const data_t *data, const char *path, size_t sample_size) {
  FILE *f = fopen(path, "wb");
  for (size_t i = 0; i < sample_size; ++i) {
    fprintf(f, "%d,%d,%d,%d\n", data[i].dtw, data[i].dtr, data[i].dtb, data[i].dtg);
  }
  fclose(f);
}

void sample_data(const char *path, size_t sample_size) {
  data_t *buffer = malloc(sizeof(*buffer) * sample_size);
  for (size_t i = 0; i < sample_size; ++i) {
    gpio_set_level(S2, 1);
    gpio_set_level(S3, 0);
    buffer[i].dtw = get_period(IN, 1);

    gpio_set_level(S2, 0);
    gpio_set_level(S3, 0);
    buffer[i].dtr = get_period(IN, 1);

    gpio_set_level(S2, 0);
    gpio_set_level(S3, 1);
    buffer[i].dtb = get_period(IN, 1);

    gpio_set_level(S2, 1);
    gpio_set_level(S3, 1);
    buffer[i].dtg = get_period(IN, 1);
  }
  print_data(buffer, path, sample_size);
  free(buffer);
}

int main(int argc, char** argv) {
  size_t sample_size;

  if (argc == 1) {
    sample_size = 1000;
  } else {
    sample_size = atol(argv[1]);
  }

  pynq_init();
  gpio_init();
  adc_init();
  buttons_init();
  switches_init();

  gpio_set_direction(S0, GPIO_DIR_OUTPUT);
  gpio_set_direction(S1, GPIO_DIR_OUTPUT);
  gpio_set_direction(S2, GPIO_DIR_OUTPUT);
  gpio_set_direction(S3, GPIO_DIR_OUTPUT);

  gpio_set_level(S0, GPIO_LEVEL_LOW);
  gpio_set_level(S1, GPIO_LEVEL_HIGH);

  sample_data("data.csv", sample_size);

  pynq_destroy();
}
