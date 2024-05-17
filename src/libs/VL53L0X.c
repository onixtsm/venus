#include "VL53L0X.h"

#include <stdbool.h>
#include <stdio.h>

#include "i2c.h"

#define ERROR() fprintf(stderr, "[ERROR] in %s and line %d\n", __func__, __LINE__);


typedef enum { CALIBRATION_TYPE_VHV, CALIBRATION_TYPE_PHASE } calibration_type_t;

const uint8_t address = VL53L0X_DEFAULT_ADDRESS;
uint8_t stop_variable = 0;

bool data_init(void) {
  bool err = 0;

  uint8_t vhv_config_scl_sda = 0;

  if (i2c_read8(address, VL53L0X_VHV_CONFIG_PAD_SCL_SDA_EXTSUP_HV, &vhv_config_scl_sda, IIC0)) {
    ERROR();
    return 1;
  }

  vhv_config_scl_sda |= 0x01;

  if (i2c_write8(address, VL53L0X_VHV_CONFIG_PAD_SCL_SDA_EXTSUP_HV, vhv_config_scl_sda, IIC0)) {
    ERROR();
    return 1;
  }

  /* standard i2c mode */
  /* magic numbers - have no clue */
  err &= i2c_write8(address, 0x88, 0x00, IIC0);
  err &= i2c_write8(address, 0x80, 0x01, IIC0);
  err &= i2c_write8(address, 0xFF, 0x01, IIC0);
  err &= i2c_write8(address, 0x00, 0x00, IIC0);
  err &= i2c_read8(address, 0x91, &stop_variable, IIC0);
  err &= i2c_write8(address, 0x00, 0x01, IIC0);
  err &= i2c_write8(address, 0xFF, 0x00, IIC0);
  err &= i2c_write8(address, 0x80, 0x00, IIC0);

  return err;
}

bool load_default_tuning_settings(void);

bool is_not_connected(void) {
  uint8_t id;
  if (i2c_read8(address, VL53L0X_IDENTIFICATION_MODEL_ID, &id, IIC0)) {
    ERROR();
    return 1;
  }
  return !(id == VL53L0X_EXPECTED_DEVICE_ID);
}

bool vl53l0x_read_default_regs(void) {
  uint8_t a;
  uint16_t b;
  
  i2c_read8(address, 0xC0, &a, IIC0);
  printf("0xC0: %02x\n", a);
  i2c_read8(address, 0xC1, &a, IIC0);
  printf("0xC1: %02x\n", a);
  i2c_read8(address, 0xC2, &a, IIC0);
  printf("0xC2: %02x\n", a);
  i2c_read16_inv(address, 0x51, &b, IIC0);
  printf("0x51: %04x\n", b);
  i2c_read16_inv(address, 0x61, &b, IIC0);
  printf("0x61: %04x\n", b);
  return 0;
}

bool configure_interrupt(void) {
  /* Interrupt on new sample ready */
  if (i2c_write8(address, VL53L0X_SYSTEM_INTERRUPT_CONFIG_GPIO, 0x04, IIC0)) {
    ERROR();
    return 1;
  }
  /* Configure active low since the pin is pulled-up on most breakout boards */
  uint8_t gpio_hv_mux_active_high = 0;
  if (i2c_read8(address, VL53L0X_GPIO_HV_MUX_ACTIVE_HIGH, &gpio_hv_mux_active_high, IIC0)) {
    ERROR();
    return 1;
  }
  gpio_hv_mux_active_high &= ~0x10;
  if (i2c_write8(address, VL53L0X_GPIO_HV_MUX_ACTIVE_HIGH, gpio_hv_mux_active_high, IIC0)) {
    ERROR();
    return 1;
  }

  if (i2c_write8(address, VL53L0X_SYSTEM_INTERRUPT_CLEAR, 0x01, IIC0)) {
    ERROR();
    return 1;
  }
  return 0;
}

bool set_sequence_steps_enabled(uint8_t sequence_step) {
  return i2c_write8(address, VL53L0X_SYSTEM_SEQUENCE_CONFIG, sequence_step, IIC0);
}

bool perform_single_ref_calibration(calibration_type_t calib_type) {
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

  if (i2c_write8(address, VL53L0X_SYSTEM_SEQUENCE_CONFIG, sequance_config, IIC0)) {
    ERROR();
    return 1;
  }
  if (i2c_write8(address, VL53L0X_SYSRANGE_START, sysrange_start, IIC0)) {
    ERROR();
    return 1;
  }

  uint8_t interrupt_status = 0;
  bool err = 0;
  do {
    err = i2c_read8(address, VL53L0X_RESULT_INTERRUPT_STATUS, &interrupt_status, IIC0);
    sleep_msec(30);
  } while (!err && ((interrupt_status & 0x07) == 0));
  if (err) {
    ERROR()
    return 1;
  }

  if (i2c_write8(address, VL53L0X_SYSTEM_INTERRUPT_CLEAR, 0x01, IIC0)) {
    ERROR();
    return 1;
  }

  if (i2c_write8(address, VL53L0X_SYSRANGE_START, 0x00, IIC0)) {
    ERROR();
    return 1;
  }

  return 0;
}

bool perform_ref_calibration(void) {
  if (perform_single_ref_calibration(CALIBRATION_TYPE_VHV)) {
    ERROR();
    return 1;
  }
  printf("Calibrated CALIBRATION_TYPE_VHV\n");
  if (perform_single_ref_calibration(CALIBRATION_TYPE_PHASE)) {
    ERROR();
    return 1;
  }
  printf("Calibrated CALIBRATION_TYPE_PHASE\n");
  if (set_sequence_steps_enabled(VL53L0X_RANGE_SEQUENCE_STEP_DSS + VL53L0X_RANGE_SEQUENCE_STEP_PRE_RANGE +
                                 VL53L0X_RANGE_SEQUENCE_STEP_FINAL_RANGE)) {
    ERROR();
    return 1;
  }
  printf("Sequence steps enabled\n");
  return 0;
}

bool static_init(void) {
  if (load_default_tuning_settings()) {
    ERROR();
    return 1;
  }
  if (configure_interrupt()) {
    ERROR();
    return 1;
  }
  if (set_sequence_steps_enabled(VL53L0X_RANGE_SEQUENCE_STEP_DSS + VL53L0X_RANGE_SEQUENCE_STEP_PRE_RANGE +
                                 VL53L0X_RANGE_SEQUENCE_STEP_FINAL_RANGE)) {
    ERROR();
    return 1;
  }
  return 0;
}

bool vl53l0x_init(void) {
  if (is_not_connected()) {
    ERROR();
    return 1;
  }
  printf("Device connected(1/4)\n");

  if (data_init()) {
    ERROR();
    return 1;
  }
  printf("Data initialisesd(2/4)\n");
  if (static_init()) {
    ERROR();
    return 1;
  }
  printf("Static init done(3/4)\n");
  if (perform_ref_calibration()) {
    ERROR();
    return 1;
  }
  printf("Calibration done(4/4)\n");

  return 0;
}

bool load_default_tuning_settings(void) {
  bool err = i2c_write8(address, 0xFF, 0x01, IIC0);
  err &= i2c_write8(address, 0x00, 0x00, IIC0);
  err &= i2c_write8(address, 0xFF, 0x00, IIC0);
  err &= i2c_write8(address, 0x09, 0x00, IIC0);
  err &= i2c_write8(address, 0x10, 0x00, IIC0);
  err &= i2c_write8(address, 0x11, 0x00, IIC0);
  err &= i2c_write8(address, 0x24, 0x01, IIC0);
  err &= i2c_write8(address, 0x25, 0xFF, IIC0);
  err &= i2c_write8(address, 0x75, 0x00, IIC0);
  err &= i2c_write8(address, 0xFF, 0x01, IIC0);
  err &= i2c_write8(address, 0x4E, 0x2C, IIC0);
  err &= i2c_write8(address, 0x48, 0x00, IIC0);
  err &= i2c_write8(address, 0x30, 0x20, IIC0);
  err &= i2c_write8(address, 0xFF, 0x00, IIC0);
  err &= i2c_write8(address, 0x30, 0x09, IIC0);
  err &= i2c_write8(address, 0x54, 0x00, IIC0);
  err &= i2c_write8(address, 0x31, 0x04, IIC0);
  err &= i2c_write8(address, 0x32, 0x03, IIC0);
  err &= i2c_write8(address, 0x40, 0x83, IIC0);
  err &= i2c_write8(address, 0x46, 0x25, IIC0);
  err &= i2c_write8(address, 0x60, 0x00, IIC0);
  err &= i2c_write8(address, 0x27, 0x00, IIC0);
  err &= i2c_write8(address, 0x50, 0x06, IIC0);
  err &= i2c_write8(address, 0x51, 0x00, IIC0);
  err &= i2c_write8(address, 0x52, 0x96, IIC0);
  err &= i2c_write8(address, 0x56, 0x08, IIC0);
  err &= i2c_write8(address, 0x57, 0x30, IIC0);
  err &= i2c_write8(address, 0x61, 0x00, IIC0);
  err &= i2c_write8(address, 0x62, 0x00, IIC0);
  err &= i2c_write8(address, 0x64, 0x00, IIC0);
  err &= i2c_write8(address, 0x65, 0x00, IIC0);
  err &= i2c_write8(address, 0x66, 0xA0, IIC0);
  err &= i2c_write8(address, 0xFF, 0x01, IIC0);
  err &= i2c_write8(address, 0x22, 0x32, IIC0);
  err &= i2c_write8(address, 0x47, 0x14, IIC0);
  err &= i2c_write8(address, 0x49, 0xFF, IIC0);
  err &= i2c_write8(address, 0x4A, 0x00, IIC0);
  err &= i2c_write8(address, 0xFF, 0x00, IIC0);
  err &= i2c_write8(address, 0x7A, 0x0A, IIC0);
  err &= i2c_write8(address, 0x7B, 0x00, IIC0);
  err &= i2c_write8(address, 0x78, 0x21, IIC0);
  err &= i2c_write8(address, 0xFF, 0x01, IIC0);
  err &= i2c_write8(address, 0x23, 0x34, IIC0);
  err &= i2c_write8(address, 0x42, 0x00, IIC0);
  err &= i2c_write8(address, 0x44, 0xFF, IIC0);
  err &= i2c_write8(address, 0x45, 0x26, IIC0);
  err &= i2c_write8(address, 0x46, 0x05, IIC0);
  err &= i2c_write8(address, 0x40, 0x40, IIC0);
  err &= i2c_write8(address, 0x0E, 0x06, IIC0);
  err &= i2c_write8(address, 0x20, 0x1A, IIC0);
  err &= i2c_write8(address, 0x43, 0x40, IIC0);
  err &= i2c_write8(address, 0xFF, 0x00, IIC0);
  err &= i2c_write8(address, 0x34, 0x03, IIC0);
  err &= i2c_write8(address, 0x35, 0x44, IIC0);
  err &= i2c_write8(address, 0xFF, 0x01, IIC0);
  err &= i2c_write8(address, 0x31, 0x04, IIC0);
  err &= i2c_write8(address, 0x4B, 0x09, IIC0);
  err &= i2c_write8(address, 0x4C, 0x05, IIC0);
  err &= i2c_write8(address, 0x4D, 0x04, IIC0);
  err &= i2c_write8(address, 0xFF, 0x00, IIC0);
  err &= i2c_write8(address, 0x44, 0x00, IIC0);
  err &= i2c_write8(address, 0x45, 0x20, IIC0);
  err &= i2c_write8(address, 0x47, 0x08, IIC0);
  err &= i2c_write8(address, 0x48, 0x28, IIC0);
  err &= i2c_write8(address, 0x67, 0x00, IIC0);
  err &= i2c_write8(address, 0x70, 0x04, IIC0);
  err &= i2c_write8(address, 0x71, 0x01, IIC0);
  err &= i2c_write8(address, 0x72, 0xFE, IIC0);
  err &= i2c_write8(address, 0x76, 0x00, IIC0);
  err &= i2c_write8(address, 0x77, 0x00, IIC0);
  err &= i2c_write8(address, 0xFF, 0x01, IIC0);
  err &= i2c_write8(address, 0x0D, 0x01, IIC0);
  err &= i2c_write8(address, 0xFF, 0x00, IIC0);
  err &= i2c_write8(address, 0x80, 0x01, IIC0);
  err &= i2c_write8(address, 0x01, 0xF8, IIC0);
  err &= i2c_write8(address, 0xFF, 0x01, IIC0);
  err &= i2c_write8(address, 0x8E, 0x01, IIC0);
  err &= i2c_write8(address, 0x00, 0x01, IIC0);
  err &= i2c_write8(address, 0xFF, 0x00, IIC0);
  err &= i2c_write8(address, 0x80, 0x00, IIC0);
  return err;
}

bool vl53l0x_read_range(uint16_t *range) {
  bool err = i2c_write8(address, 0x80, 0x01, IIC0);
  err &= i2c_write8(address, 0xFF, 0x01, IIC0);
  err &= i2c_write8(address, 0x00, 0x00, IIC0);
  err &= i2c_write8(address, 0x91, stop_variable, IIC0);
  err &= i2c_write8(address, 0x00, 0x01, IIC0);
  err &= i2c_write8(address, 0xFF, 0x00, IIC0);
  err &= i2c_write8(address, 0x80, 0x00, IIC0);
  if (err) {
    return 1;
  }
  if (i2c_write8(address, VL53L0X_SYSRANGE_START, 0x01, IIC0)) {
    return 1;
  }
  uint8_t sysrange_start = 0;
  do {
    err = i2c_read8(address, VL53L0X_SYSRANGE_START, &sysrange_start, IIC0);
    sleep_msec(30);
  } while (!err && (sysrange_start & 0x01));
  if (err) {
    return 1;
  }

  if (i2c_read16_inv(address, VL53L0X_RESULT_RANGE_STATUS + 10, range, IIC0)) {
    return 1;
  }

  if (i2c_write8(address, VL53L0X_SYSTEM_INTERRUPT_CLEAR, 0x01, IIC0)) {
    return 1;
  }
  if (*range >= 8190) {
    *range = VL53L0X_OUT_OF_RANGE;
  }
  return err;
}
