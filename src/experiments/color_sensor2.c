#include <libpynq.h>
#include <stdio.h>
#define TCS_3472x_ADDR 0x12
#define TCS_3472x_ID 0x29

int main(void) {
  switchbox_init();
  switchbox_set_pin(IO_AR_SCL, SWB_IIC0_SCL);
  switchbox_set_pin(IO_AR_SDA, SWB_IIC0_SDA);
  iic_init(IIC0);


  uint8_t i;
  if (iic_read_register(IIC0, 0x29, 0x80 | 0x12, &i, 1)) {
    printf("register[%x]=error\n", 0x12);
  } else {
    printf("register[%x]=%x\n", 0x12, i);
  }
  if (iic_read_register(IIC0, 0x29, 0x80 | 0x13, &i, 1)) {
    printf("register[%x]=error\n", 0x12);
  } else {
    printf("register[%x]=%x\n", 0x12, i);
  }
  uint8_t data = 0b00011011;
  if (iic_read_register(IIC0, 0x29, 0x80 | 0x0, &i, 1)) {
    printf("register[%x]=error\n", 0x0);
  } else {
    printf("register[%x]=%x\n", 0x0, i);
  }
  uint16_t r, g, b;
  size_t j = 0;
  while (j < 10) {
    iic_read_register(IIC0, 0x29, 0x80 | 0x16, &r, 2);
    iic_read_register(IIC0, 0x29, 0x80 | 0x18, &g, 2);
    iic_read_register(IIC0, 0x29, 0x80 | 0x1A, &b, 2);

    printf("r:%d, g:%d, b:%d\n", r, g, b);

    sleep_msec(500);
    j++;
  }
  data = 0;
  iic_write_register(IIC0, 0x29, 0x80 | 0, &data, 1);

  iic_destroy(IIC0);
  switchbox_destroy();
  return 0;
}
