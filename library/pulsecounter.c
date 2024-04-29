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
#include "pulsecounter.h"

#include <platform.h>
#include <stdio.h>
#include <string.h>

#include "arm_shared_memory_system.h"
#include "log.h"

static arm_shared pulsecounter_handles[NUM_PULSECOUNTERS];
static volatile uint32_t *pulsecounter_ptrs[NUM_PULSECOUNTERS] = {
    NULL,
};

const uint32_t PULSECOUNTER_PULSES = 0;
const uint32_t PULSECOUNTER_COUNTER = 1;
const uint32_t PULSECOUNTER_EDGE = 2;
const uint32_t PULSECOUNTER_FILTER = 3;

void pulsecounter_init(const pulsecounter_index_t pci) {
  if (!(pci >= PULSECOUNTER0 && pci < NUM_PULSECOUNTERS)) {
    pynq_error("invalid pci %d, must be 0..%d\n", pci, NUM_PULSECOUNTERS);
  }
  if (pci == PULSECOUNTER0) {
    pulsecounter_ptrs[pci] = arm_shared_init(&(pulsecounter_handles[pci]), axi_timer_0, 4096);
  } else if (pci == PULSECOUNTER1) {
    pulsecounter_ptrs[pci] = arm_shared_init(&(pulsecounter_handles[pci]), axi_timer_1, 4096);
  }
}

void pulsecounter_destroy(const pulsecounter_index_t pci) {
  if (!(pci >= PULSECOUNTER0 && pci < NUM_PULSECOUNTERS)) {
    pynq_error("invalid pci %d, must be 0..%d-1\n", pci, NUM_PULSECOUNTERS);
  }
  arm_shared_close(&(pulsecounter_handles[pci]));
  pulsecounter_ptrs[pci] = NULL;
}

uint32_t pulsecounter_get_count(const pulsecounter_index_t pci, uint32_t *timestamp) {
  if (!(pci >= PULSECOUNTER0 && pci < NUM_PULSECOUNTERS)) {
    pynq_error("invalid pci %d, must be 0..%d-1\n", pci, NUM_PULSECOUNTERS);
  }
  uint32_t retv = pulsecounter_ptrs[pci][PULSECOUNTER_PULSES];
  if (timestamp != NULL) {
    *timestamp = pulsecounter_ptrs[pci][PULSECOUNTER_COUNTER];
  }
  return retv;
}

void pulsecounter_set_edge(const pulsecounter_index_t pci, const gpio_level_t edge) {
  if (!(pci >= PULSECOUNTER0 && pci < NUM_PULSECOUNTERS)) {
    pynq_error("invalid pci %d, must be 0..%d-1\n", pci, NUM_PULSECOUNTERS);
  }
  pulsecounter_ptrs[pci][PULSECOUNTER_EDGE] = edge == GPIO_LEVEL_HIGH ? 1 : 0;
}

gpio_level_t pulsecounter_get_edge(const pulsecounter_index_t pci) {
  if (!(pci >= PULSECOUNTER0 && pci < NUM_PULSECOUNTERS)) {
    pynq_error("invalid pci %d, must be 0..%d-1\n", pci, NUM_PULSECOUNTERS);
  }
  return (pulsecounter_ptrs[pci][PULSECOUNTER_EDGE] ? GPIO_LEVEL_HIGH : GPIO_LEVEL_LOW);
}
void pulsecounter_reset_count(const pulsecounter_index_t pci) {
  if (!(pci >= PULSECOUNTER0 && pci < NUM_PULSECOUNTERS)) {
    pynq_error("invalid pci %d, must be 0..%d-1\n", pci, NUM_PULSECOUNTERS);
  }
  pulsecounter_ptrs[pci][PULSECOUNTER_PULSES] = 0;
}

uint8_t pulsecounter_get_filter_length(const pulsecounter_index_t pci) {
  if (!(pci >= PULSECOUNTER0 && pci < NUM_PULSECOUNTERS)) {
    pynq_error("invalid pci %d, must be 0..%d-1\n", pci, NUM_PULSECOUNTERS);
  }
  return pulsecounter_ptrs[pci][PULSECOUNTER_FILTER];
}

void pulsecounter_set_filter_length(const pulsecounter_index_t pci, uint8_t const count) {
  if (!(pci >= PULSECOUNTER0 && pci < NUM_PULSECOUNTERS)) {
    pynq_error("invalid pci %d, must be 0..%d-1\n", pci, NUM_PULSECOUNTERS);
  }
  if (count < 1 || count > 15) {
    pynq_error("Filter length needs to be between 1 and 16-1.");
  }
  pulsecounter_ptrs[pci][PULSECOUNTER_FILTER] = count;
}
