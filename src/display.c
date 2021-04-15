#include <allegro5/allegro5.h>
#include <allegro5/allegro_font.h>

#include "display.h"

//If true it needs to redraw
_Bool handle_display(int* screen_width, int* screen_height, 
		_Bool first/*Is true when this is the first loop*/, ALLEGRO_DISPLAY* disp, 
		ALLEGRO_EVENT*event, ALLEGRO_EVENT_QUEUE* queue, ALLEGRO_FONT* font){
	al_acknowledge_resize(disp);
	_Bool EndProgram=false;
	_Bool redraw = false;
	switch(event->type){
		case(ALLEGRO_EVENT_DISPLAY_CLOSE):
			exit(1);
			break;
		case(ALLEGRO_EVENT_KEY_DOWN):
			switch(event->keyboard.keycode){
				case(ALLEGRO_KEY_F11):
					al_set_display_flag(disp, ALLEGRO_FULLSCREEN_WINDOW, 
						!(_Bool)(al_get_display_flags(disp) & 
							 ALLEGRO_FULLSCREEN_WINDOW));
					redraw = true;//it must then redraw the boxes
					break;
			}
			break;
		case(ALLEGRO_EVENT_TIMER):
			redraw=true;
			break;
	}
	_Bool resized=true;
	if(event->type == ALLEGRO_EVENT_DISPLAY_RESIZE || first){
		font =  al_create_builtin_font();

		*screen_width = al_get_display_width(disp);
		*screen_height = al_get_display_height(disp);

		resized = true;

		
	}
	
	if(EndProgram){
		exit(1);
	}
}
// vim: cc=100
