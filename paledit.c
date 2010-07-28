#include <allegro.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "common_types.h"
#include "graymap.h"
#include "graymap_alleg.h"

/* Set directory seperator */
#define DS '/'
#if defined(WIN32) || defined(__WIN32__)
#define DS '\\'
#endif

struct
{
	BITMAP* gradient;
	int n;
	color_t pal_cols[30];
	double pal_grays[30];
	BOOL pal_change;
	char palname_buffer[41 * 4];
} paledit_data;

void write_eightbytes(FILE* fp, uint_64 eb)
{
	int i;
	for(i = 0; i < 8; ++i)
		fputc((int)(((char*) (&eb))[i]), fp);
}

uint_64 read_eightbytes(FILE* fp)
{
	int i;
	uint_64 rv;
	for(i = 0; i < 8; ++i)
		((char*)(&rv))[i] = (char) fgetc(fp);
	return rv;
}

void save_palette(char* name)
{
	char* fullpath;
	uint_64 eb;
	int i;
	fullpath = (char*) malloc(sizeof(char) * (strlen(name) + 10));
	if(fullpath == NULL)
		exit(1);
	
	sprintf(fullpath, "pals%c%s.pal", DS, name);
	
	FILE* fp;
	fp = fopen(fullpath, "wb");
	if(fp == NULL)
		exit(1);
	
	/* Write numbers of entries */
	write_eightbytes(fp, (uint_64) paledit_data.n);
	
	/* Write entries */
	for(i = 0; i < paledit_data.n; ++i)
	{
		(eb) = (uint_64) round(paledit_data.pal_grays[i]*10000.0);
		write_eightbytes(fp, eb);
		(eb) = (uint_64) round(paledit_data.pal_cols[i].r*10000.0);
		write_eightbytes(fp, eb);
		(eb) = (uint_64) round(paledit_data.pal_cols[i].g*10000.0);
		write_eightbytes(fp, eb);
		(eb) = (uint_64) round(paledit_data.pal_cols[i].b*10000.0);
		write_eightbytes(fp, eb);
		(eb) = (uint_64) round(paledit_data.pal_cols[i].a*10000.0);
		write_eightbytes(fp, eb);
	}
	
	fclose(fp);
	free(fullpath);
}

void load_palette(char* name)
{
	char* fullpath;
	uint_64 eb;
	int i;
	fullpath = (char*) malloc(sizeof(char) * (strlen(name) + 10));
	if(fullpath == NULL)
		exit(1);
	
	sprintf(fullpath, "pals%c%s.pal", DS, name);
	
	FILE* fp;
	fp = fopen(fullpath, "rb");
	if(fp == NULL)
		return;
	
	/* Read numbers of entries */
	paledit_data.n = (int) read_eightbytes(fp);
	
	/* Read entries */
	for(i = 0; i < paledit_data.n; ++i)
	{
		eb = read_eightbytes(fp);
		paledit_data.pal_grays[i] = (double)(eb/10000.0);
		eb = read_eightbytes(fp);
		paledit_data.pal_cols[i].r = (double)(eb/10000.0);
		eb = read_eightbytes(fp);
		paledit_data.pal_cols[i].g = (double)(eb/10000.0);
		eb = read_eightbytes(fp);
		paledit_data.pal_cols[i].b = (double)(eb/10000.0);
		eb = read_eightbytes(fp);
		paledit_data.pal_cols[i].a = (double)(eb/10000.0);
	}
	
	fclose(fp);
	free(fullpath);
}

void load_palette_cc(const char* name)
{
	char* passname;
	passname = malloc(sizeof(char) * (strlen(name) + 1));
	if(passname == NULL)
		exit(1);
	strcpy(passname, name);
	load_palette(passname);
	free(passname);
}

void add_pal_entry(color_t col, double gray)
{
	if((gray < .0) || (gray > 1.0))
		return;
	
	int new_pos, i;
	/* Does grayval already exist? */
	for(i = 0; i < paledit_data.n; ++i)
	{
		if(paledit_data.pal_grays[i] == gray)
		{
			new_pos = i;
			goto add_pal_entry_add_sub;
		}
	}
	
	paledit_data.n++;
	/* find position in array and shift larger values. */
	for(new_pos = 0; new_pos < paledit_data.n-2; ++new_pos)
	{
		if((gray >= paledit_data.pal_grays[new_pos]) &&
		 (gray <= paledit_data.pal_grays[new_pos+1]))
			break;
	}
	new_pos++;
	for(i = paledit_data.n-1; i >= new_pos; --i)
	{
		paledit_data.pal_grays[i+1] = paledit_data.pal_grays[i];
		paledit_data.pal_cols[i+1] = paledit_data.pal_cols[i];
	}
	
	add_pal_entry_add_sub:
	paledit_data.pal_cols[new_pos] = col;
	paledit_data.pal_grays[new_pos] = gray;
	paledit_data.pal_change = TRUE;
}

void rem_pal_entry(int i)
{
	/* Shift larger elements back. */
	for(; i < paledit_data.n; ++i)
	{
		paledit_data.pal_cols[i] = paledit_data.pal_cols[i+1];
		paledit_data.pal_grays[i] = paledit_data.pal_grays[i+1];
	}
	
	paledit_data.n--;
	
	paledit_data.pal_change = TRUE;
}

void rem_nearest_pal(double gray)
{
	int i, nearest;
	nearest = 0;
	double dist = 10000.0;
	for(i = 0; i < paledit_data.n; ++i)
	{
		if(fabs(paledit_data.pal_grays[i] - gray) < dist)
		{
			nearest = i;
			dist = fabs(paledit_data.pal_grays[i] - gray);
		}
	}
	
	if((nearest == 0) || (nearest == paledit_data.n-1))
		return;
	
	rem_pal_entry(nearest);
}

DIALOG* md;

int color_box(int msg, DIALOG* d, int c)
{
	int r,g,b;
	r = md[1].d2;
	g = md[2].d2;
	b = md[3].d2;
	d->bg = 0xff000000 | makecol(r,g,b);
	return d_box_proc(msg,d,c);
}

int my_slider(int msg, DIALOG* d, int c)
{
	int old_val = d->d2;
	int ret;
	ret = d_slider_proc(msg,d,c);
	if(d->d2 != old_val)
		return D_REDRAW;
	else
		return ret;
}

int the_gradient(int msg, DIALOG* d, int c)
{
	int x,y;
	color_t col;
	if(paledit_data.pal_change)
	{
		for(x = 0; x < 370; ++x)
		{
			col = get_palette_color((double)((double)x/362.0), paledit_data.pal_cols,
			 paledit_data.pal_grays, paledit_data.n);
			for(y = 0; y < 30; ++y)
			{
				putpixel(paledit_data.gradient, x, y, makeacol(
				 (int) round(col.r*255), (int) round(col.g*255),
				 (int) round(col.b*255), 255));
			}
		}
		paledit_data.pal_change = FALSE;
	}
	
	d->dp = paledit_data.gradient;
	return d_bitmap_proc(msg, d, c);
}

int add_button(int msg, DIALOG* d, int c)
{
	if((d->flags & D_DISABLED) && (paledit_data.n < 30))
		d->flags &= (~D_DISABLED);
	
	int rv = d_button_proc(msg,d,c);
	color_t t;
	t.a = 1.0;
	if(rv == D_EXIT)
	{
		t.r = md[1].d2 / 255.0;
		t.g = md[2].d2 / 255.0;
		t.b = md[3].d2 / 255.0;
		add_pal_entry(t, md[8].d2 / 1000.0);
		if(paledit_data.n == 30)
			d->flags |= D_DISABLED;
		return D_REDRAW;
	}
	return rv;
}

int del_button(int msg, DIALOG* d, int c)
{
	int rv = d_button_proc(msg,d,c);
	if(rv == D_EXIT)
	{
		rem_nearest_pal(md[8].d2 / 1000.0);
		return D_REDRAW;
	}
	return rv;
}

int save_palette_btn(int msg, DIALOG* d, int c)
{
	int rv;
	
	rv = d_button_proc(msg,d,c);
	if(rv == D_EXIT)
	{
		if(strcmp(paledit_data.palname_buffer, ""))
		{
			save_palette(paledit_data.palname_buffer);
			alert("","Palette saved!","", "&OK!", NULL, 'o', 0);
		}
		return D_O_K;
	}
	return rv;
}

int load_palette_btn(int msg, DIALOG* d, int c)
{
	int rv;
	
	rv = d_button_proc(msg,d,c);
	if(rv == D_EXIT)
	{
		if(strcmp(paledit_data.palname_buffer, ""))
		{
			load_palette(paledit_data.palname_buffer);
			paledit_data.pal_change = TRUE;
		}
		return D_REDRAW;
	}
	return rv;
}

DIALOG the_dialog[] =
{
   /* (dialog proc)     (x)   (y)   (w)   (h) (fg)(bg) (key) (flags)     (d1) (d2)    (dp)                   (dp2) (dp3) */
   { d_box_proc,           0,   0,  374,   200, 0,  0,     0,      0,       0,   0,    NULL,                   NULL, NULL  },
   { my_slider,           12,  102,  300,   10, 0,  0,     0,      0,     255,   0,    NULL,                   NULL, NULL  },
   { my_slider,           12,  122,  300,   10, 0,  0,     0,      0,     255,   0,    NULL,                   NULL, NULL  },
   { my_slider,           12,  142,  300,   10, 0,  0,     0,      0,     255,   0,    NULL,                   NULL, NULL  },
   { color_box,          322,  102,  50,    50, 0,  0,     0,      0,     0,     0,    NULL,                   NULL, NULL  },
   { d_text_proc,          2,  102,  10,    10, 0,  0,     0,      0,     0,     0,    "R",                    NULL, NULL  },
   { d_text_proc,          2,  122,  10,    10, 0,  0,     0,      0,     0,     0,    "G",                    NULL, NULL  },
   { d_text_proc,          2,  142,  10,    10, 0,  0,     0,      0,     0,     0,    "B",                    NULL, NULL  },
   { d_slider_proc,        2,   62, 370,    10, 0,  0,     0,      0,  1000,     0,    NULL,                   NULL, NULL  },
   { the_gradient,         6,   22, 362,    30, 0,  0,     0,      0,     0,     0,    NULL,                   NULL, NULL  },
   { add_button,           6,   77, 165,    14, 0,  0,     0, D_EXIT,     0,     0,    "Add color here",       NULL, NULL  },
   { del_button,         192,   77, 180,    14, 0,  0,     0, D_EXIT,     0,     0,    "Delete nearest color", NULL, NULL  },
   { d_text_proc,          2,  162,  110,   10, 0,  0,     0,      0,     0,     0,    "Palette name:",        NULL, NULL  },
   { d_box_proc,         111,  161,  262,   10, 0,  0,     0,      0,     0,     0,    NULL,                   NULL, NULL  },
   { d_edit_proc,        112,  162,  260,    8, 0,  0,     0,      0,    40,     0,    paledit_data.palname_buffer, NULL, NULL},
   { save_palette_btn,    57,  177,  120,   14, 0,  0,     0, D_EXIT,     0,     0,    "Save palette",         NULL, NULL  },
   { load_palette_btn,   187,  177,  120,   14, 0,  0,     0, D_EXIT,     0,     0,    "Load palette",         NULL, NULL  },
   { d_text_proc,          2,    2,  112,   10, 0,  0,     0,     0,      0,     0,    "Palette editor",       NULL, NULL  },
   { d_button_proc,        358,  0,   16,   14, 0,  0,     0,D_EXIT,      0,     0,    "X",                    NULL,  NULL },
   
   /* the next two elements don't draw anything */
   { d_yield_proc,           0,  0,    0,    0, 0,  0,     0,     0,      0,     0,    NULL,                   NULL, NULL  },
   { NULL,                   0,  0,    0,    0, 0,  0,     0,     0,      0,     0,    NULL,                   NULL, NULL  }
};

void init_paledit(int canvas_w, int canvas_h)
{
	md = the_dialog;
	position_dialog(the_dialog, (canvas_w-374) / 2, (canvas_h-200)/2);
	paledit_data.pal_change = TRUE;
	paledit_data.gradient = create_bitmap(370, 30);
	set_dialog_color(the_dialog, gui_fg_color, gui_bg_color);
	the_dialog[0].bg = 0xffffffff;
	the_dialog[13].bg = 0xff000000 | makecol(255,255,255);
	the_dialog[13].fg = 0xff000000 | makecol(0,0,0);
	the_dialog[14].bg = makecol(255, 255, 255);
	the_dialog[14].fg = makecol(0,0,0);
}

void exec_paledit()
{
	do_dialog(the_dialog, -1);
}

void destroy_paledit()
{
	destroy_bitmap(paledit_data.gradient);
}
