#include <math.h>

#include <allegro5/allegro5.h>
#include <allegro5/allegro_primitives.h>

#include "kart.h"


void drawkart(float x,float y, float angle, float scale, int width, int height, 
		kart_t kart){
	ALLEGRO_TRANSFORM transform;
	al_identity_transform(&transform);
	al_translate_transform(&transform, -x,-y );
	al_rotate_transform(&transform , angle);
	al_scale_transform(&transform, scale,scale);
	al_translate_transform(&transform, width/2,height/2);
	al_use_transform(&transform);

	int i=0;
	float points[8];
	float point_distance = sqrt(kart.kart_height*kart.kart_height+kart.kart_width*kart.kart_width)/2;
	while(i<4){
		float slope;
		if(i%2==1)
			slope=kart.kart_height/kart.kart_width;
		else
			slope=kart.kart_width/kart.kart_height;
		double point_angle=-M_PI+atan(slope)+M_PI/2*i;
		points[2*i]=cos(point_angle+kart.angle)*point_distance+kart.x;
		points[2*i+1]=sin(point_angle+kart.angle)*point_distance+kart.y;
		i++;
	}
	//TODO make this drawing more efficient
	al_draw_filled_triangle(points[0],points[1],points[2],points[3],points[4],points[5],
			kart.color);

	al_draw_filled_triangle(points[4],points[5],points[0],points[1],points[6],points[7],
			kart.color);
	al_identity_transform(&transform);
	al_use_transform(&transform);
	return;
}
