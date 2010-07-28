#ifndef _graymap_alleg_h_
#define _graymap_alleg_h_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file graymap_alleg.h
 * 
 * This header file conatins a function to render graymaps to allegro 4 bitmaps.
 */
#include <allegro.h>
#include "graymap.h"

/**
 * Render a graymap to a allegro 4 bitmap.
 *
 * A palette is represented by two arrays. One array contains colors, the other
 * one contains the corresponding grayscale values. This palette now represents
 * a color gradient. The color which fits to the grayscale value will be
 * returned.
 *
 * @param canvas The allegro 4 bitmap.
 * @param gm The graymap.
 * @param pal_cols Array of colors.
 * @param pal_grays Array of grayscale values
 * @param pal_n length of the arrays.
 */
extern void render_graymap_alleg(BITMAP* canvas, graymap_t* gm,
 color_t* pal_cols, double* pal_grays, int pal_n);

#ifdef __cplusplus
}
#endif

#endif
