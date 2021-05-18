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

#include <allegro5/allegro5.h>
#include <allegro5/allegro_font.h>

#include "global.h"
#include "display.h"
#include <stdio.h>

//If true it needs to redraw
_Bool handle_display(_Bool first/*Is true when this is the first loop*/,ALLEGRO_FONT* font){
	al_acknowledge_resize(disp);
	_Bool redraw = false;
	_Bool resized = false;
	switch(event.type){
		case(ALLEGRO_EVENT_DISPLAY_CLOSE):
			exit(1);
			break;
		case(ALLEGRO_EVENT_KEY_DOWN):
			if(event.keyboard.keycode==ALLEGRO_KEY_F11){
				al_set_display_flag(disp, ALLEGRO_FULLSCREEN_WINDOW, 
					!(_Bool)(al_get_display_flags(disp) & 
						 ALLEGRO_FULLSCREEN_WINDOW));
				resized=true;
			}
			break;
		case(ALLEGRO_EVENT_TIMER):
			redraw=true;
			break;
		case(ALLEGRO_EVENT_DISPLAY_RESIZE):
			resized=true;
	}
	if(resized|| first){
		redraw = true;
		font =  al_create_builtin_font();

		screen_width = al_get_display_width(disp);
		screen_height = al_get_display_height(disp);

		resized = true;

		
	}
	return redraw;
	
}
// vim: cc=100
