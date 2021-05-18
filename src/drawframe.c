#include <allegro5/allegro5.h>
#include <allegro5/allegro_primitives.h>

#include "global.h"
#include "config.h"
#include "kart.h"
#include "drawtrack.h"
#include "drawframe.h"
#include "misc.h"

#include <math.h>

void drawmap_with_karts(int am_karts,kart_t *karts//0th element is seen as main kart
		,float min_x //relative to the middle of the screen
		,float min_y //relative to the middle of the screen
		,float max_x //relative to the middle of the screen
		,float max_y //relative to the middle of the screen
		,TRACK_DATA* track){
	ALLEGRO_TRANSFORM transform;
	drawmap(min_x, min_y, max_x, max_y, &transform, track);
	
	//draw the dot that represents the player

	int i =0;
	while(i<am_karts){

		float dotx=karts[i].x;
		float doty=karts[i].y;
		al_transform_coordinates(&transform, &dotx, &doty);
		
		al_draw_filled_circle(dotx, doty, 3, karts[i].color);
		i++;
	}
}
	

void drawframe(int am_karts,int main_kart, kart_t *karts//0th element is seen as main kart
		, float scale, TRACK_DATA* track,/*Only used if camera is set to be relative to the track*/float track_angle){

	
	float angle=0;
	switch(config.camera_angle){
		case(AS_START):
			angle=M_PI;
			break;
		case(RELATIVE_TO_KART):
			angle=-karts[main_kart].angle-M_PI/2;
			break;
		case(RELATIVE_TO_TRACK):
			angle=-track_angle-M_PI/2;
			break;
	}
	int i =0;
	while(i<am_karts){
		drawkart(karts[main_kart].x,karts[main_kart].y,angle, scale,karts[i]);
		i++;
	}
	ALLEGRO_TRANSFORM transform;
	al_identity_transform(&transform);
	al_translate_transform(&transform, -karts[main_kart].x,-karts[main_kart].y );
	al_rotate_transform(&transform , angle);
	al_scale_transform(&transform, scale,scale);
	al_translate_transform(&transform, screen_width/2,screen_height/2);
	al_use_transform(&transform);

	drawtrack(track,scale);
	al_identity_transform(&transform);
	al_use_transform(&transform);
	//display the map
	if(config.show_map)
		drawmap_with_karts(am_karts, karts, 3*screen_width/4, screen_height/2, screen_width, 
				screen_height,  track);
		
}
// vim: cc=100
