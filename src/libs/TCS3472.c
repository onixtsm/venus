#include "TCS3472.h"

#include <libpynq.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "../libs/measurements.h"
#include "../settings.h"
#include "i2c.h"
#include "util.h"

// https://github.com/adafruit/Adafruit_TCS34725/tree/master

tcs3472_t *tcs3472_init(int iic) {
  tcs3472_t *sensor = malloc(sizeof(*sensor));
  if (sensor == NULL) {
    ERROR();
    exit(1);
  }
  memset(sensor, 0, sizeof(*sensor));
  uint8_t x;
  bool err = i2c_read8(TCS3472_ADDR, TCS3472_ID | TCS3472_COMMAND_BIT, &x, iic);
  if (err || x != 0x4d) {
    return NULL;
  }
  sensor->iic = iic;
  return sensor;
}

bool tcs3472_enable(tcs3472_t *sensor) {
  if (sensor->enable) {
    return 0;
  }
  uint8_t x;
  i2c_read8(TCS3472_ADDR, TCS3472_COMMAND_BIT | TCS3472_ENABLE, &x, sensor->iic);
  x = x | TCS3472_ENABLE_AEN | TCS3472_ENABLE_PON;
  if (i2c_write8(TCS3472_ADDR, TCS3472_COMMAND_BIT | TCS3472_ENABLE, x, sensor->iic)) {
    return 1;
  }
  sensor->enable = true;
  return false;
}

void tcs3472_read_colors(tcs3472_t *sensor) {
  if (sensor == NULL || !sensor->enable) {
    return;
  }
  bool err;
  err = i2c_read16(TCS3472_ADDR, TCS3472_REG_C | TCS3472_COMMAND_BIT, &sensor->c, sensor->iic);
  if (err) {
    ERROR("Could not read clear color reg ");
  }

  i2c_read16(TCS3472_ADDR, TCS3472_REG_R | TCS3472_COMMAND_BIT, &sensor->r, sensor->iic);
  if (err) {
    ERROR("Could not read red color reg ");
  }
  i2c_read16(TCS3472_ADDR, TCS3472_REG_G | TCS3472_COMMAND_BIT, &sensor->g, sensor->iic);
  if (err) {
    ERROR("Could not read green color reg ");
  }
  i2c_read16(TCS3472_ADDR, TCS3472_REG_B | TCS3472_COMMAND_BIT, &sensor->b, sensor->iic);
  if (err) {
    ERROR("Could not read blue color reg ");
  }
}

bool tcs3472_disable(tcs3472_t *sensor) {
  if (!sensor->enable) {
    return false;
  }

  if (i2c_write8(TCS3472_ADDR, TCS3472_COMMAND_BIT | TCS3472_ENABLE, 0, sensor->iic)) {
    return true;
  }
  sensor->enable = false;
  return 0;
}

void print_colors(tcs3472_t *sensor) { LOG("c: %d, r: %d, g: %d, b: %d\n", sensor->c, sensor->r, sensor->g, sensor->b); }
/*
color_t tcs3472_determine_color(tcs3472_t *sensor) {
  print_colors(sensor);
  if (sensor->c < 0.5 * WHITE_SENSITIVITY) {
    return BLACK;
  }
  float rr = (float)sensor->c / sensor->r;
  float rg = (float)sensor->c / sensor->g;
  float rb = (float)sensor->c / sensor->b;

  if (fabs(rr - rg) < TRESHHOLD && fabs(rr - rb) < TRESHHOLD && fabs(rg - rb) < TRESHHOLD) {
    return WHITE;
  }
  float max = MAX(sensor->r, MAX(sensor->g, sensor->b));
  if (max == sensor->r) return RED;
  if (max == sensor->g) return GREEN;
  if (max == sensor->b) return BLUE;
  return WHITE;
}
*/




color_t tcs3472_determine_single_color(tcs3472_t *sensor) {
  float nr = (float)sensor->r / sensor->c;
  float ng = (float)sensor->g / sensor->c;
  float nb = (float)sensor->b / sensor->c;

  float sum = nr + ng + nb;

  float rr = nr / sum;
  float rg = ng / sum;
  float rb = nb / sum;

  if (sensor->c < BLACK_TRESHOLD) {
    return BLACK;
  }
  if (sensor->c > WHITE_TRESHOLD) {
    return WHITE;
  }
  if (rr > rg && rr > rb) {
    return RED;
  }
  if (rg > rr && rg > rb) {
    return GREEN;
  }
  if (rb > rr && rb > rg) {
    return BLUE;
  }
  return WHITE;
}

color_t tcs3472_determine_color(tcs3472_t *sensor) {
  color_t a[COLOR_COUNT] = {0};
  for (size_t i = 0; i < TCS3472_READING_COUNT; ++i) {
    a[tcs3472_determine_single_color(sensor)]++;
    sleep_msec(100);
  }
  color_t max = COLOR_COUNT;
  for (size_t i = 0; i < COLOR_COUNT; ++i) {
    if (a[i] > max) {
      max = i;
    }
  }
  return max;
}

const char *COLOR_NAME(size_t index) {
  if (index >= COLOR_COUNT) {
    ERROR("color index out of range (%zu)\n", index);
    return NULL;
  }
  return color_names[index];
}

void tcs3472_destroy(tcs3472_t *sensor) {
  if (sensor != NULL) {
    free(sensor);
  }
}
