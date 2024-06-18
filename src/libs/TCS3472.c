#include "TCS3472.h"

#include <libpynq.h>
#include <stdio.h>
#include <string.h>


#include "../libs/measurements.h"
#include "i2c.h"

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
  bool err = i2c_write8(TCS3472_ADDR, TCS3472_COMMAND_BIT | TCS3472_ENABLE, 0, sensor->iic);
  return err;
}

#define WHITE_SENSITIVITY 2000
#define TRESHHOLD 200
color_t tcs3472_determine_color(tcs3472_t *sensor) {
  if (sensor->c < 0.5 * WHITE_SENSITIVITY) {
    return BLACK;
  }
  uint16_t rr = sensor->c / sensor->r;
  uint16_t rg = sensor->c / sensor->g;
  uint16_t rb = sensor->c / sensor->b;

  if (abs(rr - rg) < TRESHHOLD && abs(rr - rb) < TRESHHOLD && abs(rg - rb) < TRESHHOLD) {
    return WHITE;
  }
  return MAX(rr, MAX(rg, rb));
}

const char *COLOR_NAME(size_t index) {
  if (index >= COLOR_COUNT) {
    ERROR("color index out of range\n");
    return NULL;
  }
  return color_names[index];
}

void tcs3472_destroy(tcs3472_t *sensor) {
  if (sensor != NULL) {
    free(sensor);
  }
}
