#include <allegro5/allegro5.h>
#include <allegro5/allegro_primitives.h>

#include "config.h"
#include "kart.h"
#include "drawtrack.h"
#include "drawframe.h"
#include "misc.h"

#include <math.h>

void drawmap(int am_karts,kart_t *karts//0th element is seen as main kart
		,float min_x //relative to the middle of the screen
		,float min_y //relative to the middle of the screen
		,float max_x //relative to the middle of the screen
		,float max_y //relative to the middle of the screen
		,TRACK_DATA* track, CONFIG* config){
	float scale=(max_x-min_x)/(track->max_min[track_max_x]-track->max_min[track_min_x]);
	if((max_y-min_y)/(track->max_min[track_min_y] - track->max_min[track_max_y])<scale)
		scale=(max_y-min_y)/(track->max_min[track_min_y] - track->max_min[track_max_y]);


	float midx = (min_x+max_x)/2;
	float midy = (min_y+max_y)/2;
	ALLEGRO_TRANSFORM transform;
	al_identity_transform(&transform);
	al_translate_transform(&transform, 
			-(track->max_min[track_min_x] + track->max_min[track_max_x])/2, 
			-(track->max_min[track_min_y] + track->max_min[track_max_y])/2);
	al_scale_transform(&transform, scale,scale);
	al_translate_transform(&transform, midx,midy);
	al_use_transform(&transform);
	drawtrack(track,scale);
	
	//draw the dot that represents the player
	ALLEGRO_TRANSFORM identity;
	al_identity_transform(&identity);

	int i =0;
	while(i<am_karts){

		float dotx=karts[i].x;
		float doty=karts[i].y;
		al_transform_coordinates(&transform, &dotx, &doty);
		
		al_use_transform(&identity);

		al_draw_filled_circle(dotx, doty, 3, karts[i].color);
		i++;
	}
}


void drawframe(int am_karts,kart_t *karts//0th element is seen as main kart
		, float scale, int screen_width, int screen_height, TRACK_DATA* track,
		/*Only used if camera is set to be relative to the track*/float track_angle,
		CONFIG* config){

	
	float angle=0;
	switch(config->camera_angle){
		case(AS_START):
			angle=M_PI;
			break;
		case(RELATIVE_TO_KART):
			angle=-karts[0].angle-M_PI/2;
			break;
		case(RELATIVE_TO_TRACK):
			angle=-track_angle-M_PI/2;
			break;
	}
	int i =0;
	while(i<am_karts){
		drawkart(karts[0].x,karts[0].y,angle, scale, screen_width, screen_height,karts[i],
				config);
		i++;
	}
	ALLEGRO_TRANSFORM transform;
	al_identity_transform(&transform);
	al_translate_transform(&transform, -karts[0].x,-karts[0].y );
	al_rotate_transform(&transform , angle);
	al_scale_transform(&transform, scale,scale);
	al_translate_transform(&transform, screen_width/2,screen_height/2);
	al_use_transform(&transform);

	drawtrack(track,scale);
	al_identity_transform(&transform);
	al_use_transform(&transform);
	//display the map
	if(config->show_map)
		drawmap(am_karts, karts, 3*screen_width/4, screen_height/2, screen_width, 
				screen_height,  track,config);
		
}
// vim: cc=100
