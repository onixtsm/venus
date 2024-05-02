#include <libpynq.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>

#define IN IO_AR9
#define S0 IO_AR10
#define S1 IO_AR11
#define S2 IO_AR12
#define S3 IO_AR13


typedef struct _data_t {
  uint32_t dtw;
  uint32_t dtr;
  uint32_t dtb;
  uint32_t dtg;
} data_t;

uint32_t get_preiod(uint8_t pin, uint8_t level) {
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
    buffer[i].dtw = get_preiod(IN, 1);

    gpio_set_level(S2, 0);
    gpio_set_level(S3, 0);
    buffer[i].dtr = get_preiod(IN, 1);

    gpio_set_level(S2, 0);
    gpio_set_level(S3, 1);
    buffer[i].dtb = get_preiod(IN, 1);

    gpio_set_level(S2, 1);
    gpio_set_level(S3, 1);
    buffer[i].dtg = get_preiod(IN, 1);
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
