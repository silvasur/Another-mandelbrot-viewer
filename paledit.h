#ifndef _paledit_h_
#define _paledit_h_

#include <allegro.h>
#include "common_types.h"
#include "graymap.h"

extern struct
{
	BITMAP* gradient;
	int n;
	color_t pal_cols[30];
	double pal_grays[30];
	BOOL pal_change;
	char palname_buffer[41 * 4];
} paledit_data;

extern void save_palette(char* name);

extern void load_palette(char* name);
extern void load_palette_cc(const char* name);

extern void init_paledit(int canvas_w, int canvas_h);

extern void exec_paledit();

extern void destroy_paledit();

#endif
