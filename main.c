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
#include "record.h"

#define version "0.2.0"

#define thickness 2


typedef struct{
	//Position of corners as a fraction of the screen.
	float x1frac,y1frac,x2frac,y2frac;

	float border_thickness;
	ALLEGRO_COLOR border_color;
	ALLEGRO_COLOR select_color;
	ALLEGRO_COLOR text_color;
	char* text;
	const char* font_name;
}click_box;



void draw_text(const char* name, char* text, ALLEGRO_COLOR color, float x, 
		float y, int max_width, int max_height, _Bool resized);

_Bool handle_click_box(click_box box,float mouse_x, float mouse_y, 
		int screen_width, int screen_height, _Bool resized);

void dir_menu(CONFIG* config, ALLEGRO_DISPLAY* disp, ALLEGRO_EVENT* event, 
		ALLEGRO_EVENT_QUEUE *queue, char* dir_path,
		void (*click_func)(ALLEGRO_FS_ENTRY *track_file_entry,char* filename,
			CONFIG* config, ALLEGRO_DISPLAY* disp));

void main(){
	//get users directory
	char *home_path = getenv(home_var);
	
	/*true: In the previuous frame the mouse was down
	 *false:The mouse is up*/
	_Bool prev_mouse_down = false;
	//initialize and check for errors
	must_init(al_init(),"allegro");
	
	//Load configuration	
	char config_path[strlen(home_path)+sizeof(local_dir sep_str "config.cfg")];
	strcpy(config_path, home_path);

	const ALLEGRO_CONFIG* cfg = al_load_config_file(
			strcat(config_path,local_dir sep_str "config.cfg"));
	if(!cfg){
		cfg= al_load_config_file(data_dir sep_str "config.cfg");
		if(!cfg){
			fprintf(stderr, "Error: Could not find \"config.cfg\"\n");
			exit(1);
		}
	}
	CONFIG config;
	get_config(&config, cfg);
	
	
	//initialize and check for errors
	must_init(al_init(),"allegro");
	must_init(al_install_keyboard(),"couldn't initialize keyboard\n");
	must_init(al_init_primitives_addon(), "primitives");
	
	must_init(al_init_image_addon(), "image");

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

	
	click_box start_box = {0.1,0.1,0.5,0.9,config.button_border_thickness, config.button_border_color,
		config.button_select_color, config.button_text_color, "Race!", config.font_name};
	click_box record_box = {0.5,0.1,0.9,0.5,config.button_border_thickness, config.button_border_color,
		config.button_select_color, config.button_text_color, "Records", config.font_name};
	click_box quit_box = {0.5,0.5,0.9,0.9,config.button_border_thickness, config.button_border_color,
		config.button_select_color, config.button_text_color, "Quit", config.font_name};

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
			case(ALLEGRO_EVENT_KEY_DOWN):
				if(ALLEGRO_KEY_F11==event.keyboard.keycode){
					al_set_display_flag(disp, ALLEGRO_FULLSCREEN_WINDOW, 
						!(_Bool)(al_get_display_flags(disp) & 
                                                         ALLEGRO_FULLSCREEN_WINDOW));
					first = true;//it must then redraw the boxes
				}
						
				break;
		}
		_Bool resized=false;
		if(event.type == ALLEGRO_EVENT_DISPLAY_RESIZE || first){
			font =  al_create_builtin_font();
			must_init(font, "builtin font");

			screen_width = al_get_display_width(disp);
			screen_height = al_get_display_height(disp);

			resized = true;

		}
		
		if(EndProgram){
			break;
		}
		_Bool mouse_down;
		if(first|al_is_event_queue_empty(queue)){
			ALLEGRO_MOUSE_STATE mouse_state;
			al_get_mouse_state(&mouse_state);
			al_clear_to_color(al_map_rgb(0,0,0));
			mouse_down = (_Bool)(mouse_state.buttons&0x1);
			if(mouse_down && !prev_mouse_down)
				click =true;

			al_draw_text(font, al_map_rgb(255,255,255), 0, 0, 0, "Zobrollo v" version);
			if(handle_click_box(start_box, mouse_state.x, mouse_state.y, 
						screen_width, screen_height, resized)&&click){
				dir_menu(&config, disp, &event, queue, data_dir sep_str "tracks",
						race);
			}
			if(handle_click_box(record_box, mouse_state.x, mouse_state.y, 
						screen_width, screen_height, resized)&&click){
				
				char record_path[strlen(home_path) + 
					strlen(local_dir sep_str"records")];
				strcpy(record_path, home_path);
				dir_menu(&config, disp, &event, queue, 
						strcat(record_path, local_dir sep_str "records"),
						show_record);

			}
			if(handle_click_box(quit_box, mouse_state.x, mouse_state.y, 
						screen_width, screen_height, resized)&&click){
				exit(1);
			}
			al_flip_display();
		}
		al_wait_for_event(queue,&event);
		prev_mouse_down = mouse_down;
	}

	al_shutdown_font_addon();
}


//Every filein the specified directory is an item in this menu
void dir_menu(CONFIG* config, ALLEGRO_DISPLAY* disp, ALLEGRO_EVENT* event, 
		ALLEGRO_EVENT_QUEUE *queue, char* dir_path,
		void (*click_func)(ALLEGRO_FS_ENTRY *track_file_entry,char* filename, 
			CONFIG* config, ALLEGRO_DISPLAY* disp)){

	/*true: In the previuous frame the mouse was down
	 *false:The mouse is up*/
	_Bool prev_mouse_down = true;
	
	//get a list of file names
	ALLEGRO_FS_ENTRY *dir = al_create_fs_entry(dir_path);
	if(!al_open_directory(dir)){
		fprintf(stderr, "Could not open \"%s\"\n", dir_path);
		return;
	}
	ALLEGRO_FONT* font =  al_create_builtin_font();
	
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
	click_box file_box[am_files];
	int i=0;
	while(i<am_files){
		float y_centre = 1/(float)(am_files+1)*(i+1);
		file_box[i].x1frac = 0.25;
		file_box[i].x2frac = 0.75;
		file_box[i].y1frac = (float)y_centre-1/(float)(am_files+1)/2;
		file_box[i].y2frac = (float)y_centre+1/(float)(am_files+1)/2;
		file_box[i].border_thickness = config->button_border_thickness;
		file_box[i].border_color = config->button_border_color;
		file_box[i].select_color = config->button_select_color;
		file_box[i].text = file_names[i];
		file_box[i].text_color = config->button_text_color;
		file_box[i].font_name = config->font_name;
		
		i++;
	}
	#define KEY_SEEN     1
	#define KEY_RELEASED 2

	unsigned char key[ALLEGRO_KEY_MAX];
	memset(key, 0, sizeof(key));

	_Bool first = true;
	
	int screen_width; int screen_height;

	_Bool back_from_race = false;

	while(true){
		al_acknowledge_resize(disp);
		_Bool click = false;
		_Bool EndProgram=false;
		switch(event->type){
			case(ALLEGRO_EVENT_DISPLAY_CLOSE):
				EndProgram=true;
				break;
			case(ALLEGRO_EVENT_KEY_DOWN):
				switch(event->keyboard.keycode){
					case(ALLEGRO_KEY_F11):
						al_set_display_flag(disp, ALLEGRO_FULLSCREEN_WINDOW, 
							!(_Bool)(al_get_display_flags(disp) & 
								 ALLEGRO_FULLSCREEN_WINDOW));
						first = true;//it must then redraw the boxes
						break;
					case(ALLEGRO_KEY_ESCAPE):
						return;
				}
				break;
		}
		_Bool resized=true;
		if(event->type == ALLEGRO_EVENT_DISPLAY_RESIZE || first){
			font =  al_create_builtin_font();
			must_init(font, "builtin font");

			screen_width = al_get_display_width(disp);
			screen_height = al_get_display_height(disp);

			resized = true;

			
		}
		
		if(EndProgram){
			exit(1);
		}
		_Bool mouse_down;
		if(first|al_is_event_queue_empty(queue)){
			ALLEGRO_MOUSE_STATE mouse_state;
			al_get_mouse_state(&mouse_state);
			al_clear_to_color(al_map_rgb(0,0,0));
			mouse_down = (_Bool)(mouse_state.buttons&0x1);
			if(mouse_down && !prev_mouse_down){
				click =true;
			}

			al_draw_text(font, al_map_rgb(255,255,255), 0, 0, 0, "Zobrollo v" version);
			i=0;
			while(i<am_files){
				_Bool mouse_in_box = handle_click_box(file_box[i], mouse_state.x,mouse_state.y, screen_width, screen_height, resized);
				if(mouse_in_box && click){

					click_func(files[i], file_names[i], config, disp);
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
		al_wait_for_event(queue,event);
	}
}

//returns true if the mouse is above this box
_Bool handle_click_box(click_box box,float mouse_x, float mouse_y, 
		int screen_width, int screen_height, _Bool resized){
	float x1 = box.x1frac*screen_width;
	float x2 = box.x2frac*screen_width;
	float y1 = box.y1frac*screen_height;
	float y2 = box.y2frac*screen_height;

	_Bool ret= false;
	al_draw_rectangle(x1,y1,x2,y2,box.border_color, box.border_thickness);
	if(mouse_x > x1 && mouse_y > y1 && mouse_x < x2 && mouse_y < y2){
		ret=true;
		al_draw_filled_rectangle(x1+box.border_thickness/2,
                                         y1+box.border_thickness/2,
				         x2-box.border_thickness/2,
                                         y2-box.border_thickness/2,
                                         al_map_rgb(0,200,0));
	}
	draw_text(box.font_name, box.text, box.text_color, (x2+x1)/2, (y2+y1)/2, x2-x1, 
			y2-y1, resized);
	return ret;
}

void draw_text(const char* name, char* text, ALLEGRO_COLOR color, float x, 
		float y, int max_width, int max_height, _Bool resized){
	int text_len = strlen(text);
	float width_height;
	if(max_width>max_height)
		width_height = max_width/strlen(text);
	else
		width_height = max_width/strlen(text);
		//only reload if the size of the window has changed
	ALLEGRO_FONT* font = al_load_ttf_font_stretch(name, width_height, width_height, 
			ALLEGRO_TTF_MONOCHROME);
	al_draw_text(font, color, x, y-width_height/2, ALLEGRO_ALIGN_CENTRE, text);
	al_destroy_font(font);
}
// vim: cc=100
