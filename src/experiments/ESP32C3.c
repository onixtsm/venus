#include <libpynq.h>
#include <stdio.h>

int main(void) {
  pynq_init();
  switchbox_set_pin(IO_AR0, SWB_UART0_RX);
  switchbox_set_pin(IO_AR1, SWB_UART0_TX);

  uart_init(UART0);
  uart_reset_fifos(UART0);

  printf("AR2 %d\n", gpio_get_level(IO_AR2));
  printf("AR3 %d\n", gpio_get_level(IO_AR3));

  while (1) {
    uart_send(UART0, 4);
    uart_send(UART0, 0);
    uart_send(UART0, 0);
    uart_send(UART0, 0);
    uart_send(UART0, 'o');
    uart_send(UART0, 'n');
    uart_send(UART0, 'i');
    uart_send(UART0, 'x');
    sleep_msec(100);
  }
  pynq_destroy();
}
