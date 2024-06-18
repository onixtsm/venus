#ifndef VL53L0X_H_
#define VL53L0X_H_
#include <stdbool.h>
#include <stdint.h>

#define VL53L0X_IDENTIFICATION_MODEL_ID (0xC0)
#define VL53L0X_VHV_CONFIG_PAD_SCL_SDA_EXTSUP_HV (0x89) 
#define VL53L0X_MSRC_CONFIG_CONTROL (0x60)
#define VL53L0X_FINAL_RANGE_CONFIG_MIN_COUNT_RATE_RTN_LIMIT (0x44)
#define VL53L0X_SYSTEM_SEQUENCE_CONFIG (0x01)
#define VL53L0X_DYNAMIC_SPAD_REF_EN_START_OFFSET (0x4F)
#define VL53L0X_DYNAMIC_SPAD_NUM_REQUESTED_REF_SPAD (0x4E)
#define VL53L0X_GLOBAL_CONFIG_REF_EN_START_SELECT (0xB6)
#define VL53L0X_SYSTEM_INTERRUPT_CONFIG_GPIO (0x0A)
#define VL53L0X_GPIO_HV_MUX_ACTIVE_HIGH (0x84)
#define VL53L0X_SYSTEM_INTERRUPT_CLEAR (0x0B)
#define VL53L0X_RESULT_INTERRUPT_STATUS (0x13)
#define VL53L0X_SYSRANGE_START (0x00)
#define VL53L0X_GLOBAL_CONFIG_SPAD_ENABLES_REF_0 (0xB0)
#define VL53L0X_RESULT_RANGE_STATUS (0x14)
#define VL53L0X_SLAVE_DEVICE_ADDRESS (0x8A)

#define VL53L0X_RANGE_SEQUENCE_STEP_TCC (0x10) /* Target CentreCheck */
#define VL53L0X_RANGE_SEQUENCE_STEP_MSRC (0x04) /* Minimum Signal Rate Check */
#define VL53L0X_RANGE_SEQUENCE_STEP_DSS (0x28) /* Dynamic SPAD selection */
#define VL53L0X_RANGE_SEQUENCE_STEP_PRE_RANGE (0x40)
#define VL53L0X_RANGE_SEQUENCE_STEP_FINAL_RANGE (0x80)

#define VL53L0X_EXPECTED_DEVICE_ID (0xEE)
#define VL53L0X_DEFAULT_ADDRESS (0x29)

/* There are two types of SPAD: aperture and non-aperture. My understanding
 * is that aperture ones let it less light (they have a smaller opening), similar
 * to how you can change the aperture on a digital camera. Only 1/4 th of the
 * SPADs are of type non-aperture. */
#define VL53L0X_SPAD_TYPE_APERTURE (0x01)
/* The total SPAD array is 16x16, but we can only activate a quadrant spanning 44 SPADs at
 * a time. In the ST api code they have (for some reason) selected 0xB4 (180) as a starting
 * point (lies in the middle and spans non-aperture (3rd) quadrant and aperture (4th) quadrant). */
#define VL53L0X_SPAD_START_SELECT (0xB4)
/* The total SPAD map is 16x16, but we should only activate an area of 44 SPADs at a time. */
#define VL53L0X_SPAD_MAX_COUNT (44)
/* The 44 SPADs are represented as 6 bytes where each bit represents a single SPAD.
 * 6x8 = 48, so the last four bits are unused. */
#define VL53L0X_SPAD_MAP_ROW_COUNT (6)
#define VL53L0X_SPAD_ROW_SIZE (8)
/* Since we start at 0xB4 (180), there are four quadrants (three aperture, one aperture),
 * and each quadrant contains 256 / 4 = 64 SPADs, and the third quadrant is non-aperture, the
 * offset to the aperture quadrant is (256 - 64 - 180) = 12 */
#define VL53L0X_SPAD_APERTURE_START_INDEX (12)

#define VL53L0X_OUT_OF_RANGE (8190)


typedef struct {
  uint8_t address;
  uint16_t range;
  uint8_t stop_variable;
} vl53l0x_t;

vl53l0x_t *vl53l0x_init(void);
bool vl53l0x_read_range(vl53l0x_t *sensor);
bool vl53l0x_read_default_regs(vl53l0x_t *sensor);
bool vl53l0x_change_address(vl53l0x_t *sensor, uint8_t new_address);
void vl53l0x_destroy(vl53l0x_t *sensor);


#endif
