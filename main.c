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


#include <stdio.h>
#include <stdlib.h>


#include <allegro5/allegro_image.h>
#include <allegro5/allegro5.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>

#include "config.h"
#include "race.h"
#include "misc.h"
#include "file_paths.h"

#define version "0.1.2"

#define thickness 2


typedef struct{
	float x1,y1,x2,y2;
	float border_thickness;
	ALLEGRO_COLOR border_color;
	ALLEGRO_COLOR area_color;
	ALLEGRO_COLOR text_color;
	const char* text;
	ALLEGRO_FONT* font;
}click_box;



ALLEGRO_FONT* get_font(const char* name, int screen_width, int screen_height);


_Bool handle_click_box(click_box box, float mouse_x, float mouse_y);

void main(){
	CONFIG config;
	get_config(&config);
	
	//initialize and check for errors
	must_init(al_init(),"allegro");
	must_init(al_install_keyboard(),"couldn't initialize keyboard\n");
	must_init(al_init_primitives_addon(), "primitives");
	
	must_init(al_init_image_addon(), "image");

	//get a list of track names
	ALLEGRO_FS_ENTRY *track_dir = al_create_fs_entry(data_dir"/tracks");
	if(!al_open_directory(track_dir)){
		fprintf(stderr, "Could not open track directory\n");
		return;
	}
	
	int am_tracks = 0;
	int track_name_list_size = 10;
	int track_entry_list_size = 10;
	char** track_names = malloc(sizeof(char*)*track_name_list_size);

	ALLEGRO_FS_ENTRY *track_file;
	ALLEGRO_FS_ENTRY **track_files= malloc(sizeof(ALLEGRO_FS_ENTRY *)*track_entry_list_size);
	while(true){
		char track_path;
		add_element((void*)track_names, &am_tracks, &track_name_list_size, sizeof(char*));
		am_tracks--;
		add_element((void*)track_files, &am_tracks, &track_entry_list_size, sizeof(ALLEGRO_FS_ENTRY *));
		if(!(track_files[am_tracks-1] = al_read_directory(track_dir))){
			am_tracks--;
			break;
		}
		track_names[am_tracks-1]  = strrchr(al_get_fs_entry_name(track_files[am_tracks-1]), sep_char)+1;

	}


	ALLEGRO_EVENT_QUEUE* queue = al_create_event_queue();
	must_init(queue,"queue");

	al_register_event_source(queue, al_get_keyboard_event_source());


    	
	must_init(al_init_font_addon(), "font addon");
	must_init(al_init_ttf_addon(), "ttf");
	ALLEGRO_FONT* font =  al_create_builtin_font();
	must_init(font, "builtin font");

	if(font==NULL){
		fprintf(stderr, "Could not load font\n");
	}
	
	ALLEGRO_EVENT event;

	al_set_new_display_flags(ALLEGRO_RESIZABLE|ALLEGRO_WINDOWED);
	ALLEGRO_DISPLAY* disp = al_create_display(config.window_width, config.window_height);
	must_init(disp,"couldn't initialize display\n");
    	al_register_event_source(queue, al_get_display_event_source(disp));

	must_init(al_install_mouse(), "mouse");
    	al_register_event_source(queue, al_get_mouse_event_source());
	click_box track_box[am_tracks];

	int i=0;
	while(i<am_tracks){
		track_box[i].border_thickness = 2;
		track_box[i].border_color = al_map_rgb(200,0,0);
		track_box[i].area_color = al_map_rgb(0,200,0);
		track_box[i].text = track_names[i];
		track_box[i].text_color = al_map_rgb(255, 255, 255);
		i++;
	}
	#define KEY_SEEN     1
	#define KEY_RELEASED 2

	unsigned char key[ALLEGRO_KEY_MAX];
	memset(key, 0, sizeof(key));

	_Bool first = true;
	
	int screen_width;
	int screen_height;

	_Bool back_from_race = false;

	while(true){
		al_acknowledge_resize(disp);
		_Bool click = false;
		_Bool EndProgram=false;
		switch(event.type){
			case(ALLEGRO_EVENT_DISPLAY_CLOSE):
				EndProgram=true;
				break;
			case(ALLEGRO_EVENT_MOUSE_BUTTON_DOWN):
				click =true;
				break;
			case(ALLEGRO_EVENT_KEY_DOWN):
				if(ALLEGRO_KEY_F11==event.keyboard.keycode){
					al_set_display_flag(disp, ALLEGRO_FULLSCREEN_WINDOW, 
						!(_Bool)(al_get_display_flags(disp) & ALLEGRO_FULLSCREEN_WINDOW));
					first = true;//it must then redraw the boxes
				}
						
				break;
		}
		if(event.type == ALLEGRO_EVENT_DISPLAY_RESIZE || first){
			font =  al_create_builtin_font();
			must_init(font, "builtin font");

			screen_width = al_get_display_width(disp);
			screen_height = al_get_display_height(disp);
			i=0;
			while(i<am_tracks){
				float y_centre = screen_height/(am_tracks+1)*(i+1);
				
				track_box[i].font =get_font(config.font_name, screen_width, screen_height);
				if(!track_box[i].font)
					printf("Couldn't get font %s\n",config.font_name);
				
				track_box[i].x1=screen_width/4;
				track_box[i].y1=y_centre-screen_height/(am_tracks+1)/2;
				track_box[i].x2=screen_width/4*3;
				track_box[i].y2=y_centre+screen_height/(am_tracks+1)/2;
				i++;
			}
		}
		
		if(EndProgram){
			break;
		}
		if(first|al_is_event_queue_empty(queue)){
			ALLEGRO_MOUSE_STATE mouse_state;
			al_get_mouse_state(&mouse_state);
			al_clear_to_color(al_map_rgb(0,0,0));

			al_draw_text(font, al_map_rgb(255,255,255), 0, 0, 0, "Zobrollo v" version);
			
			i=0;
			while(i<am_tracks){
				_Bool mouse_in_box = handle_click_box(track_box[i], mouse_state.x,mouse_state.y);
				if(mouse_in_box && click){
					race(track_files[i], &config, disp);
					back_from_race=true;
					al_flush_event_queue(queue);
					break;
				}
				i++;
			}

			al_flip_display();
		}
		first=back_from_race;
		back_from_race=false;
		al_wait_for_event(queue,&event);
	}
}


//returns true if the mouse is above this box
_Bool handle_click_box(click_box box,float mouse_x, float mouse_y){
	_Bool ret = false;
	al_draw_rectangle(box.x1,box.y1,box.x2,box.y2,box.border_color, box.border_thickness);
	if(mouse_x > box.x1 && mouse_y > box.y1 && mouse_x < box.x2 && mouse_y < box.y2){
		ret=true;
		al_draw_filled_rectangle(box.x1+box.border_thickness/2,box.y1+box.border_thickness/2,
				box.x2-box.border_thickness/2,box.y2-box.border_thickness/2,al_map_rgb(0,200,0));
	}
	al_draw_text(box.font, box.text_color, (box.x1+box.x2)/2, (box.y1+box.y2)/2, 
			ALLEGRO_ALIGN_CENTRE, box.text);
	return ret;
}

ALLEGRO_FONT* get_font(const char* name, int screen_width, int screen_height){
	float width_height;
	if(screen_width>screen_height)
		width_height = screen_height/15;
	else
		width_height = screen_width/15;
	return(al_load_ttf_font_stretch(name, width_height, width_height, ALLEGRO_TTF_MONOCHROME));
}
