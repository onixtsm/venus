#ifndef MEASUREMENTS_H
#define MEASUREMENTS_H
#include <stdint.h>
#include <stdio.h>

extern char name[10];

uint32_t get_period(uint8_t pin, uint8_t level);

double incremental_mean(double new_value, double running_mean, size_t count);

int map(int x, int in_min, int in_max, int out_min, int out_max);

#define clamp(x, min, max) (x) > (max) ? (max) : (x) < (min) ? (min) : (x)

#define MAX(a, b) (a) > (b) ? (a) : (b)
#define MIN(a, b) (a) < (b) ? (a) : (b)

#define ERROR(fmt, ...) fprintf(stderr, "[ERROR %s] in %s and line %d\n"fmt, name, __func__, __LINE__, ##__VA_ARGS__);
#define LOG(fmt, ...) fprintf(stderr, "[LOG %s]"fmt "\n", name, ##__VA_ARGS__)
#endif
