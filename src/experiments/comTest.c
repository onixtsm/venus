#include "../libs/comms.h"
#include <libpynq.h>
#include "../libs/vtypes.h"

int main(void){
  pynq_init();
  uart_init(UART0);
  uart_reset_fifos(UART0);
  switchbox_set_pin(IO_AR0, SWB_UART0_RX);
  switchbox_set_pin(IO_AR1, SWB_UART0_TX);

  robot_t robot = {0};
  obstacle_t obstacle = {0};


  send_msg(obstacle, robot);

  /*
    while(1){
    
        if(uart_has_data(UART0)){
            recv_msg(&obstacle, &robot);
                printf("robot data: %.2f %.2f %d\nobstacle data: %.2f %.2f %d %d\n",
                  robot.x, robot.y, robot.status,
                  obstacle.x, obstacle.y, obstacle.color, obstacle.type
                );
        }
        
    }
    */
  
    printf("end");
  
  uart_destroy(UART0);
}