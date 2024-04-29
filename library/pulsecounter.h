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
#ifndef PULSECOUNTER_H
#define PULSECOUNTER_H
#include <gpio.h>
#include <stdbool.h>
#include <stdint.h>

/**
 * @defgroup PULSECOUNTER PULSECOUNTER library
 *
 * @brief Functions to use the PULSECOUNTER.
 *
 * This interacts with hardware pulsecounter.
 * It can count up to 2**31-1 pulses and has a free-running 32bit counter that can be
 * read out consistently with the pulsecount to get an accurate pulses/time unit.
 * The module runs at ~100MHz.
 *
 * @{
 **/

/**
 * @brief Enum of PULSECOUNTERs.
 * Functions use a switch numbered from 0..NUM_PULSECOUNTERS-1.
 */
typedef enum { PULSECOUNTER0 = 0, PULSECOUNTER1 = 1, NUM_PULSECOUNTERS = 2 } pulsecounter_index_t;

/**
 * @param uart the pulsecounter index to initialize.
 * @brief initialize the pulsecounter specified by the index with a shared
 * memory pointer
 */
extern void pulsecounter_init(const pulsecounter_index_t pci);

/**
 * @brief Close the shared memory handle for the specified PULSECOUNTER index.
 * @param pci the pulsecounter index to remove from the shared memory space.
 * @warning fails with program exit if the pulsecounter channel is outside valid
 * range.
 */
extern void pulsecounter_destroy(const pulsecounter_index_t pci);

/**
 * @param pci the pulsecounter index.
 * @param timestamp The timestamp matching the count value. [out]
 *
 * Reads the content of the register into data.
 *
 * @return the read count and write associated timestamp to *timestamp
 */
extern uint32_t pulsecounter_get_count(const pulsecounter_index_t pci, uint32_t *timestamp);

/**
 * @param pci the pulsecounter index.
 *
 * Reset the pulsecounter count back to 0.
 */
extern void pulsecounter_reset_count(const pulsecounter_index_t pci);

/**
 * @param pci the pulcounter index
 * @param edge The edge to trigger on
 *
 * Set high to trigger on the rising edge, set to low to trigger on falling
 * edge. The signal needs to be stable at 'edge' level for filter length (10 by
 * default) cycles.
 *
 */
extern void pulsecounter_set_edge(const pulsecounter_index_t pci, const gpio_level_t edge);

/**
 * @param pci the pulcounter index
 *
 * @returns the edge that is used to trigger on. See set_edge.
 */
extern gpio_level_t pulsecounter_get_edge(const pulsecounter_index_t pci);

/**
 * @param pci the pulcounter index
 * @param count the filter count
 *
 * Set the debounce filter length. Default is 10 cycles, can be set from 1 to 15
 * cycles.
 */
extern void pulsecounter_set_filter_length(const pulsecounter_index_t pci, uint8_t const count);

/**
 * @param pci the pulcounter index
 *
 * @return the current filter length.
 */
uint8_t pulsecounter_get_filter_length(const pulsecounter_index_t pci);
/**
 * @}
 */
#endif
