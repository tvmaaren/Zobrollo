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


#include <allegro5/allegro5.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>

#include "config.h"
#include "race.h"
#include "misc.h"

#define fps 10
#define window_width 100
#define window_height 100

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



ALLEGRO_FONT* get_font(char* name, int screen_width, int screen_height);


_Bool handle_click_box(click_box box, float mouse_x, float mouse_y);

void main(){
	CONFIG config;
	get_config(&config);
	
	//initialize and check for errors
	must_init(al_init(),"allegro");
	must_init(al_install_keyboard(),"couldn't initialize keyboard\n");

	//get a list of track names
	ALLEGRO_FS_ENTRY *track_dir = al_create_fs_entry("tracks");
	if(!al_open_directory(track_dir)){
		fprintf(stderr, "Could not open track directory\n");
		return;
	}
	
	int am_tracks = 0;
	int track_name_list_size = 10;
	int track_entry_list_size = 10;
	const char** track_names = malloc(sizeof(char*)*track_name_list_size);

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
		track_names[am_tracks-1] = strrchr(al_get_fs_entry_name(track_files[am_tracks-1]), '/')+1;
		printf("%s\n", track_names[am_tracks-1]);
	}
	printf("amount of tracks: %d\n", am_tracks);


	ALLEGRO_EVENT_QUEUE* queue = al_create_event_queue();
	must_init(queue,"queue");
	
	ALLEGRO_TIMER* timer = al_create_timer(1.0 / fps);
	must_init(timer,"timer");
	al_start_timer(timer);
    	al_register_event_source(queue, al_get_timer_event_source(timer));
    	
	al_init_font_addon();
	al_init_ttf_addon();
	char* font_name = "/usr/share/fonts/TTF/DejaVuSansCondensed-Bold.ttf";
	ALLEGRO_FONT* font = al_load_ttf_font(font_name, 10, ALLEGRO_TTF_MONOCHROME);

	if(font==NULL){
		fprintf(stderr, "Could not load font\n");
	}
	
	ALLEGRO_EVENT event;

	al_set_new_display_flags(ALLEGRO_RESIZABLE|ALLEGRO_WINDOWED);
	ALLEGRO_DISPLAY* disp = al_create_display(window_width, window_height);
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

	_Bool first = true;

	while(true){
		_Bool click = false;
		al_acknowledge_resize(disp);

		int screen_width = al_get_display_width(disp);
		int screen_height = al_get_display_height(disp);
		if(first||event.type==ALLEGRO_EVENT_DISPLAY_RESIZE){
			i=0;
			while(i<am_tracks){
				float y_centre = screen_height/(am_tracks+1)*(i+1);
				track_box[i].font =get_font(font_name, screen_width, screen_height);
				track_box[i].x1=screen_width/4;
				track_box[i].y1=y_centre-screen_height/(am_tracks+1)/2;
				track_box[i].x2=screen_width/4*3;
				track_box[i].y2=y_centre+screen_height/(am_tracks+1)/2;
				i++;
			}
		}
		ALLEGRO_MOUSE_STATE mouse_state;
		al_get_mouse_state(&mouse_state);
		_Bool EndProgram=false;
		switch(event.type){
			case(ALLEGRO_EVENT_DISPLAY_CLOSE):
				EndProgram=true;
				break;
			case(ALLEGRO_EVENT_MOUSE_BUTTON_DOWN):
				printf("Mouse down\n");
				click =true;
				break;
 
				
		}
		if(EndProgram){
			break;
		}
		
		al_clear_to_color(al_map_rgb(0,0,0));
		i=0;
		while(i<am_tracks){
			_Bool mouse_in_box = handle_click_box(track_box[i], mouse_state.x,mouse_state.y);
			if(mouse_in_box && click){
				printf("Before calling race func\n");
				race(track_files[i], &config, disp);
				printf("Back\n");
			}
			i++;
		}

		/*al_draw_rectangle(x1,y1,x2,y2,al_map_rgb(200,0,0), 2);
		if(mouse_state.x > x1 && mouse_state.y > y1 && mouse_state.x < x2 && mouse_state.y < y2)
			al_draw_filled_rectangle(x1+thickness/2,y1+thickness/2,x2-thickness/2,y2-thickness/2,
				al_map_rgb(0,200,0));*/
		al_draw_text(font, al_map_rgb(255,255,255), 0, 0, 0, "hey");
		al_flip_display();
		al_wait_for_event(queue,&event);
		first=false;
	}
}


//returns true if the mouse is above this box
_Bool handle_click_box(click_box box,float mouse_x, float mouse_y){
	_Bool ret = false;
	
	al_draw_rectangle(box.x1,box.y1,box.x2,box.y2,box.border_color, box.border_thickness);
	//al_flip_display();
	if(mouse_x > box.x1 && mouse_y > box.y1 && mouse_x < box.x2 && mouse_y < box.y2){
		ret=true;
		al_draw_filled_rectangle(box.x1+box.border_thickness/2,box.y1+box.border_thickness/2,
				box.x2-box.border_thickness/2,box.y2-box.border_thickness/2,al_map_rgb(0,200,0));
	}
	al_draw_text(box.font, box.text_color, (box.x1+box.x2)/2, (box.y1+box.y2)/2, ALLEGRO_ALIGN_CENTRE, box.text);
	return ret;
}

ALLEGRO_FONT* get_font(char* name, int screen_width, int screen_height){
	float width_height;
	if(screen_width>screen_height)
		width_height = screen_height/20;
	else
		width_height = screen_width/20;
	return(al_load_ttf_font_stretch(name, width_height, width_height, ALLEGRO_TTF_MONOCHROME));
}
