#include <allegro5/allegro5.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>

#include <stdio.h>

#include "config.h"
#include "gui.h"


//returns true if the mouse is on this box
_Bool handle_click_box(float mouse_x,float mouse_y,float x1,float y1,float x2,float y2,
		CONFIG* config, char* text){

	_Bool ret= false;
	al_draw_rectangle(x1,y1,x2,y2,config->button_border_color, config->button_border_thickness);
	if(mouse_x > x1 && mouse_y > y1 && mouse_x < x2 && mouse_y < y2){
		ret=true;
		al_draw_filled_rectangle(x1+config->button_border_thickness/2,
                                         y1+config->button_border_thickness/2,
				         x2-config->button_border_thickness/2,
                                         y2-config->button_border_thickness/2,
                                         al_map_rgb(0,200,0));
	}
	printf("bef draw_text\n");
	draw_text(config->font_name, text, config->button_text_color, (x2+x1)/2, (y2+y1)/2, 
			x2-x1, y2-y1);
	printf("A draw_text\n");
	return ret;
}

//returns true if the mouse is on this box
_Bool handle_click_box_relative(float mouse_x,float mouse_y,float x1frac,float y1frac,float x2frac,
		float y2frac, int screen_width, int screen_height, CONFIG* config, char* text){
	float x1 = x1frac*screen_width;
	float x2 = x2frac*screen_width;
	float y1 = y1frac*screen_height;
	float y2 = y2frac*screen_height;

	return handle_click_box(mouse_x, mouse_y, x1, y1, x2, y2, config, text);
}

void draw_text(const char* name, char* text, ALLEGRO_COLOR color, float x, 
		float y, int max_width, int max_height){
	int text_len = strlen(text);
	float width_height;
	if(max_width>max_height)
		width_height = max_width/strlen(text);
	else
		width_height = max_width/strlen(text);
		//only reload if the size of the window has changed
	
	ALLEGRO_FONT* font = al_load_ttf_font_stretch(name, width_height, width_height, 
			ALLEGRO_TTF_MONOCHROME);
	al_draw_text(font, color, x, y-width_height/2, ALLEGRO_ALIGN_CENTRE, text);
	al_destroy_font(font);
}
// vim: cc=100
