#include <allegro5/allegro5.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>

#include <stdio.h>
#include <math.h>

#include "global.h"
#include "config.h"
#include "drawtrack.h"
#include "misc.h"
#include "ghost.h"
#include "kart.h"
#include "drawframe.h"

void play_ghost(ALLEGRO_FS_ENTRY *ghost_file_entry, ALLEGRO_FS_ENTRY *track_entry){
	//load the ghost file
	ALLEGRO_FILE* ghost_file = al_open_fs_entry(ghost_file_entry, "rb");
	
	int frames;
       	al_fread(ghost_file, &frames, sizeof(frames));

	float fps;
       	al_fread(ghost_file, &fps, sizeof(fps));
	ALLEGRO_TIMER* timer = al_create_timer(1.0 / config.fps);
	must_init(timer,"timer");
	al_start_timer(timer);


	float ghost_buf[frames*3];
       	al_fread(ghost_file, ghost_buf, sizeof(ghost_buf));

	//load the track file
	TRACK_DATA track;
	ALLEGRO_FILE* track_file= al_open_fs_entry(track_entry, "r");
	if(!track_file){
		fprintf(stderr, "Could not open track file\n");
		return;
	}


	loadtrack(track_file, &track);
	al_fclose(track_file);

	
	int screen_width = al_get_display_width(disp);
	int screen_height = al_get_display_height(disp);
	
	ALLEGRO_EVENT_QUEUE* queue = al_create_event_queue();
	must_init(queue,"queue");
    	al_register_event_source(queue, al_get_timer_event_source(timer));
	al_register_event_source(queue, al_get_keyboard_event_source());
	al_register_event_source(queue, al_get_display_event_source(disp));
	ALLEGRO_EVENT event;

	_Bool quit = false;
	float scale = 1;
	
	#define KEY_SEEN     1
	#define KEY_RELEASED 2

	unsigned char key[ALLEGRO_KEY_MAX];
	memset(key, 0, sizeof(key));

	int i =0;
	int segment = 0;
	while(i<frames){
		al_clear_to_color(al_map_rgb(0,0,0));
		al_wait_for_event(queue,&event);
		switch(event.type){
			case(ALLEGRO_EVENT_DISPLAY_RESIZE):
				al_acknowledge_resize(disp);
				screen_height = al_get_display_height(disp);
				screen_width  = al_get_display_width(disp);
				break;
			case(ALLEGRO_EVENT_TIMER):
				if(key[ALLEGRO_KEY_EQUALS]){
					scale*=pow(2,1/config.fps);
				}
				if(key[ALLEGRO_KEY_MINUS]){
					scale*=pow(2,-1/config.fps);
				}

				if(key[ALLEGRO_KEY_ESCAPE]){
					quit = true;
				}
				int i = 0; 
				while(i < ALLEGRO_KEY_MAX){
			    		key[i] &= KEY_SEEN;
					i++;
				}
				break;
			case(ALLEGRO_EVENT_KEY_DOWN):
				key[event.keyboard.keycode] = KEY_SEEN | KEY_RELEASED;
				if(key[ALLEGRO_KEY_F11]){
					//Toggle full screen
					al_set_display_flag(disp, ALLEGRO_FULLSCREEN_WINDOW, 
							!(_Bool)(al_get_display_flags(disp) & 
							ALLEGRO_FULLSCREEN_WINDOW));
					screen_height = al_get_display_height(disp);
					screen_width  = al_get_display_width(disp);

					key[ALLEGRO_KEY_F11] = 0;
				}
						
				break;
		    	case ALLEGRO_EVENT_KEY_UP:
				key[event.keyboard.keycode] &= KEY_RELEASED;
				break;
			case(ALLEGRO_EVENT_DISPLAY_CLOSE):
				exit(1);
				break;
		}
		float x = ghost_buf[i*3+1];
		float y = ghost_buf[i*3+2];
		float angle = ghost_buf[i*3];
		float track_angle;
		if(config.camera_angle==RELATIVE_TO_TRACK){
			segment = get_cur_segment(x, y, &track_angle, segment, &track);
		}
		kart_t kart = {ghost_buf[i*3],ghost_buf[i*3+1], ghost_buf[i*3+2], 
			0,config.kart_color};
		drawframe(1,0, &kart, scale, &track,track_angle);
		al_flip_display();
		if(quit)
			break;
		i++;
	}
}

// vim: cc=100
