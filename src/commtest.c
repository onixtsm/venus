#include "comms.h"
#include <libpynq.h>

int main(void){
  pynq_init();
  uart_init(UART0);
  uart_reset_fifos(UART0);
  switchbox_set_pin(IO_AR0, SWB_UART0_RX);
  switchbox_set_pin(IO_AR1, SWB_UART0_TX);
  
  /*
  char *json = encodeJson(1, 2, 3, 4, 5);
  printf("%s\n", json);

  decodeJson(&x, &y, &obj_found, &color_found, &status, json);
  printf("x: %f, y: %f, obj: %i, color: %i, status: %i\n", x, y, obj_found, color_found, status);
  */

  /*
  send_msg(6, 7, 0, 8, 0);
    */
/*
  decodeJson(&x, &y, &obj_found, &color_found, &status, json);
  printf("x: %f, y: %f, obj: %i, color: %i\n", x, y, obj_found, color_found);
  */
    while(1){
        if(uart_has_data(UART0)){
            recv_msg(&x, &y, &obj_found, &color_found, &status);
            printf("X: %f\n Y:%f\n obj:%i\n color:%i\n status:%i\n", x,y,obj_found,color_found,status);
        }
        
    }
  
    printf("end");
  
  uart_destroy(UART0);
}
