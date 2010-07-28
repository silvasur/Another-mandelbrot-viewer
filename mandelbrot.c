#include "common_types.h"
#include "graymap.h"
#include "graymap_alleg.h"
#include "paledit.h"
#include <allegro.h>
#include <math.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

void blur(BITMAP* src, BITMAP* dst)
{
	int lim_w = src->w > dst->w ? src->w : dst->w;
	int lim_h = src->h > dst->h ? src->h : dst->h;
	int x,y;
	
	#pragma omp parallel
	{
		#pragma omp for collapse(2)
		for(y = 1; y < lim_h-1; ++y)
			for(x = 1; x < lim_w-1; ++x)
			{
				int r,g,b,xr,yr,n;
				r = g = b = 0;
				n = 0;
				for(xr = x-1; xr < x+2; ++xr)
				{
					for(yr = y-1; yr < y+2; ++yr)
					{
						if((xr >= 0) && (xr < lim_w) && (yr >= 0) &&
						 (yr < lim_h))
						{
							++n;
							r += getr(getpixel(src,xr,yr));
							g += getg(getpixel(src,xr,yr));
							b += getb(getpixel(src,xr,yr));
						}
					}
				}
				if(n != 0)
				{
					r /= n;
					g /= n;
					b /= n;
				}
				putpixel(dst,x,y,makeacol(r,g,b,255));
			}
	}
}

double mandel_iter(double cx, double cy, double bailout, double max_iter)
{
	double x, y, xt;
	int iter;
	x = y = 0;
	iter = 0;
	while((x*x+y*y < bailout*bailout) && (iter < max_iter))
	{
		xt = x*x-y*y+cx;
		y = 2.0*x*y+cy;
		x = xt;
		iter++;
	}
	return (double) iter + (double) ((log(2*log(bailout)) - log(log(x*x+y*y))) / log(2));
}

void mandelbrot(graymap_t* gm, double x1,double y1, double x2, double y2,
 int max_iter)
{
	double bailout = 1000.0;
	double stepx = (x2-x1)/gm->w;
	double stepy = (y2-y1)/gm->h;
	
	int px, py;
	
	#pragma omp parallel
	{
		#pragma omp for collapse(2)
		for(px = 0; px < gm->w ;px++)
			for(py = 0; py < gm->h; py++)
				GM_PIX(*gm,px,py) = mandel_iter(x1+(px*stepx),y1+(py*stepy),
				 bailout, max_iter);
	}
	int i;
	static double max_gray = 0.0;
	if(max_gray == 0.0)
		for(i = 0; i < (gm->w * gm->h); ++i)
			max_gray = gm->data[i] > max_gray ? gm->data[i] : max_gray;
	#pragma omp parallel
	{
		#pragma omp for
		for(i = 0; i < (gm->w * gm->h); ++i)
			gm->data[i] /= max_gray;
	}
}

double dabs(double in)
{
	return in < 0 ? in * -1.0 : in;
}

void fix_rect(double* x1, double* y1, double* x2, double* y2, double asp_ratio)
{
	double swap = .0;
	if(*x1 > *x2)
	{
		swap = *x1;
		*x1 = *x2;
		*x2 = swap;
	}
	if(*y1 > *y2)
	{
		swap = *y1;
		*y1 = *y2;
		*y2 = swap;
	}
	
	double dx = *x2 - *x1;
	double dy = *y2 - *y1;
	double new_dx;
	if(dx/dy != asp_ratio)
	{
		new_dx = dy * asp_ratio;
		*x1 += (dx - new_dx) / 2;
		*x2 -= (dx - new_dx) / 2;
	}
}

void introtext(BITMAP* canvas)
{
	textprintf_centre_ex(canvas, font, (int) canvas->w/2, 10, 0xffffffff, -1,
	 "simple mandelbrot viewer by");
	textprintf_centre_ex(canvas, font, (int) canvas->w/2, 22, 0xffffffff, -1,
	 "Kevin Chabowski (kevin@kch42.de)");
	textprintf_centre_ex(canvas, font, (int) canvas->w/2, 34, 0xffffffff, -1,
	 "v 0.5");
	textprintf_centre_ex(canvas, font, (int) canvas->w/2, 46, 0xffffffff, -1,
	 "This program may be redistributed under the conditions");
	textprintf_centre_ex(canvas, font, (int) canvas->w/2, 58, 0xffffffff, -1,
	 "of the MIT-License. See LICENSE for more details.");
	
	textprintf_centre_ex(canvas, font, (int) canvas->w/2, 82, 0xffffffff, -1,
	 "Instructions:");
	textprintf_centre_ex(canvas, font, (int) canvas->w/2, 94, 0xffffffff, -1,
	 "Draw a border with the mouse to zoom in.");
	textprintf_centre_ex(canvas, font, (int) canvas->w/2, 106, 0xffffffff, -1,
	 "Hold right mouse button and move mouse wheel to zoom in/out.");
	textprintf_centre_ex(canvas, font, (int) canvas->w/2, 118, 0xffffffff, -1,
	 "Drag image with middle mouse button to move section.");
	textprintf_centre_ex(canvas, font, (int) canvas->w/2, 130, 0xffffffff, -1,
	 "[P] opens the palette editor.");
	textprintf_centre_ex(canvas, font, (int) canvas->w/2, 142, 0xffffffff, -1,
	 "[ESC] to quit.");
	
	textprintf_centre_ex(canvas, font, (int) canvas->w/2, 166, 0xffffffff, -1,
	 "Click to start.");
}

/* Speed control thread */
sem_t timer_sem;
BOOL speed_control;
void ticker()
{
	if(speed_control)
		sem_post(&timer_sem);
}
END_OF_FUNCTION(ticker)


int main()
{
	/* init allegro */
	allegro_init();
	install_keyboard();
	install_mouse();
	install_timer();
	set_color_depth(32);
	
	set_config_file("mandelbrot.cfg");
	int scr_return;
	if(get_config_int("mandelbrot", "fullscreen", 0) != 0)
		scr_return = set_gfx_mode(GFX_AUTODETECT,
		 get_config_int("mandelbrot", "width", 800),
		 get_config_int("mandelbrot", "height", 600), 0, 0);
	else
		scr_return = set_gfx_mode(GFX_AUTODETECT_WINDOWED,
		 get_config_int("mandelbrot", "width", 800),
		 get_config_int("mandelbrot", "height", 600), 0, 0);
	if(scr_return != 0)
	{
		set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
		allegro_message("Unable to set any graphic mode\n%s\n",
		 allegro_error);
		return 1;
	}
	set_alpha_blender();
	drawing_mode(DRAW_MODE_TRANS, NULL, 0, 0);
	BITMAP* cursor;
	BITMAP* checkered;
	BITMAP* buffer;
	BITMAP* blurbuf;
	BITMAP* mandel;
	cursor = create_bitmap(21, 21);
	checkered = create_bitmap(SCREEN_W, SCREEN_H);
	buffer = create_bitmap(SCREEN_W, SCREEN_H);
	blurbuf = create_bitmap(SCREEN_W, SCREEN_H);
	mandel = create_bitmap(SCREEN_W, SCREEN_H);
	
	/* Prepare cursor */
	cursor = load_tga("cursor.tga", NULL);
	
	/* Prepare checkered background pattern */
	int x,y;
	long color = 0;
	for(y = 0; y < (int) floor(SCREEN_H / 20); ++y)
	{
		for(x = 0; x < (int) floor(SCREEN_W / 20); ++x)
		{
			if(x%2)
				color = (y%2) ? 0xff707070 : 0xff909090;
			else
				color = (y%2) ? 0xff909090 : 0xff707070;
			rectfill(checkered, x*20, y*20, (x+1)*20, (y+1)*20, color);
		}
	}
	
	double x1,x2,y1,y2, dx, dy;
	double dppx = 0;
	int max_iter = get_config_int("mandelbrot", "max_iter", 250);
	x1 = -2.5;
	y1 = -1.5;
	x2 = 1.0;
	y2 = 1.5;
	
	BOOL update_fract = TRUE;
	BOOL screen_update = FALSE;
	BOOL render_graymap = FALSE;
	
	int r_x1 = -1;
	int r_y1 = 0;
	int r_x2 = 0;
	int r_y2 = 0;
	BOOL old_mousebtn = FALSE;
	BOOL new_mousebtn = FALSE;
	int mmx, mmy;
	
	int draw_offset_x = 0;
	int draw_offset_y = 0;
	double draw_zoom = 1.0;
	
	BOOL drag = FALSE;
	
	graymap_t my_map = create_graymap(SCREEN_W, SCREEN_H);
	
	/* GUI setup */
	gui_fg_color = 0xff000000;
	gui_bg_color = 0xffffffff;
	
	init_paledit(SCREEN_W, SCREEN_H);
	paledit_data.pal_cols[0] = mkcol(.0, .0, .0, 1.0);
	paledit_data.pal_cols[1] = mkcol(.75, .0, .0, 1.0);
	paledit_data.pal_cols[2] = mkcol(1.0, 1.0, .0, 1.0);
	paledit_data.pal_cols[3] = mkcol(.0, 0.5, 1.0, 1.0);
	paledit_data.pal_cols[4] = mkcol(1.0, 1.0, 1.0, 1.0);
	paledit_data.pal_cols[5] = mkcol(.0, .0, .0, 1.0);
	paledit_data.pal_grays[0] = 0.0;
	paledit_data.pal_grays[1] = .05;
	paledit_data.pal_grays[2] = .25;
	paledit_data.pal_grays[3] = .5;
	paledit_data.pal_grays[4] = 0.9999;
	paledit_data.pal_grays[5] = 1.0;
	paledit_data.n = 6;
	load_palette_cc(get_config_string("mandelbrot", "palette", "default"));
	
	/* Introtext */
	introtext(screen);
	while(!(mouse_b & 0x1))
		rest(1);
	
	/* init speed control */
	LOCK_FUNCTION(ticker);
	install_int_ex(ticker, BPS_TO_TIMER(60));
	speed_control = TRUE;
	
	while(!key[KEY_ESC])
	{
		mainloop:
		sem_wait(&timer_sem);
		if(update_fract)
		{
			fix_rect(&x1, &y1, &x2, &y2, ((double)SCREEN_W / (double)SCREEN_H));
			dppx = (x2-x1) / SCREEN_W;
			mandelbrot(&my_map, x1, y1, x2, y2, max_iter);
			clear_to_color(mandel, 0xff000000);
			draw_offset_x = draw_offset_y = 0;
			draw_zoom = 1.0;
			update_fract = FALSE;
			screen_update = TRUE;
			render_graymap = TRUE;
		}
		
		new_mousebtn = mouse_b;
		if(new_mousebtn != old_mousebtn)
		{
			old_mousebtn = new_mousebtn;
			screen_update = TRUE;
		}
		get_mouse_mickeys(&mmx, &mmy);
		if((mmx != 0) || (mmy != 0) || (screen_update) || (mouse_z != 0))
		{
			screen_update = TRUE;
			if(new_mousebtn & 0x1)
			{
				if(r_x1 == -1)
				{
					r_x1 = r_x2 = mouse_x;
					r_y1 = r_y2 = mouse_y;
				}
				else
				{
					r_x2 = mouse_x;
					r_y2 = mouse_y;
				}
			}
			else if(r_x1 != -1)
			{
				if((r_x1 != r_x2) && (r_y1 != r_y2))
				{
					dx = x2 - x1;
					dy = y2 - y1;
					x2 = x1 + (r_x2 * (dx/SCREEN_W));
					y2 = y1 + (r_y2 * (dy/SCREEN_H));
					x1 = x1 + (r_x1 * (dx/SCREEN_W));
					y1 = y1 + (r_y1 * (dy/SCREEN_H));
					screen_update = update_fract = TRUE;
				}
				r_x1 = -1; /* Disable zoom mode */
			}
			if(new_mousebtn & 0x2)
			{
				if(mouse_z < 0)
					draw_zoom *= 0.75;
				else if(mouse_z > 0)
					draw_zoom /= 0.75;
				if(mouse_z != 0)
				{
					draw_offset_x = (int) round((SCREEN_W - (SCREEN_W *
					 draw_zoom))/2);
					draw_offset_y = (int) round((SCREEN_H - (SCREEN_H *
					 draw_zoom))/2);
					screen_update = TRUE;
				}
			}
			else if(draw_zoom != 1.0)
			{
				draw_zoom = 1.0 / draw_zoom;
				dx = x2 - x1;
				dy = y2 - y1;
				x1 -= ((dx*draw_zoom) - dx)/2;
				x2 += ((dx*draw_zoom) - dx)/2;
				y1 -= ((dy*draw_zoom) - dy)/2;
				y2 += ((dy*draw_zoom) - dy)/2;
				draw_zoom = 1.0 / draw_zoom;
				screen_update = update_fract = TRUE;
			}
			if(new_mousebtn & 0x4)
			{
				drag = TRUE;
				draw_offset_x += mmx;
				draw_offset_y += mmy;
				screen_update = TRUE;
			}
			else if(drag)
			{
				drag = FALSE;
				x1 -= draw_offset_x * dppx;
				x2 -= draw_offset_x * dppx;
				y1 -= draw_offset_y * dppx;
				y2 -= draw_offset_y * dppx;
				screen_update = update_fract = TRUE;
			}
		}
		
		if(key[KEY_P])
		{
			speed_control = FALSE;
			blur(buffer, blurbuf);
			blit(blurbuf, screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);
			exec_paledit();
			render_graymap = TRUE;
			speed_control = TRUE;
		}
		
		position_mouse_z(0);
		
		if(screen_update)
		{
			/* Render graymap */
			if(render_graymap)
			{
				render_graymap_alleg(mandel, &my_map, paledit_data.pal_cols,
				 paledit_data.pal_grays, paledit_data.n);
				render_graymap = FALSE;
			}
			
			blit(checkered, buffer, 0, 0, 0, 0, SCREEN_W, SCREEN_H);
			if((draw_offset_x != 0) || (draw_offset_y != 0) ||
			 (draw_zoom != 1.0))
				stretch_blit(mandel, buffer, 0, 0, mandel->w, mandel->h,
				 draw_offset_x, draw_offset_y,
				 (int) round(mandel->w * draw_zoom),
				 (int) round(mandel->h * draw_zoom));
			else
				blit(mandel, buffer, 0, 0, 0, 0, SCREEN_W, SCREEN_H);
			if(r_x1 != -1)
			{
				rectfill(buffer, r_x1, r_y1, r_x2, r_y2, 0x80ffffff);
				rect(buffer, r_x1, r_y1, r_x2, r_y2, 0xffffffff);
			}
			/* Draw cursor */
			draw_trans_sprite(buffer, cursor, mouse_x-10, mouse_y-10);
			
			/* Infobar */
			rectfill(buffer, 0, SCREEN_H - 14, SCREEN_W, SCREEN_H,
			 0x90000000);
			textprintf_ex(buffer, font, 2, SCREEN_H - 12, 0xffffffff, -1,
			 "X = %lf   Y = %lf", x1 + mouse_x * dppx, y1 + mouse_y * dppx);
			 textprintf_right_ex(buffer, font, SCREEN_W - 2, SCREEN_H - 12,
			  0xffffffff, -1, "(c) 2010 by Kevin Chabowski");
			
			if(update_fract)
				textprintf_centre_ex(buffer, font, (int)(SCREEN_W/2),
				 (int)(SCREEN_H/2), 0xffffffff, makecol(0,0,0),
				 "Rerendering. Please wait...");
			
			blit(buffer, screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);
			screen_update = FALSE;
		}
	}
	blur(buffer, blurbuf);
	blit(blurbuf, screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);
	if(alert("", "Do you really want to quit?", "", "&Yes, get out of here!",
	 "&No, I want to stay", 'y', 'n') == 2)
	{
		screen_update = TRUE;
		goto mainloop;
	}
	
	/* Tidy up */
	destroy_paledit();
	destroy_graymap(&my_map);
	destroy_bitmap(cursor);
	destroy_bitmap(blurbuf);
	destroy_bitmap(checkered);
	destroy_bitmap(buffer);
	destroy_bitmap(mandel);
	allegro_exit();
	return 0;
}
END_OF_MAIN()
