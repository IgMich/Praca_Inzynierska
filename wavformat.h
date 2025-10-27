#pragma once
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#define PRINT_ERROR(a, args...) printf("ERROR %s() %s Line %d: " a "\n", __FUNCTION__, __FILE__, __LINE__, ##args);

typedef struct {
	uint32_t samples;
	int16_t *data;
	int32_t bytes_per_second;	// sample_rate * num_channels * bits_per_sample / 8
} sound_t;

bool LoadWav(const char *filename, sound_t *sound);