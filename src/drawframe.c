#include <allegro5/allegro5.h>

#include "config.h"
#include "kart.h"
#include "drawtrack.h"
#include "drawframe.h"
#include "misc.h"

#include <math.h>

void drawframe(float x_pos, float y_pos, float kart_angle, float scale, int screen_width, 
		int screen_height, TRACK_DATA* track,
		/*Only used if camera is set to be relative to the track*/float track_angle,
		CONFIG* config){

	
	//draw player karts
	kart_t main_kart = {kart_angle,x_pos,y_pos,config->kart_width,config->kart_height, 
		config->kart_color};

	float angle=0;
	switch(config->camera_angle){
		case(AS_START):
			angle=M_PI;
			break;
		case(RELATIVE_TO_KART):
			angle=-kart_angle-M_PI/2;
			break;
		case(RELATIVE_TO_TRACK):
			angle=-track_angle-M_PI/2;
			break;
	}
	drawtrack(x_pos,y_pos,angle, scale, screen_width, screen_height,track);
	drawkart(x_pos, y_pos,angle, scale, screen_width, screen_height,main_kart);
		
}
// vim: cc=100
