#ifndef _graymap_h_
#define _graymap_h_

#ifdef __cplusplus
extern "C" {
#endif
/**
 * @file graymap.h
 *
 * This header files contains thedefinition of data types and functions to deal
 * with graymap images.
 */

/* ******************************** MACROS ************************************/
/**
 * This macro makes accessing graymap pixels easier.
 * 
 * If you use pointers to graymaps, you have to call this macro like this:
 * GM_PIX(*my_graymap, x,y) = foo;
 *
 * @param gm the graymap
 * @param x X coordinate
 * @param y Y coordinate
 * @return The pixel of the graymap which can be used like a variable.
 * @see set_pix_graymap()
 * @see get_pix_graymap()
 * @see color_t
 */
#define GM_PIX(gm,x,y) (gm).data[((y)*(gm).w)+(x)]

/* ****************************** DATA TYPES **********************************/
/**
 * This is a data type to hold RGBA color values.
 *
 * All values are stored as floating point numbers.
 */
typedef struct
{
	double r; /**< Red color component. */
	double g; /**< Green color component. */
	double b; /**< Blue color component. */
	double a; /**< Alpha value component. */
} color_t;

/**
 * The data structure for graymaps.
 */
typedef struct
{
	int w; /**< Width of the image */
	int h; /**< Height of the image. */
	double* data; /**< Image data. */
} graymap_t;

/* ******************************* FUNCTIONS **********************************/
/**
 * Creating a graymap.
 * 
 * @param w the width of the new image.
 * @param h the height of the new image.
 * @return a new graymap image.
 * @see destroy_graymap()
 */
extern graymap_t create_graymap(int w, int h);

/**
 * Destroying a graymap.
 * 
 * This will free the memory of the image data and set width/height to -1.
 *
 * @param gm Pointer to the graymap.
 * @see create_graymap()
 */
extern void destroy_graymap(graymap_t* gm);

/**
 * Clear the graymap to a grayscale value.
 *
 * This will override all pixels in the graymap to the specified greyscale
 * value.
 *
 * @param gm Pointer to the graymap.
 * @param grayval Grayscale value.
 */
extern void clear_graymap(graymap_t* gm, double grayval);

/**
 * Set a pixel in a graymap.
 *
 * @param gm Pointer to the graymap
 * @param x X coordinate
 * @param y Y coordinate
 * @param gray grayscale value of the pixel
 * @see get_pix_graymap()
 * @see GM_PIX()
 */
extern void set_pix_graymap(graymap_t* gm, int x, int y, double gray);

/**
 * Get a pixel from a graymap.
 *
 * @param gm Pointer to the graymap
 * @param x X coordinate
 * @param y Y coordinate
 * @return grayscale value of the pixel
 * @see set_pix_graymap()
 * @see GM_PIX()
 */
extern double get_pix_graymap(graymap_t* gm, int x, int y);

/**
 * Blitting (copying) image data from one graymap to another.
 *
 * @param src pointer to the source graymap
 * @param dst pointer to the destination graymap
 * @param src_x Blitting start X coordinate in the source graymap
 * @param src_y Blitting start Y coordinate in the source graymap
 * @param dst_x Blitting start X coordinate in the destination graymap
 * @param dst_y Blitting start Y coordinate in the destination graymap
 * @param w Width of the blitting rectangle.
 * @param h Height of the blitting rectangle.
 */
extern void blit_graymaps(graymap_t* src, graymap_t* dst, int src_x, int src_y,
 int dst_x, int dst_y, int w, int h);

/**
 * Calculate a color from a palette and a grayscale value.
 * 
 * A palette is represented by two arrays. One array contains colors, the other
 * one contains the corresponding grayscale values. This palette now represents
 * a color gradient. The color which fits to the grayscale value will be
 * returned.
 *
 * @param grayval The grayscale value.
 * @param pal_cols Array of colors.
 * @param pal_grays Array of grayscale values
 * @param pal_n length of the arrays.
 * @return The calculated color.
 */
extern color_t get_palette_color(double grayval, color_t* pal_cols,
 double* pal_grays, int pal_n);

/**
 * Create a color_t object.
 *
 * @param r The red color component.
 * @param g The green color component.
 * @param b The blue color component.
 * @param a The Alpha value.
 * @return the color_t object.
 * @see color_t
 */
extern color_t mkcol(double r, double g, double b, double a);

#ifdef __cplusplus
}
#endif

#endif
