#ifndef TCS3472_H_
#define TCS3472_H_
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define TCS3472_ADDR 0x29
#define TCS3472_ID 0x12
#define TCS3472_COMMAND_BIT 0x80
#define TCS3472_ENABLE 0x00
#define TCS3472_ENABLE_PON 0x1
#define TCS3472_ENABLE_AEN 0x2
#define TCS3472_ENABLE_WEN 0x8
#define TCS3472_ENABLE_AIEN 0x10
#define TCS3472_REG_C 0x14
#define TCS3472_REG_R 0x16
#define TCS3472_REG_G 0x18
#define TCS3472_REG_B 0x1A
typedef enum { RED, GREEN, BLUE, WHITE, BLACK, COLOR_COUNT } color_t;

static const char *color_names[COLOR_COUNT+1] = {"RED", "GREEN", "BLUE", "WHITE", "BLACK", "UNKNOWN"};


typedef struct {
  uint16_t iic;
  bool enable;
  uint16_t c, r, g, b;
} tcs3472_t;

const char *COLOR_NAME(size_t index);

color_t tcs3472_determine_color(tcs3472_t* sensor);

/*
 * @brief Checks if sensor is connected
 * @return 0 if successful, 1 on error
 */
tcs3472_t *tcs3472_init(int iic);

/*
 * @brief Enables sensor
 * @return 0 if successful, 1 on error
 */
bool tcs3472_enable(tcs3472_t *sensor);
/*
 * @brief Reads sensed values
 * @param c The pointer to clear value
 * @param r The pointer to red value
 * @param g The pointer to green value
 * @param b The pointer to blue value
 *
 * @return 0 if successful, 1 on error
 */
void tcs3472_read_colors(tcs3472_t *sensor);

/*
 * @brief Disables sensor
 * @return 0 if successful, 1 on error
 */
bool tcs3472_disable(tcs3472_t *sensor);

void tcs3472_destroy(tcs3472_t *sensor);

void print_colors(tcs3472_t *sensor);

#endif
