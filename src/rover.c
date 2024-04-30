#include <stdio.h>
#include "libs/foo.h"
#include <libpynq.h>

int main(void) {
  pynq_init();
  gpio_init();
  printf("%d\n", foo(1, 2));
  return 0;
}
