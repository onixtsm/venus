#include <libpynq.h>
#include <stdio.h>
#include "i2c.h"


bool iic_read8(uint8_t adress, uint16_t reg, uint8_t *a, iic_index_t iic) {
  if (iic > 1 || iic < 0) {
    fprintf(stderr, "[ERROR] Wrong IIC number: %d\n", iic);
    return 1;
  }
  bool ok = iic_read_register(iic, adress, reg, a, 1);
  return ok;
}

bool iic_read16(uint8_t adress, uint16_t reg, uint16_t *a, iic_index_t iic) {
  if (iic > 1 || iic < 0) {
    fprintf(stderr, "[ERROR] Wrong IIC number: %d\n", iic);
    return 1;
  }
  bool ok = iic_read_register(iic, adress, reg, (uint8_t *)a, 2);
  return ok;
}

bool iic_write8(uint8_t adress, uint16_t reg, uint8_t a, iic_index_t iic) {
  if (iic > 1 || iic < 0) {
    fprintf(stderr, "[ERROR] Wrong IIC number: %d\n", iic);
    return 1;
  }
  bool ok = iic_write_register(iic, adress, reg, &a, 1);
  return ok;
}

bool iic_write16(uint8_t adress, uint16_t reg, uint16_t a, iic_index_t iic) {
  if (iic > 1 || iic < 0) {
    fprintf(stderr, "[ERROR] Wrong IIC number: %d\n", iic);
    return 1;
  }
  bool ok = iic_write_register(iic, adress, reg, (uint8_t *)&a, 2);
  return ok;
}
