#include "TCS3472.h"

#include <libpynq.h>
#include <stdio.h>

#include "i2c.h"
#include "../libs/measurements.h"

// https://github.com/adafruit/Adafruit_TCS34725/tree/master

bool tcs3472_init(int iic) {
  uint8_t x;
  bool err = i2c_read8(TCS3472_ADDR, TCS3472_ID | TCS3472_COMMAND_BIT, &x, iic);
  if (err || x != 0x4d) {
    return 1;
  }
  return 0;
}

bool tcs3472_enable(int iic) {
  uint8_t x;
  i2c_read8(TCS3472_ADDR, TCS3472_COMMAND_BIT | TCS3472_ENABLE, &x, iic);
  x = x | TCS3472_ENABLE_AEN | TCS3472_ENABLE_PON;
  bool err = i2c_write8(TCS3472_ADDR, TCS3472_COMMAND_BIT | TCS3472_ENABLE, x, iic);
  return err;
}

void tcs3472_read_colors(int iic, uint16_t *c, uint16_t *r, uint16_t* g, uint16_t *b) {
  bool err;
  if (c != NULL) {
    err = i2c_read16(TCS3472_ADDR, TCS3472_REG_C | TCS3472_COMMAND_BIT, c, iic);
    if (err) {
      fprintf(stderr, "[ERROR] Could not read clear color reg \n");
    }
  }

  if (r != NULL) {
    i2c_read16(TCS3472_ADDR, TCS3472_REG_R | TCS3472_COMMAND_BIT, r, iic);
    if (err) {
      fprintf(stderr, "[ERROR] Could not read red color reg \n");
    }
  }
  if (g != NULL) {
    i2c_read16(TCS3472_ADDR, TCS3472_REG_G | TCS3472_COMMAND_BIT, g, iic);
    if (err) {
      fprintf(stderr, "[ERROR] Could not read green color reg \n");
    }
  }
  if (b != NULL) {
    i2c_read16(TCS3472_ADDR, TCS3472_REG_B | TCS3472_COMMAND_BIT, b, iic);
    if (err) {
      fprintf(stderr, "[ERROR] Could not read blue color reg \n");
    }
  }
}

bool tcs3472_disable(int iic) {

  bool err = i2c_write8(TCS3472_ADDR, TCS3472_COMMAND_BIT | TCS3472_ENABLE, 0, iic);
  return err;
}

#define WHITE_SENSITIVITY 2000
#define TRESHHOLD 200
color_t tcs3472_determine_color(uint16_t c, uint16_t r, uint16_t g, uint16_t b) {
  if (c < 0.5 * WHITE_SENSITIVITY) {
    return BLACK;
  }
  uint16_t rr = c / r;
  uint16_t rg = c / g;
  uint16_t rb = c / b;

  if (abs(rr - rg) < TRESHHOLD && abs(rr - rb) < TRESHHOLD && abs(rg - rb) < TRESHHOLD) {
    return WHITE;
  }
  return MAX(rr, MAX(rg, rb));
  
}

const char *COLOR_NAME(size_t index) {
  if (index >= COLOR_COUNT) {
    printf("[ERROR] color index out of range\n");
    return NULL;
  }
  return color_names[index];
}
