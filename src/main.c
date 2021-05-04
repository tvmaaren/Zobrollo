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

#include "networking.h"
#include "config.h"
#include "misc.h"
#include "file_paths.h"
#include "input_boxes.h"
#include "ghost.h"
#include "gui.h"
#include "drawtrack.h"
#include "record.h"
#include "race.h"
#include "display.h"

typedef struct{
	float x1frac;
	float y1frac;
	float x2frac;
	float y2frac;
}box_relative;


void track_menu(CONFIG* config, ALLEGRO_DISPLAY* disp, ALLEGRO_EVENT* event,
		ALLEGRO_EVENT_QUEUE *queue, char* dir_path, PATHS* paths,ALLEGRO_FONT* font,
		void (*click_func)(TRACK_DATA *track,char* filename,
			CONFIG* config, ALLEGRO_DISPLAY* disp, PATHS* paths,ALLEGRO_EVENT* event,
			ALLEGRO_EVENT_QUEUE *queue, ALLEGRO_FONT* font));

void multiplayer_menu(CONFIG* config, ALLEGRO_DISPLAY* disp, PATHS* paths, ALLEGRO_EVENT* event,
		ALLEGRO_EVENT_QUEUE *queue, ALLEGRO_FONT* font);

void start_server(TRACK_DATA *track,char* filename,CONFIG* config, ALLEGRO_DISPLAY* disp, 
		PATHS* paths,ALLEGRO_EVENT* event,ALLEGRO_EVENT_QUEUE *queue, ALLEGRO_FONT* font);

void main(){
	//get directories
	PATHS paths;
	
	paths.home= getenv(home_var);
	
	int home_path_len = strlen(paths.home);
	
	char record_path[home_path_len + sizeof(local_dir sep_str "records")];
	strcpy(record_path, paths.home);
	strcat(record_path, local_dir sep_str "records");
	paths.record = record_path;
	
	char ghost_path[home_path_len + sizeof(local_dir sep_str "ghosts")];
	strcpy(ghost_path, paths.home);
	strcat(ghost_path, local_dir sep_str "ghosts");
	paths.ghost = ghost_path;

	al_change_directory(data_dir);
	paths.data = al_get_current_directory();

	/*true: In the previuous frame the mouse was down
	 *false:The mouse is up*/
	_Bool prev_mouse_down = false;
	//initialize and check for errors
	must_init(al_init(),"allegro");
	
	//Load configuration	
	char config_path[home_path_len+sizeof(local_dir sep_str "config.cfg")];
	strcpy(config_path, paths.home);

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

	//timers
	ALLEGRO_TIMER* timer = al_create_timer(1.0 / 10);
	must_init(timer,"timer");
	al_start_timer(timer);
	
	must_init(al_init_image_addon(), "image");

	ALLEGRO_EVENT_QUEUE* queue = al_create_event_queue();
	must_init(queue,"queue");

	al_register_event_source(queue, al_get_keyboard_event_source());
    	al_register_event_source(queue, al_get_timer_event_source(timer));


    	
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

	

	_Bool first = true;
	
	int screen_width;
	int screen_height;

	_Bool back_from_race = false;


	while(true){
		al_acknowledge_resize(disp);
		_Bool click = false;
		_Bool EndProgram=false;
		handle_display(&screen_width,&screen_height,first, disp,&event, queue, font);
		_Bool mouse_down;
		if(first|al_is_event_queue_empty(queue)){
			ALLEGRO_MOUSE_STATE mouse_state;
			al_get_mouse_state(&mouse_state);
			al_clear_to_color(al_map_rgb(0,0,0));
			mouse_down = (_Bool)(mouse_state.buttons&0x1);
			if(mouse_down && !prev_mouse_down)
				click =true;


			draw_text(config.font_name, "Zobrollo", al_map_rgb(255,255,255), 
					0.5*screen_width, 0.2*screen_height, 0.8*screen_width, 
					0.3*screen_height);
			draw_text(config.font_name, "v"version, al_map_rgb(255,255,255), 
					0.5*screen_width, 0.34*screen_height, 0.15*screen_width, 
					0.1*screen_height);
			if(handle_click_box_relative(mouse_state.x, mouse_state.y, 0.3,0.42,0.7,0.55,
						screen_width,screen_height,&config,"Time Trial")&&click){
				track_menu(&config, disp, &event, queue, data_dir sep_str "tracks",
						&paths,font,singleplayer_race);
			}
			if(handle_click_box_relative(mouse_state.x, mouse_state.y, 0.3,0.57,0.7,0.70,
						screen_width,screen_height,&config,"Multiplayer")&&click){
				multiplayer_menu(&config, disp, &paths, &event, queue, font);
			}
			if(handle_click_box_relative(mouse_state.x, mouse_state.y,0.3,0.72,0.49,0.85,
						screen_width, screen_height, &config, "Records")&&
					click){
				
				track_menu(&config, disp, &event, queue, data_dir sep_str "tracks",
						&paths, font, show_record);

			}
			if(handle_click_box_relative(mouse_state.x, mouse_state.y,0.51,0.72,0.7,0.85,
						screen_width, screen_height, &config, "Quit")&&
					click){
				exit(1);
			}
			al_flip_display();
		}
		al_wait_for_event(queue,&event);
		prev_mouse_down = mouse_down;
	}

	al_shutdown_font_addon();
}


//Every file in the specified directory is an item in this menu
void track_menu(CONFIG* config, ALLEGRO_DISPLAY* disp, ALLEGRO_EVENT* event,
		ALLEGRO_EVENT_QUEUE *queue, char* dir_path, PATHS* paths,ALLEGRO_FONT* font,
		void (*click_func)(TRACK_DATA *track,char* filename,
			CONFIG* config, ALLEGRO_DISPLAY* disp, PATHS* paths,ALLEGRO_EVENT* event,
			ALLEGRO_EVENT_QUEUE *queue, ALLEGRO_FONT* font)){

	/*true: In the previuous frame the mouse was down
	 *false:The mouse is up*/
	_Bool prev_mouse_down = true;
	
	//get a list of file names
	ALLEGRO_FS_ENTRY *dir = al_create_fs_entry(dir_path);
	if(!al_open_directory(dir)){
		fprintf(stderr, "Could not open \"%s\"\n", dir_path);
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
	
	int screen_width; int screen_height;

	_Bool back_from_race = false;

	while(true){
		_Bool click = false;
		_Bool EndProgram=false;
		_Bool redraw = false;
		handle_display(&screen_width,&screen_height,first, disp,event, queue, font);
		if(event->type == ALLEGRO_EVENT_KEY_DOWN 
				&& event->keyboard.keycode == ALLEGRO_KEY_ESCAPE)
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

			al_draw_text(font, al_map_rgb(255,255,255), 0, 0, 0, "Zobrollo v" version);
			i=0;
			while(i<am_files){

				_Bool mouse_in_box = handle_click_box_relative(mouse_state.x,
						mouse_state.y, file_box[i].x1frac,
						file_box[i].y1frac,file_box[i].x2frac,
						file_box[i].y2frac, screen_width, 
						screen_height, config, " ");

				//draw contents of the box
				float box_width =file_box[i].x2frac-file_box[i].x1frac;
				float box_height =file_box[i].y2frac-file_box[i].y1frac;
				float track_x1=(box_width*0.1+file_box[i].x1frac)*screen_width;
				float track_y1=(box_height*0.1+file_box[i].y1frac)*screen_height;
				float track_x2=(box_width*0.9+file_box[i].x1frac)*screen_width;
				float track_y2=(box_height*0.6+file_box[i].y1frac)*screen_height;
				al_draw_rectangle(track_x1,track_y1,track_x2,track_y2,
						config->button_border_color, 
						config->button_border_thickness);
				al_draw_filled_rectangle(
						track_x1+config->button_border_thickness/2,
						track_y1+config->button_border_thickness/2,
						track_x2-config->button_border_thickness/2,
						track_y2-config->button_border_thickness/2,
						al_map_rgb(0,0,0));
						
				drawmap(track_x1,track_y1,track_x2,track_y2,
						NULL, tracks+i);
				draw_text(config->font_name, file_names[i], 
						config->button_text_color,
						(box_width*0.5+file_box[i].x1frac)*screen_width,
						(box_height*0.8+file_box[i].y1frac)*screen_height,
						box_width*0.8*screen_width,
						box_height*0.2*screen_height);
						
						


				if(mouse_in_box && click){

					click_func(tracks+i, file_names[i], config, disp, paths,
							event, queue, font);
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

void multiplayer_menu(CONFIG* config, ALLEGRO_DISPLAY* disp, PATHS* paths, ALLEGRO_EVENT* event,
		ALLEGRO_EVENT_QUEUE *queue, ALLEGRO_FONT* font){
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
		handle_display(&screen_width,&screen_height,first, disp,event, queue, font);
		if(event->type == ALLEGRO_EVENT_KEY_DOWN 
				&& event->keyboard.keycode == ALLEGRO_KEY_ESCAPE)
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
						0.45, screen_width,screen_height, config,
						"Join Server")&&click)
				input_box(config,disp,paths, event, queue, font);
				back=true;
			if(handle_click_box_relative(mouse_state.x, mouse_state.y,0.20,0.55,0.80,
						0.80, screen_width,screen_height, config,
						"Start Server")&&click){
				track_menu(config, disp, event, queue, data_dir sep_str "tracks",
					paths, font, start_server);
				back=true;
			}

			al_draw_text(font, al_map_rgb(255,255,255), 0, 0, 0, "Zobrollo v" version);
			al_flip_display();
		}
		first=back;
		back=false;
		prev_mouse_down = mouse_down;
		al_wait_for_event(queue,event);
	}
}

void start_server(TRACK_DATA *track,char* filename,CONFIG* config, ALLEGRO_DISPLAY* disp, 
		PATHS* paths,ALLEGRO_EVENT* event,ALLEGRO_EVENT_QUEUE *queue, ALLEGRO_FONT* font){
	int server_socket , new_socket;
	struct sockaddr_in server , client;
	int max_sd;
	int c;
#ifdef __WIN32
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2,2),&wsa) != 0)
	{
		printf("Failed. Error Code : %d",WSAGetLastError());
		return;
	}
	
#else
	//Otherwise a SIGPIPE signal would crash the program
	sigaction(SIGPIPE, &(struct sigaction){SIG_IGN}, NULL);
#endif
	//Create a socket
	if((server_socket = socket(AF_INET , SOCK_STREAM , 0 )) == -1)
	{
		error_message("creating socket");
		return;
	}
	max_sd = server_socket;
	
	//Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons( 8888 );
	
	//Bind
	if( bind(server_socket ,(struct sockaddr *)&server , sizeof(server)) == -1)
	{
		error_message("To bind");
	}
	

	//Listen to incoming connections
	listen(server_socket , 3);
	
	node_t sockets_head;
	sockets_head.value = server_socket;
	sockets_head.next = NULL;
	
	server_race(server_socket, max_sd, &sockets_head,
			track,filename,config,disp,paths,event,
			queue, font);
	close(server_socket);
#ifdef __WIN32
	WSACleanup();
#endif
}



// vim: cc=100
