#ifndef I2C_H_
#define I2C_H_
#include <libpynq.h>


/**
 * @brief reads 1 byte from I2C.
 * @param adress The I2C device adress.
 * @param reg The I2C register to read.
 * @param a The pointer where to write data.
 * @param iic The IIC to us (IIC0 or IIC1).
 * @warning Fails with program exit if the IIC channel is outside valid range.
 */
bool i2c_read8(uint8_t adress, uint16_t reg, uint8_t *a, iic_index_t iic);

/**
 * @brief reads 2 byte from I2C.
 * @param adress The I2C device adress.
 * @param reg The I2C register to read.
 * @param a The pointer where to write data.
 * @param iic The IIC to us (IIC0 or IIC1).
 * @warning Fails with program exit if the IIC channel is outside valid range.
 */
bool i2c_read16(uint8_t adress, uint16_t reg, uint16_t *a, iic_index_t iic);

/**
 * @brief writes 1 byte from I2C.
 * @param adress The I2C device adress.
 * @param reg The I2C register to write.
 * @param a The data to write.
 * @param iic The IIC to us (IIC0 or IIC1).
 * @warning Fails with program exit if the IIC channel is outside valid range.
 */
bool i2c_write8(uint8_t adress, uint16_t reg, uint8_t a, iic_index_t iic);

/**
 * @brief writes 2 byte from I2C.
 * @param adress The I2C device adress.
 * @param reg The I2C register to write.
 * @param a The data to write.
 * @param iic The IIC to us (IIC0 or IIC1).
 * @warning Fails with program exit if the IIC channel is outside valid range.
 */
bool i2c_write16(uint8_t adress, uint16_t reg, uint16_t a, iic_index_t iic);
#endif
