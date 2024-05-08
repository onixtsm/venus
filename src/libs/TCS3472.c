#include "TCS3472.h"

#include <libpynq.h>
#include <stdio.h>

#include "i2c.h"

bool tcs3472_init(void) {
  uint8_t x;
  bool err = i2c_read8(TCS3472_ADDR, TCS3472_ID | TCS3472_COMMAND_BIT, &x, IIC0);
  if (!err || x != 0x4d) {
    return true;
  }
  return err;
}

bool tcs3472_enable(void) {
  uint8_t x;
  i2c_read8(TCS3472_ADDR, TCS3472_COMMAND_BIT | TCS3472_ENABLE, &x, IIC0);
  printf("x:%08b\n", x);
  x = x | TCS3472_ENABLE_AEN | TCS3472_ENABLE_PON;
  printf("x:%08b\n", x);
  bool err = i2c_write8(TCS3472_ADDR, TCS3472_COMMAND_BIT | TCS3472_ENABLE, x, IIC0);
  return err;
}

void tcs3472_read_colors(uint16_t *c, uint16_t *r, uint16_t* g, uint16_t *b) {
  bool err;
  if (c != NULL) {
    err = i2c_read16(TCS3472_ADDR, TCS3472_REG_C | TCS3472_COMMAND_BIT, c, IIC0);
    if (err) {
      fprintf(stderr, "[ERROR] Could not read clear color reg \n");
    }
  }

  if (r != NULL) {
    i2c_read16(TCS3472_ADDR, TCS3472_REG_R | TCS3472_COMMAND_BIT, r, IIC0);
    if (err) {
      fprintf(stderr, "[ERROR] Could not read red color reg \n");
    }
  }
  if (g != NULL) {
    i2c_read16(TCS3472_ADDR, TCS3472_REG_G | TCS3472_COMMAND_BIT, g, IIC0);
    if (err) {
      fprintf(stderr, "[ERROR] Could not read green color reg \n");
    }
  }
  if (b != NULL) {
    i2c_read16(TCS3472_ADDR, TCS3472_REG_B | TCS3472_COMMAND_BIT, b, IIC0);
    if (err) {
      fprintf(stderr, "[ERROR] Could not read blue color reg \n");
    }
  }
}

bool tcs3472_disable(void) {

  bool err = i2c_write8(TCS3472_ADDR, TCS3472_COMMAND_BIT | TCS3472_ENABLE, 0, IIC0);
  return err;
}
