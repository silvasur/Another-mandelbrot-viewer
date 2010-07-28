#include "common_types.h"
#include "graymap_alleg.h"
#include "graymap.h"
#include <math.h>
#include <allegro.h>

void render_graymap_alleg(BITMAP* canvas, graymap_t* gm, color_t* pal_cols,
 double* pal_grays, int pal_n)
{
	int limit_w = gm->w;
	int limit_h = gm->h;
	if(limit_w > canvas->w)
		limit_w = canvas->w;
	if(limit_h > canvas->h)
		limit_h = canvas->h;
	
	color_t col;
	
	int x,y,i;
	uint_8 r,g,b,a;
	for(y = 0, i = 0; y < limit_h; ++y)
	{
		for(x = 0; x < limit_w; ++x, ++i)
		{
			col = get_palette_color(gm->data[i], pal_cols, pal_grays, pal_n);
			r = (uint_8) round(col.r * 255.0);
			g = (uint_8) round(col.g * 255.0);
			b = (uint_8) round(col.b * 255.0);
			a = (uint_8) round(col.a * 255.0);
			putpixel(canvas, x,y,makeacol(r,g,b,a));
		}
	}
}
