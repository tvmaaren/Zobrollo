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
#include <signal.h>

#define PORT 8888
#define IP INADDR_ANY

#include <allegro5/allegro_image.h>
#include <allegro5/allegro5.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>

#ifdef _WIN32
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#include "global.h"
#include "networking.h"
#include "config.h"
#include "misc.h"
#include "file_paths.h"
#include "join_server.h"
#include "ghost.h"
#include "gui.h"
#include "drawtrack.h"
#include "start_server.h"
#include "record.h"
#include "race.h"
#include "display.h"



typedef struct{
	float x1frac;
	float y1frac;
	float x2frac;
	float y2frac;
}box_relative;


void track_menu(void (*click_func)(TRACK_DATA *track,char* filename));

void multiplayer_menu();


//Every file in the specified directory is an item in this menu
void track_menu(void (*click_func)(TRACK_DATA *track,char* filename)){

	ALLEGRO_FONT* font =  al_create_builtin_font();
	/*true: In the previuous frame the mouse was down
	 *false:The mouse is up*/
	_Bool prev_mouse_down = true;
	
	//get a list of file names
	ALLEGRO_FS_ENTRY *dir = al_create_fs_entry("tracks/");
	if(!al_open_directory(dir)){
		fprintf(stderr, "Could not open " data_dir"/tracks\n");
		return;
	}
	
	int am_files = 0;
	int file_name_list_size = 10;
	int file_entry_list_size = 10;
	char** file_names = malloc(sizeof(char*)*file_name_list_size);

	ALLEGRO_FS_ENTRY *file;
	ALLEGRO_FS_ENTRY **files= malloc(sizeof(ALLEGRO_FS_ENTRY *)*file_entry_list_size);
	while(true){
		char file_path;
		add_element((void*)file_names, &am_files, &file_name_list_size, sizeof(char*));
		am_files--;
		add_element((void*)files, &am_files, &file_entry_list_size, 
                            sizeof(ALLEGRO_FS_ENTRY *));
		if(!(files[am_files-1] = al_read_directory(dir))){
			am_files--;
			break;
		}
		file_names[am_files-1]  = strrchr(al_get_fs_entry_name(files[am_files-1]), 
                                                    sep_char)+1;

	}
	box_relative file_box[am_files];
	TRACK_DATA tracks[am_files];
	int i=0;
	while(i<am_files){
		ALLEGRO_FILE* track_file = al_open_fs_entry(files[i],"r");
		loadtrack(track_file, tracks+i);
		al_fclose(track_file);
		float y_centre = 1/(float)(am_files+1)*(i+1);
		file_box[i].x1frac = 0.25;
		file_box[i].x2frac = 0.75;
		file_box[i].y1frac = (float)y_centre-0.9/(float)(am_files+1)/2;
		file_box[i].y2frac = (float)y_centre+0.9/(float)(am_files+1)/2;
		
		i++;
	}

	_Bool first = true;
	
	_Bool back_from_race = false;

	while(true){
		_Bool click = false;
		_Bool EndProgram=false;
		_Bool redraw = false;
		handle_display(first, font);
		if(event.type == ALLEGRO_EVENT_KEY_DOWN 
				&& event.keyboard.keycode == ALLEGRO_KEY_ESCAPE)
			return;
		_Bool mouse_down;
		if(first|al_is_event_queue_empty(queue)){
			al_clear_to_color(al_map_rgb(0,0,0));
			ALLEGRO_MOUSE_STATE mouse_state;
			al_get_mouse_state(&mouse_state);
			mouse_down = (_Bool)(mouse_state.buttons&0x1);
			if(mouse_down && !prev_mouse_down){
				click =true;
			}

			al_draw_text(font, config.button_text_color, 0, 0, 0, "Zobrollo v" version);
			i=0;
			while(i<am_files){

				_Bool mouse_in_box = handle_click_box_relative(mouse_state.x,
						mouse_state.y, file_box[i].x1frac,
						file_box[i].y1frac,file_box[i].x2frac,
						file_box[i].y2frac, " ");

				//draw contents of the box
				float box_width =file_box[i].x2frac-file_box[i].x1frac;
				float box_height =file_box[i].y2frac-file_box[i].y1frac;
				float track_x1=(box_width*0.1+file_box[i].x1frac)*screen_width;
				float track_y1=(box_height*0.1+file_box[i].y1frac)*screen_height;
				float track_x2=(box_width*0.9+file_box[i].x1frac)*screen_width;
				float track_y2=(box_height*0.6+file_box[i].y1frac)*screen_height;
				al_draw_rectangle(track_x1,track_y1,track_x2,track_y2,
						config.button_border_color, 
						config.button_border_thickness);
				al_draw_filled_rectangle(
						track_x1+config.button_border_thickness/2,
						track_y1+config.button_border_thickness/2,
						track_x2-config.button_border_thickness/2,
						track_y2-config.button_border_thickness/2,
						al_map_rgb(0,0,0));
						
				drawmap(track_x1,track_y1,track_x2,track_y2,
						NULL, tracks+i);
				draw_text(config.font_name, file_names[i], 
						config.button_text_color,
						(box_width*0.5+file_box[i].x1frac)*screen_width,
						(box_height*0.8+file_box[i].y1frac)*screen_height,
						box_width*0.8*screen_width,
						box_height*0.2*screen_height);
						
						


				if(mouse_in_box && click){

					click_func(tracks+i, file_names[i]);
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
		prev_mouse_down = mouse_down;
		al_wait_for_event(queue,&event);
	}
}

void multiplayer_menu(){

	ALLEGRO_FONT* font = al_create_builtin_font();

	/*true: In the previuous frame the mouse was down
	 *false:The mouse is up*/
	_Bool prev_mouse_down = true;
	

	_Bool first = true;
	int screen_width; int screen_height;
	_Bool back= false;
	while(true){
		_Bool click = false;
		_Bool EndProgram=false;
		_Bool redraw = false;
		handle_display(first,font);
		if(event.type == ALLEGRO_EVENT_KEY_DOWN 
				&& event.keyboard.keycode == ALLEGRO_KEY_ESCAPE)
			return;
		_Bool mouse_down;
		if(first|al_is_event_queue_empty(queue)){
			al_clear_to_color(al_map_rgb(0,0,0));
			ALLEGRO_MOUSE_STATE mouse_state;
			al_get_mouse_state(&mouse_state);
			mouse_down = (_Bool)(mouse_state.buttons&0x1);
			if(mouse_down && !prev_mouse_down){
				click =true;
			}
			if(handle_click_box_relative(mouse_state.x, mouse_state.y,0.20,0.20,0.80,
						0.45,"Join Server")&&click){
				join_server(config,disp,paths, event, queue, font);
				back=true;
			}
			if(handle_click_box_relative(mouse_state.x, mouse_state.y,0.20,0.55,0.80,
						0.80, "Start Server")&&click){
				track_menu(start_server);
				back=true;
			}

			al_draw_text(font, al_map_rgb(255,255,255), 0, 0, 0, "Zobrollo v" version);
			al_flip_display();
		}
		first=back;
		back=false;
		prev_mouse_down = mouse_down;
		al_wait_for_event(queue,&event);
	}
}
