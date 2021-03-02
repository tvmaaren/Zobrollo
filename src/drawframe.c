#include <allegro5/allegro5.h>

#include "config.h"
#include "kart.h"
#include "drawtrack.h"
#include "drawframe.h"

void drawframe(float x_pos, float y_pos, float kart_angle, float scale, int screen_width, 
		int screen_height, TRACK_DATA* track, CONFIG* config){
	drawtrack(x_pos,y_pos, 0,scale,screen_width, screen_height,track);
	
	//draw player karts
	ALLEGRO_COLOR kart_color=al_map_rgb(config->kart_color_r,config->kart_color_g,
			config->kart_color_b);
	kart_t main_kart = {kart_angle,x_pos,y_pos,config->kart_width,config->kart_height, kart_color};
	drawkart(x_pos, y_pos,0, scale, screen_width, screen_height,  main_kart);
}
// vim: cc=100
