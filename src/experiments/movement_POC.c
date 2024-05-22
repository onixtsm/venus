#include <libpynq.h>
#include <stdio.h>
#include <stepper.h>
#include "../libs/movement.h"

int main(void) {
  pynq_init();
  gpio_init();

  stepper_init();
  stepper_enable();


  stepper_set_speed(60000,  60000);

    char cmd;
    float distance;
    int input;
    float degrees;
    //int in;

  do{
    printf("Command? ");
    scanf(" %c", &cmd);

    switch(cmd)
    {
        case 'S': //straight
            printf("Distance? ");
            scanf(" %f", &distance);
            printf("Direction? ");
            scanf(" %d", &input);
            m_forwardOr(distance, input == 0 ? backward : forward);
            break;
        case 'R': //rotation
            printf("Degrees? ");
            scanf(" %f", &degrees);
            printf("Direction? "); //if 1: right & if 0: left
            scanf(" %d", &input);
            m_turnDegrees(degrees, input == 0 ? left : right);
            break;
        case 'W':; //waggle
            int q, i, d = 0;
            printf("Movement? \n");
            scanf(" %d",&q);        //checks whether there are no obstacles
            printf("Step size? \n");
            scanf("%d", &i);
            printf("Total distance? \n");
            scanf(" %d", &d);
            m_waggle(i,d);

            break;
        case 'q':
            printf("Bye!");
            break;
        default:
            printf("Unknown command %c", cmd);
            break;
    }
  } while(cmd != 'q');

  return 0;
}
