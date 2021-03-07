#include <allegro5/allegro5.h>
#include <allegro5/allegro_font.h>
#include <stdlib.h>
#include <stdio.h>

#include "file_paths.h"
#include "config.h"
#include "record.h"
#include "misc.h"
#include "gui.h"
#include "ghost.h"

char * TimeToString(double  secs){
	int milsecs  = (int)(secs*1000);
	char *ret=malloc(sizeof(char)*20);
	sprintf(ret, "%02d:%02d.%03d", (int)(secs/60), ((int)secs)%60, milsecs%1000);
	return ret;
}

//loads and sorts records from a file
//returns the amount of records
int load_record(ALLEGRO_FILE* file, record** records, 
		_Bool ret_date /*when false will read only time not date*/){
	int record_list_len = 10;
	int am_records = 0;
	*records=malloc(sizeof(record)*record_list_len);
	char record_file_text[20];
	char *date_string = malloc(sizeof(char)*30);
	while(true){

		add_element((void**)records, &am_records, &record_list_len, sizeof(record));
		al_fgets(file, date_string, 30);
		if(!al_fgets(file,record_file_text,20)){
			am_records--;
			break;
		}
		(*records)[am_records-1].time = atof(record_file_text);
		if(ret_date){
			//remove \n
			date_string[strlen(date_string)-1]='\0';

			(*records)[am_records-1].date= date_string;
			date_string = malloc(sizeof(char)*30);
		}
	}
	if(!ret_date)
		free(date_string);

	if(am_records>0)
		qsort((void*)*records, am_records, sizeof(record), compare_record);
	return(am_records);
}


void show_record(ALLEGRO_FS_ENTRY *record_file_entry, char* filename, CONFIG* config,
		ALLEGRO_DISPLAY* disp, PATHS* paths){
	
	ALLEGRO_EVENT_QUEUE* queue = al_create_event_queue();
	must_init(queue,"queue");
	ALLEGRO_EVENT event;
	must_init(al_install_mouse(), "mouse");
	must_init(al_install_keyboard(), "mouse");
    	al_register_event_source(queue, al_get_mouse_event_source());
    	al_register_event_source(queue, al_get_display_event_source(disp));
	al_register_event_source(queue, al_get_keyboard_event_source());

	/*true: In the previuous frame the mouse was down
	 *false:The mouse is up*/
	_Bool prev_mouse_down = true;

	ALLEGRO_FILE* file = al_open_fs_entry(record_file_entry, "r");
	record* records;
	int am_records = load_record(file, &records, true);

	_Bool has_replay_file[am_records];
	int i =0;
	char ghost_filename[25];
	while(i<am_records){
		al_change_directory(paths->ghost);
		al_change_directory(filename);
		strcpy(ghost_filename, records[i].date);
		strcat(ghost_filename, ".bin");
		has_replay_file[i] = al_filename_exists(ghost_filename);
		i++;
	}

	int loopcount = 0;

	while(true){
		al_acknowledge_resize(disp);
		al_clear_to_color(al_map_rgb(0,0,0));
		_Bool mouse_down;
		_Bool click = false;

		ALLEGRO_MOUSE_STATE mouse_state;
		al_get_mouse_state(&mouse_state);
		mouse_down = (_Bool)(mouse_state.buttons&0x1);
		if(mouse_down && !prev_mouse_down){
			click =true;
		}
		switch(event.type){

			case(ALLEGRO_EVENT_KEY_DOWN):
				switch(event.keyboard.keycode){
					case(ALLEGRO_KEY_F11):
						al_set_display_flag(disp, ALLEGRO_FULLSCREEN_WINDOW, 
							!(_Bool)(al_get_display_flags(disp) & 
								 ALLEGRO_FULLSCREEN_WINDOW));
						break;
					case(ALLEGRO_KEY_ESCAPE):
						al_change_directory(data_dir);
						return;
				}
				break;
			
			case(ALLEGRO_EVENT_DISPLAY_CLOSE):
				exit(1);
		}
		al_clear_to_color(al_map_rgb(0,0,0));
		i =0;
		ALLEGRO_FONT* font = al_create_builtin_font();
		_Bool back_from_race=false;
		while(i<am_records){
			al_draw_textf(font, al_map_rgb(255,255,255),0,(i)*30+10, 
					0,"%s  %s", records[i].date,TimeToString(records[i].time));

			if(has_replay_file[i]){
				al_change_directory(paths->data);
				if(handle_click_box(mouse_state.x, mouse_state.y, 250, i*30, 
						350, (i+1)*30, config, "Replay")&&click){

					al_change_directory(paths->ghost);
					al_change_directory(filename);
					strcpy(ghost_filename, records[i].date);
					strcat(ghost_filename, ".bin");
					ALLEGRO_FS_ENTRY * ghost_entry = 
						al_create_fs_entry(ghost_filename);
					al_change_directory(paths->data);
					al_change_directory("tracks");
					ALLEGRO_FS_ENTRY * track_entry =
						al_create_fs_entry(filename);
					al_change_directory("..");
					play_ghost(ghost_entry, track_entry,
							config, disp);
					al_flush_event_queue(queue);
					back_from_race=true;
				}
			}
			i++;
		}
		prev_mouse_down = mouse_down;
		al_flip_display();
		loopcount++;
		al_destroy_font(font);
		if(!back_from_race)
			al_wait_for_event(queue,&event);
	}
}

	
int compare_record(const void* a, const void* b){//Function used by sort function
	float x = ((record*)a)->time;
	float y = ((record*)b)->time;
	return x<=y ? (x<y?-1 : 0) : 1;
}
//  vim: cc=100	
