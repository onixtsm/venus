/*
Copyright (c) 2023 Eindhoven University of Technology

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#ifndef IIC_H
#define IIC_H
#include <stdbool.h>
#include <stdint.h>

/**
 * @defgroup IIC IIC library
 *
 * @brief Functions to use the Inter-Integrated Circuit (IIC).
 *
 * High-level functions to read/write to clients connected to the two
 * integrated IIC modules.
 * Before sending and receiving bytes the IIC2 must be connect to some I/O pins
 * through the switchbox (see switchbox.h), e.g. Pmod A:
 * @code
 * switchbox_set_pin(IO_PMODA3, SWB_IIC0_SCL);
 * switchbox_set_pin(IO_PMODA4, SWB_IIC0_SDA);
 * @endcode
 * or the SCL and SDA Arduino IIC pins:
 * @code
 * switchbox_set_pin(IO_AR_SCL, SWB_IIC0_SCL);
 * switchbox_set_pin(IO_AR_SDA, SWB_IIC0_SDA);
 * @endcode
 * The Pmod A pins (see pinmap.h) are good because they have 2K2 pull-up
 * resistors built in. If you want to use more than three IICs then you can use
 * different pins with an external 2K2 pull-up resistor, which should work for
 * more than three boards.
 *
 * After that, an example of how to use this library for the MASTER.
 * @code
 * #include <libpynq.h>
 * int main (void)
 * {
 *   // initialise all I/O
 *   pynq_init();
 *   iic_init(IIC0);
 *   uint32_t i;
 *   // you can use multiple slaves, here only one is shown
 *   uint32_t slave_address = 0x70;
 *   // read out all registers of the slave
 *   for (int reg=0; reg < 32; reg++) {
 *     if (iic_read_register(IIC0, slave_address, reg, (uint8_t *) &i, 4)) {
 *       // 4 means 4 bytes, do not change
 *       printf("register[%d]=error\n",reg); } else {
 *       printf("register[%d]=%d\n",reg,i);
 *     }
 *   }
 *   // clean up after use
 *   pynq_destroy();
 *   return EXIT_SUCCESS;
 * }
 * @endcode
 *
 * An example of how to use this library for the SLAVE.
 * @code
 * int main(void)
 * {
 *   // this is the address by which this slave is reached by the master
 *   // different slaves must have different addresses
 *   const uint32_t my_slave_address = 0x70;
 *   // array contains 32 registers that can be written & read by the master
 *   // the slave can of course modify the values of the registers
 *   uint32_t my_register_map[32] = {1,2,3,4,5,6,7,8,9,10,11,12,13,
 *        14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32};
 *   const uint32_t my_register_map_length =
 *                        sizeof(my_register_mmap)/sizeof(uint32_t);
 *
 *   pynq_init();
 *   iic_init(IIC0);
 *   iic_reset(IIC0);
 *   iic_set_slave_mode(IIC0, my_slave_address,
 *                      &(my_register_map[0]), my_register_map_length);
 *   while (1) {
 *     // the slave mode handler must be run regularly to react to the master
 *     iic_slave_mode_handler(IIC0);
 *     // insert your own code here, to do whatever the slave needs to do;
 *     // but ensure that you execute the slave mode handler regularly enough
 *     sleep_msec(10);
 *   }
 *   iic_destroy(IIC0);
 *   pynq_destroy();
 *   return EXIT_SUCCESS;
 * }
 * @endcode
 *
 * @{
 **/

/**
 * @brief Enum of IICs.
 * Functions use a switch numbered from 0..NUM_IICS-1.
 */
typedef enum { IIC0 = 0, IIC1 = 1, NUM_IICS = 2 } iic_index_t;

/**
 * @param uart The IIC index to initialize.
 * @brief Initialize the IIC specified by the index with a shared memory handle
 * and a buffer size of 4096 bytes.
 * @warning Fails with program exit if the IIC channel is outside valid range
 * or when the shared memory system has not been instantiated.
 */
extern void iic_init(const iic_index_t iic);

/**
 * @brief Close the shared memory handle for the specified IIC index.
 * @param uart The IIC index to remove from the shared memory space.
 * @warning Fails with program exit if the IIC channel is outside valid range.
 */
extern void iic_destroy(const iic_index_t iic);

/**
 * @param iic The IIC index to initialize.
 * @param addr The IIC address of the client to access.
 * @param reg The clients register address.
 * @param data Buffer where the register content is stored. [out]
 * @param length The amount of data to read.
 *
 * Reads the content of the register into data.
 *
 * @return 0 if successful, 1 on error
 */
extern bool iic_read_register(const iic_index_t iic, const uint8_t addr, const uint8_t reg, uint8_t *data, uint16_t length);

/**
 * @param iic The IIC index to initialize.
 * @param addr The IIC address of the client to access.
 * @param reg The clients register address.
 * @param data Buffer where new the register content is stored.
 * @param length The amount of data to write.
 *
 * Writes data to register.
 *
 * @return 0 if successful, 1 on error
 */
extern bool iic_write_register(const iic_index_t iic, const uint8_t addr, const uint8_t reg, uint8_t *data, uint16_t length);

extern bool iic_set_slave_mode(const iic_index_t iic, const uint8_t addr, uint32_t *register_map, const uint32_t rm_length);

/**
 * @param iic The IIC index of the hardware to use.
 *
 * This handles requests that came in to the IIC unit when it is in
 * slave mode.
 */
extern void iic_slave_mode_handler(const iic_index_t iic);

/**
 * @param iic The IIC index of the hardware to use.
 * Return the IIC module into its default mode.
 * This way it can be used as master.
 */
extern void iic_reset(const iic_index_t iic);
/**
 * @}
 */
#endif
