#include "TCS3472.h"

#include <libpynq.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "../libs/measurements.h"
#include "../settings.h"
#include "i2c.h"

// https://github.com/adafruit/Adafruit_TCS34725/tree/master
//

bool set_integration_time(tcs3472_t *sensor) {
  bool err = i2c_write8(TCS3472_ADDR, TC3472_REG_ATIME, 60, sensor->iic);
  if (!err) {
    sensor->integration_time_ns = 60;
  }
  return err;
}

hsl_t rgb_to_hsl(tcs3472_t *sensor) {
  hsl_t hsl_color;
  float nr = (float)sensor->r / 30000;
  float ng = (float)sensor->g / 30000;
  float nb = (float)sensor->b / 30000;

  float c_max = MAX(nr, MAX(ng, nb));
  float c_min = MIN(nr, MIN(ng, nb));

  float delta = c_max - c_min;

  hsl_color.l = (c_max + c_min) / 2;

  if (delta == 0) {
    hsl_color.h = 0;
  } else if (c_max == nr) {
    hsl_color.h = (((ng - nb) / delta));
  } else if (c_max == ng) {
    hsl_color.h = (((nb - nr) / delta + 2));
  } else {
    hsl_color.h = (((nr - ng) / delta + 4));
  }

  if (delta == 0) {
    hsl_color.s = 0;
  } else {
    hsl_color.s = delta / (1 - fabsf(2 * hsl_color.l - 1));
  }
  while (hsl_color.h > 1) {
    hsl_color.h -= 1;
  }
  return hsl_color;
}

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

  set_integration_time(sensor);
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

color_t tcs3472_determine_single_color(tcs3472_t *sensor) {
  tcs3472_read_colors(sensor);
  hsl_t hsl_colors = rgb_to_hsl(sensor);

  if (hsl_colors.l < 0.06) {
    return BLACK;
  }
  if (hsl_colors.l > 0.2) {
    return WHITE;
  }
  if (hsl_colors.h >= 0 && hsl_colors.h < 1.0/3) {
    return RED;
  }
  if (hsl_colors.h >= 1.0/3 && hsl_colors.h < 2.0/3) {
    return BLUE;
  }
  if (hsl_colors.h >= 2.0/3 && hsl_colors.h <= 1) {
    return GREEN;
  }
  return COLOR_COUNT;
}

color_t tcs3472_determine_color(tcs3472_t *sensor) {
  color_t a[COLOR_COUNT] = {0};
  for (size_t i = 0; i < TCS3472_READING_COUNT; ++i) {
    color_t color;
    do {
      color = tcs3472_determine_single_color(sensor);
      sleep_msec(35);
    } while (color == COLOR_COUNT);
    sleep_msec(100);
    a[color]++;
  }
  size_t index, max = 0;

  for (size_t i = 0; i < COLOR_COUNT; ++i) {
    if (max < a[i]) {
      max = a[i];
      index = i;
    }
  }
  return index;
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
