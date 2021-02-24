#include <allegro5/allegro5.h>
#include <allegro5/allegro_font.h>
#include <stdlib.h>
#include <stdio.h>

#include "config.h"
#include "record.h"
#include "misc.h"

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
		ALLEGRO_DISPLAY* disp){
	
	ALLEGRO_EVENT_QUEUE* queue = al_create_event_queue();
	must_init(queue,"queue");
	ALLEGRO_EVENT event;
    	al_register_event_source(queue, al_get_display_event_source(disp));
	al_register_event_source(queue, al_get_keyboard_event_source());

	ALLEGRO_FILE* file = al_open_fs_entry(record_file_entry, "r");
	record* records;
	int am_records = load_record(file, &records, true);
	while(true){
		al_acknowledge_resize(disp);
		if(event.type == ALLEGRO_EVENT_KEY_DOWN){
			switch(event.keyboard.keycode){
				case(ALLEGRO_KEY_F11):
					al_set_display_flag(disp, ALLEGRO_FULLSCREEN_WINDOW, 
						!(_Bool)(al_get_display_flags(disp) & 
							 ALLEGRO_FULLSCREEN_WINDOW));
					break;
				case(ALLEGRO_KEY_ESCAPE):
					return;
			}
		}
		if(event.type == ALLEGRO_EVENT_DISPLAY_CLOSE){
			exit(1);
		}
		al_clear_to_color(al_map_rgb(0,0,0));
		int i =0;
		ALLEGRO_FONT* font = al_create_builtin_font();
		while(i<am_records){
			al_draw_textf(al_create_builtin_font(), al_map_rgb(255,255,255),0,(i+1)*10, 
					0,"%s  %s", records[i].date,TimeToString(records[i].time));
			i++;
		}
		al_flip_display();
		al_wait_for_event(queue,&event);
	}
}

	
int compare_record(const void* a, const void* b){//Function used by sort function
	float x = ((record*)a)->time;
	float y = ((record*)b)->time;
	return x<=y ? (x<y?-1 : 0) : 1;
}
//  vim: cc=100	
