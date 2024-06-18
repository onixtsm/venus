#include "VL53L0X.h"

#include <stdbool.h>
#include <stdio.h>

#include "i2c.h"
#include "../libs/measurements.h"

typedef enum { CALIBRATION_TYPE_VHV, CALIBRATION_TYPE_PHASE } calibration_type_t;


// const uint8_t address = VL53L0X_DEFAULT_ADDRESS;
// uint8_t stop_variable = 0;

bool data_init(vl53l0x_t *sensor) {
  bool err = 0;

  uint8_t vhv_config_scl_sda = 0;

  if (i2c_read8(sensor->address, VL53L0X_VHV_CONFIG_PAD_SCL_SDA_EXTSUP_HV, &vhv_config_scl_sda, IIC0)) {
    ERROR();
    return 1;
  }

  vhv_config_scl_sda |= 0x01;

  if (i2c_write8(sensor->address, VL53L0X_VHV_CONFIG_PAD_SCL_SDA_EXTSUP_HV, vhv_config_scl_sda, IIC0)) {
    ERROR();
    return 1;
  }

  /* standard i2c mode */
  /* magic numbers - have no clue */
  err &= i2c_write8(sensor->address, 0x88, 0x00, IIC0);
  err &= i2c_write8(sensor->address, 0x80, 0x01, IIC0);
  err &= i2c_write8(sensor->address, 0xFF, 0x01, IIC0);
  err &= i2c_write8(sensor->address, 0x00, 0x00, IIC0);
  err &= i2c_read8(sensor->address, 0x91, &sensor->stop_variable, IIC0);
  err &= i2c_write8(sensor->address, 0x00, 0x01, IIC0);
  err &= i2c_write8(sensor->address, 0xFF, 0x00, IIC0);
  err &= i2c_write8(sensor->address, 0x80, 0x00, IIC0);

  return err;
}

bool load_default_tuning_settings(vl53l0x_t *sensor);

bool ping_sensor(vl53l0x_t *sensor) {
  uint8_t id;
  if (i2c_read8(sensor->address, VL53L0X_IDENTIFICATION_MODEL_ID, &id, IIC0)) {
    ERROR();
    return 1;
  }
  return !(id == VL53L0X_EXPECTED_DEVICE_ID);
}

bool vl53l0x_read_default_regs(vl53l0x_t *sensor) {
  uint8_t a;
  uint16_t b;

  i2c_read8(sensor->address, 0xC0, &a, IIC0);
  LOG("0xC0: %02x\n", a);
  i2c_read8(sensor->address, 0xC1, &a, IIC0);
  LOG("0xC1: %02x\n", a);
  i2c_read8(sensor->address, 0xC2, &a, IIC0);
  LOG("0xC2: %02x\n", a);
  i2c_read16_inv(sensor->address, 0x51, &b, IIC0);
  LOG("0x51: %04x\n", b);
  i2c_read16_inv(sensor->address, 0x61, &b, IIC0);
  LOG("0x61: %04x\n", b);
  return 0;
}

bool configure_interrupt(vl53l0x_t *sensor) {
  /* Interrupt on new sample ready */
  if (i2c_write8(sensor->address, VL53L0X_SYSTEM_INTERRUPT_CONFIG_GPIO, 0x04, IIC0)) {
    ERROR();
    return 1;
  }
  /* Configure active low since the pin is pulled-up on most breakout boards */
  uint8_t gpio_hv_mux_active_high = 0;
  if (i2c_read8(sensor->address, VL53L0X_GPIO_HV_MUX_ACTIVE_HIGH, &gpio_hv_mux_active_high, IIC0)) {
    ERROR();
    return 1;
  }
  gpio_hv_mux_active_high &= ~0x10;
  if (i2c_write8(sensor->address, VL53L0X_GPIO_HV_MUX_ACTIVE_HIGH, gpio_hv_mux_active_high, IIC0)) {
    ERROR();
    return 1;
  }

  if (i2c_write8(sensor->address, VL53L0X_SYSTEM_INTERRUPT_CLEAR, 0x01, IIC0)) {
    ERROR();
    return 1;
  }
  return 0;
}

bool set_sequence_steps_enabled(vl53l0x_t *sensor, uint8_t sequence_step) {
  return i2c_write8(sensor->address, VL53L0X_SYSTEM_SEQUENCE_CONFIG, sequence_step, IIC0);
}

bool perform_single_ref_calibration(vl53l0x_t *sensor, calibration_type_t calib_type) {
  uint8_t sysrange_start = 0;
  uint8_t sequance_config = 0;

  switch (calib_type) {
    case CALIBRATION_TYPE_VHV:
      sequance_config = 0x01;
      sysrange_start = 0x01 | 0x40;
      break;
    case CALIBRATION_TYPE_PHASE:
      sequance_config = 0x02;
      sysrange_start = 0x01 | 0x00;
      break;
  }

  if (i2c_write8(sensor->address, VL53L0X_SYSTEM_SEQUENCE_CONFIG, sequance_config, IIC0)) {
    ERROR();
    return 1;
  }
  if (i2c_write8(sensor->address, VL53L0X_SYSRANGE_START, sysrange_start, IIC0)) {
    ERROR();
    return 1;
  }

  uint8_t interrupt_status = 0;
  bool err = 0;
  do {
    err = i2c_read8(sensor->address, VL53L0X_RESULT_INTERRUPT_STATUS, &interrupt_status, IIC0);
    sleep_msec(30);
  } while (!err && ((interrupt_status & 0x07) == 0));
  if (err) {
    ERROR();
    return 1;
  }

  if (i2c_write8(sensor->address, VL53L0X_SYSTEM_INTERRUPT_CLEAR, 0x01, IIC0)) {
    ERROR();
    return 1;
  }

  if (i2c_write8(sensor->address, VL53L0X_SYSRANGE_START, 0x00, IIC0)) {
    ERROR();
    return 1;
  }

  return 0;
}

bool perform_ref_calibration(vl53l0x_t *sensor) {
  if (perform_single_ref_calibration(sensor, CALIBRATION_TYPE_VHV)) {
    ERROR();
    return 1;
  }
  printf("Calibrated CALIBRATION_TYPE_VHV\n");
  if (perform_single_ref_calibration(sensor, CALIBRATION_TYPE_PHASE)) {
    ERROR();
    return 1;
  }
  printf("Calibrated CALIBRATION_TYPE_PHASE\n");
  if (set_sequence_steps_enabled(sensor, VL53L0X_RANGE_SEQUENCE_STEP_DSS + VL53L0X_RANGE_SEQUENCE_STEP_PRE_RANGE +
                                             VL53L0X_RANGE_SEQUENCE_STEP_FINAL_RANGE)) {
    ERROR();
    return 1;
  }
  printf("Sequence steps enabled\n");
  return 0;
}

bool static_init(vl53l0x_t *sensor) {
  if (load_default_tuning_settings(sensor)) {
    ERROR();
    return 1;
  }
  if (configure_interrupt(sensor)) {
    ERROR();
    return 1;
  }
  if (set_sequence_steps_enabled(sensor, VL53L0X_RANGE_SEQUENCE_STEP_DSS + VL53L0X_RANGE_SEQUENCE_STEP_PRE_RANGE +
                                             VL53L0X_RANGE_SEQUENCE_STEP_FINAL_RANGE)) {
    ERROR();
    return 1;
  }
  return 0;
}

vl53l0x_t *vl53l0x_init(void) {
  vl53l0x_t *sensor = malloc(sizeof(*sensor));
  sensor->address = VL53L0X_DEFAULT_ADDRESS;

  if (ping_sensor(sensor)) {
    ERROR();
    goto borked;
  }
  LOG("Device connected(1/4)\n");

  if (data_init(sensor)) {
    ERROR();
    goto borked;
  }
  LOG("Data initialisesd(2/4)\n");
  if (static_init(sensor)) {
    ERROR();
    goto borked;
  }
  LOG("Static init done(3/4)\n");
  if (perform_ref_calibration(sensor)) {
    ERROR();
    goto borked;
  }
  LOG("Calibration done(4/4)\n");

  return sensor;
borked:
  free(sensor);
  return NULL;
}

bool load_default_tuning_settings(vl53l0x_t *sensor) {
  bool err = false;
  err &= i2c_write8(sensor->address, 0xFF, 0x01, IIC0);
  err &= i2c_write8(sensor->address, 0x00, 0x00, IIC0);
  err &= i2c_write8(sensor->address, 0xFF, 0x00, IIC0);
  err &= i2c_write8(sensor->address, 0x09, 0x00, IIC0);
  err &= i2c_write8(sensor->address, 0x10, 0x00, IIC0);
  err &= i2c_write8(sensor->address, 0x11, 0x00, IIC0);
  err &= i2c_write8(sensor->address, 0x24, 0x01, IIC0);
  err &= i2c_write8(sensor->address, 0x25, 0xFF, IIC0);
  err &= i2c_write8(sensor->address, 0x75, 0x00, IIC0);
  err &= i2c_write8(sensor->address, 0xFF, 0x01, IIC0);
  err &= i2c_write8(sensor->address, 0x4E, 0x2C, IIC0);
  err &= i2c_write8(sensor->address, 0x48, 0x00, IIC0);
  err &= i2c_write8(sensor->address, 0x30, 0x20, IIC0);
  err &= i2c_write8(sensor->address, 0xFF, 0x00, IIC0);
  err &= i2c_write8(sensor->address, 0x30, 0x09, IIC0);
  err &= i2c_write8(sensor->address, 0x54, 0x00, IIC0);
  err &= i2c_write8(sensor->address, 0x31, 0x04, IIC0);
  err &= i2c_write8(sensor->address, 0x32, 0x03, IIC0);
  err &= i2c_write8(sensor->address, 0x40, 0x83, IIC0);
  err &= i2c_write8(sensor->address, 0x46, 0x25, IIC0);
  err &= i2c_write8(sensor->address, 0x60, 0x00, IIC0);
  err &= i2c_write8(sensor->address, 0x27, 0x00, IIC0);
  err &= i2c_write8(sensor->address, 0x50, 0x06, IIC0);
  err &= i2c_write8(sensor->address, 0x51, 0x00, IIC0);
  err &= i2c_write8(sensor->address, 0x52, 0x96, IIC0);
  err &= i2c_write8(sensor->address, 0x56, 0x08, IIC0);
  err &= i2c_write8(sensor->address, 0x57, 0x30, IIC0);
  err &= i2c_write8(sensor->address, 0x61, 0x00, IIC0);
  err &= i2c_write8(sensor->address, 0x62, 0x00, IIC0);
  err &= i2c_write8(sensor->address, 0x64, 0x00, IIC0);
  err &= i2c_write8(sensor->address, 0x65, 0x00, IIC0);
  err &= i2c_write8(sensor->address, 0x66, 0xA0, IIC0);
  err &= i2c_write8(sensor->address, 0xFF, 0x01, IIC0);
  err &= i2c_write8(sensor->address, 0x22, 0x32, IIC0);
  err &= i2c_write8(sensor->address, 0x47, 0x14, IIC0);
  err &= i2c_write8(sensor->address, 0x49, 0xFF, IIC0);
  err &= i2c_write8(sensor->address, 0x4A, 0x00, IIC0);
  err &= i2c_write8(sensor->address, 0xFF, 0x00, IIC0);
  err &= i2c_write8(sensor->address, 0x7A, 0x0A, IIC0);
  err &= i2c_write8(sensor->address, 0x7B, 0x00, IIC0);
  err &= i2c_write8(sensor->address, 0x78, 0x21, IIC0);
  err &= i2c_write8(sensor->address, 0xFF, 0x01, IIC0);
  err &= i2c_write8(sensor->address, 0x23, 0x34, IIC0);
  err &= i2c_write8(sensor->address, 0x42, 0x00, IIC0);
  err &= i2c_write8(sensor->address, 0x44, 0xFF, IIC0);
  err &= i2c_write8(sensor->address, 0x45, 0x26, IIC0);
  err &= i2c_write8(sensor->address, 0x46, 0x05, IIC0);
  err &= i2c_write8(sensor->address, 0x40, 0x40, IIC0);
  err &= i2c_write8(sensor->address, 0x0E, 0x06, IIC0);
  err &= i2c_write8(sensor->address, 0x20, 0x1A, IIC0);
  err &= i2c_write8(sensor->address, 0x43, 0x40, IIC0);
  err &= i2c_write8(sensor->address, 0xFF, 0x00, IIC0);
  err &= i2c_write8(sensor->address, 0x34, 0x03, IIC0);
  err &= i2c_write8(sensor->address, 0x35, 0x44, IIC0);
  err &= i2c_write8(sensor->address, 0xFF, 0x01, IIC0);
  err &= i2c_write8(sensor->address, 0x31, 0x04, IIC0);
  err &= i2c_write8(sensor->address, 0x4B, 0x09, IIC0);
  err &= i2c_write8(sensor->address, 0x4C, 0x05, IIC0);
  err &= i2c_write8(sensor->address, 0x4D, 0x04, IIC0);
  err &= i2c_write8(sensor->address, 0xFF, 0x00, IIC0);
  err &= i2c_write8(sensor->address, 0x44, 0x00, IIC0);
  err &= i2c_write8(sensor->address, 0x45, 0x20, IIC0);
  err &= i2c_write8(sensor->address, 0x47, 0x08, IIC0);
  err &= i2c_write8(sensor->address, 0x48, 0x28, IIC0);
  err &= i2c_write8(sensor->address, 0x67, 0x00, IIC0);
  err &= i2c_write8(sensor->address, 0x70, 0x04, IIC0);
  err &= i2c_write8(sensor->address, 0x71, 0x01, IIC0);
  err &= i2c_write8(sensor->address, 0x72, 0xFE, IIC0);
  err &= i2c_write8(sensor->address, 0x76, 0x00, IIC0);
  err &= i2c_write8(sensor->address, 0x77, 0x00, IIC0);
  err &= i2c_write8(sensor->address, 0xFF, 0x01, IIC0);
  err &= i2c_write8(sensor->address, 0x0D, 0x01, IIC0);
  err &= i2c_write8(sensor->address, 0xFF, 0x00, IIC0);
  err &= i2c_write8(sensor->address, 0x80, 0x01, IIC0);
  err &= i2c_write8(sensor->address, 0x01, 0xF8, IIC0);
  err &= i2c_write8(sensor->address, 0xFF, 0x01, IIC0);
  err &= i2c_write8(sensor->address, 0x8E, 0x01, IIC0);
  err &= i2c_write8(sensor->address, 0x00, 0x01, IIC0);
  err &= i2c_write8(sensor->address, 0xFF, 0x00, IIC0);
  err &= i2c_write8(sensor->address, 0x80, 0x00, IIC0);
  return err;
}

bool vl53l0x_read_range(vl53l0x_t *sensor) {
  bool err = i2c_write8(sensor->address, 0x80, 0x01, IIC0);
  err &= i2c_write8(sensor->address, 0xFF, 0x01, IIC0);
  err &= i2c_write8(sensor->address, 0x00, 0x00, IIC0);
  err &= i2c_write8(sensor->address, 0x91, sensor->stop_variable, IIC0);
  err &= i2c_write8(sensor->address, 0x00, 0x01, IIC0);
  err &= i2c_write8(sensor->address, 0xFF, 0x00, IIC0);
  err &= i2c_write8(sensor->address, 0x80, 0x00, IIC0);
  if (err) {
    return 1;
  }
  if (i2c_write8(sensor->address, VL53L0X_SYSRANGE_START, 0x01, IIC0)) {
    return 1;
  }
  uint8_t sysrange_start = 0;
  do {
    err = i2c_read8(sensor->address, VL53L0X_SYSRANGE_START, &sysrange_start, IIC0);
    sleep_msec(30);
  } while (!err && (sysrange_start & 0x01));
  if (err) {
    return 1;
  }

  if (i2c_read16_inv(sensor->address, VL53L0X_RESULT_RANGE_STATUS + 10, &sensor->range, IIC0)) {
    return 1;
  }

  if (i2c_write8(sensor->address, VL53L0X_SYSTEM_INTERRUPT_CLEAR, 0x01, IIC0)) {
    return 1;
  }
  if (sensor->range >= 8190) {
    sensor->range = VL53L0X_OUT_OF_RANGE;
  }
  return err;
}
bool vl53l0x_change_address(vl53l0x_t *sensor, uint8_t new_address) {
  if (i2c_write8(sensor->address, VL53L0X_SLAVE_DEVICE_ADDRESS, new_address, IIC0)) {
    return true;
  }
  sensor->address = new_address;
  return false;
}

void vl53l0x_destroy(vl53l0x_t *sensor) {
  if (sensor != NULL) {
    free(sensor);
  }
}
void vl53l0x_read_mean_range (vl53l0x_t *sensor, uint16_t *range){
  int total = 0;
  for (int i = 0; i < VL53L0X_READING_COUNT; i++){
    if (vl53l0x_read_range(sensor)) {
	    ERROR();
    }
    total += sensor->range;
    sleep_msec(75);
  }
  *range = total / VL53L0X_READING_COUNT;
}
