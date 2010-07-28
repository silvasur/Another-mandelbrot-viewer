#include "graymap.h"
#include "common_types.h"
#include <stdlib.h>

graymap_t create_graymap(int w, int h)
{
	graymap_t rv;
	rv.w = -1;
	rv.h = -1;
	rv.data = (double*) malloc(sizeof(double) * w * h);
	if(rv.data == NULL)
		return rv;
	rv.w = w;
	rv.h = h;
	return rv;
}

void destroy_graymap(graymap_t* gm)
{
	free(gm->data);
	gm->w = gm->h = -1;
}

void clear_graymap(graymap_t* gm, double grayval)
{
	int limit = gm->w * gm->h;
	int i;
	double* data_p = gm->data;
	for(i = 0; i < limit; ++i, ++data_p)
		(*data_p) = grayval;
}

void set_pix_graymap(graymap_t* gm, int x, int y, double gray)
{
	GM_PIX((*gm),x,y) = gray;
}

double get_pix_graymap(graymap_t* gm, int x, int y)
{
	return GM_PIX(*gm,x,y);
}

void blit_graymaps(graymap_t* src, graymap_t* dst, int src_x, int src_y,
 int dst_x, int dst_y, int w, int h)
{
	if(src_x + w >= src->w)
		w = src->w - src_x;
	if(src_y + h >= src->h)
		h = src->h - src_y;
	if(dst_x + w >= dst->w)
		w = dst->w - dst_x;
	if(dst_y + h >= dst->h)
		h = dst->h - dst_y;
	int x,y;
	for(y = 0; y < h; ++y)
		for(x = 0; x < w; ++x)
			GM_PIX(*dst,dst_x+x, dst_y+y) = GM_PIX(*src, src_x+x, src_y+y);
}

color_t get_palette_color(double grayval, color_t* pal_cols, double* pal_grays,
 int pal_n)
{
	color_t rv;
	rv.r = rv.g = rv.b = rv.a = .0;
	int index;
	double factor1, factor2;
	BOOL found = FALSE;
	/* find the index number */
	for(index = 0; index < ( pal_n - 1 ); ++index)
	{
		if((grayval >= pal_grays[index]) && (grayval <= pal_grays[index+1]))
		{
			found = TRUE;
			break;
		}
	}
	
	if(!found)
		return rv;
	
	/* Calculate factors */
	factor1 = (pal_grays[index+1] - grayval ) /
	 ( pal_grays[index+1] -pal_grays[index] );
	factor2 = 1.0 - factor1;
	
	/* Calculate color */
	rv.r = ( pal_cols[index].r * factor1 ) +
	 ( pal_cols[index+1].r * factor2 );
	rv.g = ( pal_cols[index].g * factor1 ) +
	 ( pal_cols[index+1].g * factor2 );
	rv.b = ( pal_cols[index].b * factor1 ) +
	 ( pal_cols[index+1].b * factor2 );
	rv.a = ( pal_cols[index].a * factor1 ) +
	 ( pal_cols[index+1].a * factor2 );
	return rv;
}

color_t mkcol(double r, double g, double b, double a)
{
	color_t rv;
	rv.r = r;
	rv.g = g;
	rv.b = b;
	rv.a = a;
	return rv;
}
