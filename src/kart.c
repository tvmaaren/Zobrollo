/*
Zobrollo is a 2d minimalistic top-view racing game
Copyright (C) 2021  Thomas van Maaren

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.

e-mail:thomas.v.maaren@outlook.com
*/


#include <math.h>

#include <allegro5/allegro5.h>
#include <allegro5/allegro_primitives.h>

#include "global.h"
#include "config.h"
#include "kart.h"


void drawkart(float x,float y, float angle, float scale,kart_t kart){
	ALLEGRO_TRANSFORM transform;
	al_identity_transform(&transform);
	al_translate_transform(&transform, -x,-y );
	al_rotate_transform(&transform , angle);
	al_scale_transform(&transform, scale,scale);
	al_translate_transform(&transform, screen_width/2,screen_height/2);
	al_use_transform(&transform);

	int i=0;
	float points[8];
	float point_distance = sqrt(config.kart_height*config.kart_height+config.kart_width*config.kart_width)/2;
	while(i<4){
		float slope;
		if(i%2==1)
			slope=config.kart_height/config.kart_width;
		else
			slope=config.kart_width/config.kart_height;
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
// vim: cc=100
