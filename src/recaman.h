#ifndef RECAMAN_H
#define RECAMAN_H

#include <inttypes.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "raylib.h"

typedef uint8_t u8;
typedef uint64_t u64;

#define ST_NONE ((u64)(-1))

typedef struct {
	u64 prev_value;
	u64 x_value;
	u64 next_value;
	float alpha;
} SequenceTerm;

typedef struct {
	// metadata
	Vector2 window_size;
	const char* window_title;
	// settings
	int theme; // 1 = blue, 2 = monochrome
	bool draw_x_marks;
	bool show_numbers;
	bool show_fps;
	// term index range
	u64 term_start;
	u64 term_end;
	// value range on the x axis
	u64 x_axis_value_min;
	u64 x_axis_value_max;
	// screen position
	Vector2 x_axis_start_px;
	Vector2 x_axis_end_px;
	// sequence
	SequenceTerm* sequence;
	u64 seq_len;
	// display value
	float display_value_min;
	float display_value_max;
	// lerp speed
	float vel_min;
	float vel_max;
} AppData;

// main
void init();
void populate_sequence();
void compute_value_range();
void run_visualization();

// draw
void draw_x_axis();
void draw_rings();

// utils
int log10_step_size(int value);
float value_to_px(u64 value);
void debug_print_sequence();

extern AppData APP;

#endif // RECAMAN_H
