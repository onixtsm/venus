#include "TCS3472.h"

#include <float.h>
#include <libpynq.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "../libs/measurements.h"
#include "i2c.h"

// https://github.com/adafruit/Adafruit_TCS34725/tree/master

hsv_t rgb2hsv(tcs3472_t sensor) {
  rgb_t in;
  in.r = (double)sensor.r / 30000;
  in.g = (double)sensor.g / 30000;
  in.b = (double)sensor.b / 30000;

  hsv_t out;
  double min, max, delta;

  min = in.r < in.g ? in.r : in.g;
  min = min < in.b ? min : in.b;

  max = in.r > in.g ? in.r : in.g;
  max = max > in.b ? max : in.b;

  out.v = max;  // v
  delta = max - min;
  if (delta < 0.00001) {
    out.s = 0;
    out.h = 0;  // undefined, maybe nan?
    return out;
  }
  if (max > 0.0) {          // NOTE: if Max is == 0, this divide would cause a crash
    out.s = (delta / max);  // s
  } else {
    // if max is 0, then r = g = b = 0
    // s = 0, h is undefined
    out.s = 0.0;
    out.h = FLT_MAX;  // its now undefined
    return out;
  }
  if (in.r >= max)                  // > is bogus, just keeps compilor happy
    out.h = (in.g - in.b) / delta;  // between yellow & magenta
  else if (in.g >= max)
    out.h = 2.0 + (in.b - in.r) / delta;  // between cyan & yellow
  else
    out.h = 4.0 + (in.r - in.g) / delta;  // between magenta & cyan

  out.h *= 60.0;  // degrees

  if (out.h < 0.0) out.h += 360.0;

  return out;
}

bool set_integration_time(tcs3472_t *sensor) {
  bool err = i2c_write8(TCS3472_ADDR, TC3472_REG_ATIME | TCS3472_COMMAND_BIT, 60, sensor->iic);
  if (!err) {
    sensor->integration_time_ns = 60;
  }
  return err;
}

hsl_t rgb_to_hsl(tcs3472_t *sensor) {
  hsl_t hsl_color;
  uint8_t ur = (sensor->r >> 4);
  uint8_t ug = (sensor->g >> 4);
  uint8_t ub = (sensor->b >> 4);

  float nr = ((float)ur / 255);
  float ng = ((float)ug / 255);
  float nb = ((float)ub / 255);

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
  if (hsl_color.h < 0) {
    hsl_color.h = -hsl_color.h;
  }
  LOG("h:%f, s:%f, l:%f", hsl_color.h, hsl_color.s, hsl_color.l);
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

  return sensor;
}

bool set_gain(tcs3472_t *sensor, uint8_t gain) {
  bool err = 1;
  if (sensor->enable == 1) {
    // err = i2c_write8(TCS3472_ADDR, TCS3472_CONTROL_REG | TCS3472_COMMAND_BIT, gain | 0x03, sensor->iic);
    err = i2c_write8(TCS3472_ADDR, TCS3472_CONTROL_REG | TCS3472_COMMAND_BIT, gain, sensor->iic);
  }
  return err;
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
  set_gain(sensor, 1);
  set_integration_time(sensor);
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

  if (hsl_colors.l < 0.05) {
    return BLACK;
  }
  if (hsl_colors.l > 0.2) {
    return WHITE;
  }
  if (hsl_colors.h >= 10 && hsl_colors.h < 25) {
    return RED;
  }
  if (hsl_colors.h >= 90 && hsl_colors.h < 100) {
    return GREEN;
  }
  if (hsl_colors.h >= 175 && hsl_colors.h < 200) {
    return BLUE;
  }
  return COLOR_COUNT;
}

color_t tcs3472_determine_color(tcs3472_t *sensor) {
  hsv_t hsv_colors = {0};
  for (size_t i = 0; i < 30; ++i) {
    tcs3472_read_colors(sensor);
    hsv_t t = rgb2hsv(*sensor);
    hsv_colors.h = incremental_mean(t.h, hsv_colors.h, i);
    hsv_colors.s = incremental_mean(t.s, hsv_colors.s, i);
    hsv_colors.v = incremental_mean(t.v, hsv_colors.v, i);
  }

  if (sensor->iic == IIC0) {
    if (hsv_colors.v < 0.05) {
      return BLACK;
    }
    if (hsv_colors.v > 0.178) {
      return WHITE;
    }
    if (hsv_colors.h >= 10 && hsv_colors.h < 25) {
      return RED;
    }
    if (hsv_colors.h >= 90 && hsv_colors.h < 100) {
      return GREEN;
    }
    if (hsv_colors.h >= 175 && hsv_colors.h < 200) {
      return BLUE;
    }
    return WHITE;
  } else {
    LOG("Bottom color: %f", hsv_colors.v);
    if (hsv_colors.v < 0.15) {
    
      return BLACK;
    }
    return WHITE;
  }
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
