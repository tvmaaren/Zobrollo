/*
Zobrollo is a 2d minimalistic top-view racing game
Copyright (C) 2021  Thomas van Maaren

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
any later version.
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.

e-mail:thomas.v.maaren@outlook.com
*/




#include <allegro5/allegro5.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_image.h>

#include <math.h>
#include <stdio.h>
#include <time.h>

#include "drawtrack.h"
#include "config.h"
#include "kart.h"
#include "file_paths.h"
#include "race.h"
#include "misc.h"
#include "record.h"
#include "drawframe.h"

//the different modes of play
#define COUNTDOWN 0
#define PLAYING 1
#define END 2


char * SecToString(double  secs);//Easily display time
int inc_circ_count(int i, int max);//Make it loop round forwards
int dec_circ_count(int i, int max);//Make it loop round backwards
int GetCurSegment(float x, float y, float* track_angle, int cur_segment, TRACK_DATA* track_data);
float InInterval(float a);//Have angle between -pi and pi
void SaveRace(float *buf, int am_frames, FILE *save_file, float fps);//Saves the race in a file

void race(ALLEGRO_FS_ENTRY *track_file_entry, char* filename, CONFIG* config, 
		ALLEGRO_DISPLAY* disp, PATHS *paths){

	
	//timers
	ALLEGRO_TIMER* timer = al_create_timer(1.0 / config->fps);
	must_init(timer,"timer");
	ALLEGRO_TIMER* countdown = al_create_timer(1);
	must_init(countdown,"countdown");

	
	ALLEGRO_EVENT_QUEUE* queue = al_create_event_queue();
	must_init(queue,"queue");

	ALLEGRO_FONT* font = al_create_builtin_font();

	must_init(font,"couldn't initialize font\n");

	ALLEGRO_FONT* splash = al_load_ttf_font(config->font_name, 20, ALLEGRO_TTF_MONOCHROME);

	ALLEGRO_EVENT event;
	al_start_timer(timer);
	al_start_timer(countdown);

	al_register_event_source(queue, al_get_keyboard_event_source());
    	al_register_event_source(queue, al_get_display_event_source(disp));
    	al_register_event_source(queue, al_get_timer_event_source(timer));

	printf("Dir when starting race: %s\n", al_get_current_directory());
	ALLEGRO_BITMAP* full_heart = al_load_bitmap("full heart.png");
	ALLEGRO_BITMAP* half_heart = al_load_bitmap("half heart.png");

	#define KEY_SEEN     1
	#define KEY_RELEASED 2

	unsigned char key[ALLEGRO_KEY_MAX];
	memset(key, 0, sizeof(key));
	
	TRACK_DATA track;
	
	ALLEGRO_FILE* track_file = al_open_fs_entry(track_file_entry, "r");
	if(!track_file){
		fprintf(stderr, "Could not open track file\n");
		return;
	}


	loadtrack(track_file, &track);
	al_fclose(track_file);

	int frame = 0;
	int ghost_buf_len = 10;
	float* ghost_buf = malloc(sizeof(float)*ghost_buf_len);


	if(!al_make_directory(paths->record)){
		fprintf(stderr, "Error: Could not create \"%s\"\n",paths->record);
		exit(1);
	}
	if(!al_change_directory(paths->record)){
		fprintf(stderr, "Error: Could not open \"%s\"\n",paths->record);
		exit(1);
	}

	
	ALLEGRO_FILE* record_file =al_fopen(filename,"r+");
	if(!record_file){
	    //file does not exist yet
	    record_file = al_fopen(filename , "w+");
	    if(!record_file){
		    fprintf(stderr, "Error: Could not create \"%s\"\n", filename);
	    }
	}

	al_change_directory(paths->data);

	record *records;
	int am_records = load_record(record_file, &records, true);

	//load ghost of the record
	int frames_record;
	float fps_record;
	float* record_ghost_buf;
	ALLEGRO_FILE* ghost_record_file;

	int n_karts = 1;
	if(config->play_against_ghost){
		_Bool found_file=false;
		al_change_directory(paths->ghost);
		al_change_directory(filename);
		int i =0;
		while(i<am_records){
			char ghost_record_filename[strlen(records[i].date)+sizeof(".bin")];
			strcpy(ghost_record_filename, records[i].date);
			strcat(ghost_record_filename, ".bin");
			
			ghost_record_file = al_fopen(ghost_record_filename, "r");
			if(ghost_record_file){
				found_file = true;
				break;
			}
			i++;
		}
		if(found_file){
			al_fread(ghost_record_file, &frames_record, sizeof(frames_record));
			al_fread(ghost_record_file, &fps_record, sizeof(fps_record));
			record_ghost_buf = malloc(sizeof(float)*frames_record*3);
			al_fread(ghost_record_file, record_ghost_buf, 3*sizeof(float)*frames_record);
			al_fclose(ghost_record_file);
			n_karts =2;
		}else{
			config->play_against_ghost=false;
		}
		al_change_directory(paths->data);
	}
	kart_t karts[n_karts];

	float v=0;
	karts[0].angle=M_PI/2;
	float min_radius = sqrt(config->kart_height*config->kart_height/4+
		pow(config->kart_height/tan(config->max_wheel_angle)+config->kart_width/2,2));
	float scale=1;
	float track_angle;
	float damage=0;

	//the position of the middle of the kart
	karts[0].x=0;
	karts[0].y=0;

	karts[0].color = config->kart_color;
	karts[1].color = config->kart_color;
	karts[1].color = al_map_rgb(config->kart_color_r/2,config->kart_color_g/2,
			config->kart_color_b/2);

	//keeps track in wich segment the kart is in.
	//negative means outside the track
	int cur_segment =0;

	int max_segment =cur_segment;//the highest segment it has fairly reached
	double stopwatch=0;
	int mode = COUNTDOWN;
	int lap = 1;

	int screen_width = al_get_display_width(disp);
	int screen_height = al_get_display_height(disp);

	double start_time;

	while(true){
		al_wait_for_event(queue,&event);

		_Bool EndProgram=false;
		_Bool redraw=false;
		
		int count_val;
		
		if(mode==COUNTDOWN){
			count_val = (int)(config->sec_before_start-al_get_timer_count(countdown));
			if( count_val<= 0){
				mode=PLAYING;
				start_time = al_get_time();
			}
		}
		switch(event.type){
			case(ALLEGRO_EVENT_DISPLAY_RESIZE): 
				al_acknowledge_resize(disp);
				screen_height = al_get_display_height(disp);
				screen_width  = al_get_display_width(disp);
				font = al_create_builtin_font();

				full_heart = al_load_bitmap("full heart.png");
				half_heart = al_load_bitmap("half heart.png");

				redraw=true;
				break;
			case(ALLEGRO_EVENT_TIMER):
				//due to friction the speed decreases every frame
				v=v/pow(config->speed_decrease, 1.0/(float)config->fps);
				if(key[ALLEGRO_KEY_UP]&&mode==PLAYING){
					if(v>=0)v+=config->accelleration/config->fps;
					if(v<0)v+=config->break_speed/config->fps;
				}
				if(key[ALLEGRO_KEY_DOWN]&&mode==PLAYING){
					if(v>0)v-=config->break_speed/config->fps;
					if(v<=0)v-=config->back_accelleration/config->fps;
				}
				
				
				float radius = config->mass*v*v/config->max_F;
				if(key[ALLEGRO_KEY_LEFT]&&mode==PLAYING){
					if(radius<min_radius)radius=min_radius;
					karts[0].angle-=v/radius/config->fps;
				}
				if(key[ALLEGRO_KEY_RIGHT]&&mode==PLAYING){
					if(radius<min_radius)radius=min_radius;
					karts[0].angle+=v/radius/config->fps;
				}
				if(key[ALLEGRO_KEY_EQUALS]){
					scale*=pow(2,1/config->fps);
				}
				if(key[ALLEGRO_KEY_MINUS]){
					scale*=pow(2,-1/config->fps);
				}

				if(key[ALLEGRO_KEY_ESCAPE]){
					EndProgram = true;
				}


				int i = 0;
				while(i < ALLEGRO_KEY_MAX){
			    		key[i] &= KEY_SEEN;
					i++;
				}

				float new_x_pos=karts[0].x+cos(karts[0].angle)*v/config->fps;
				float new_y_pos=karts[0].y+sin(karts[0].angle)*v/config->fps;

				int new_cur_segment=get_cur_segment(new_x_pos, new_y_pos,
						&track_angle, cur_segment, &track);
				if(new_cur_segment!=-1){
					
					if(mode==PLAYING&&max_segment==track.n_segments-1 && 
							cur_segment==track.n_segments-1 && 
							new_cur_segment==0){
						lap++;
						if(lap>config->laps){
							mode=END;

							//store the time the race was made in the 
							//record file
							time_t pSec = time(NULL);
							struct tm* local_time = localtime(&pSec);
							#define date_string_len 18
							char date_string[date_string_len];
							sprintf(date_string,
									"%d-%02d-%02d "
									"%02d%02d%02d\n", 
									local_time->tm_year+1900,
									local_time->tm_mon+1,
									local_time->tm_mday,
									local_time->tm_hour,
									local_time->tm_min,
									local_time->tm_sec);;
							al_fputs(record_file,date_string);
							char record_file_text[20];

							sprintf(record_file_text,"%f\n",stopwatch);
							al_fputs(record_file, record_file_text);
							al_fclose(record_file);

							//Store the ghost in a bin file at
							//local_dir/ghosts/%trackname%/%time%.bin
							if(config->save_ghost){

								//remove \n
								date_string[date_string_len-1]=
									'\0';
								al_change_directory(paths->ghost);
								
								char ghost_filename[
									strlen(filename)+1+
									date_string_len+
									sizeof(".bin")];
								strcpy(ghost_filename, filename);
								strcat(ghost_filename, sep_str);
								if(!al_make_directory(
										ghost_filename)){
									fprintf(stderr, 
										"Error: Could not"
										" create "
										"\"%s\"\n",
										ghost_filename);
									exit(1);
								}
								strcat(ghost_filename,date_string);
								strcat(ghost_filename, ".bin");
								ALLEGRO_FILE* ghost_file = 
									al_fopen(ghost_filename,
									"wb");
								if(ghost_file){
									al_fwrite(ghost_file, 
										&frame,sizeof(int));
									al_fwrite(ghost_file,
										&(config->fps),
										sizeof(float));
									al_fwrite(ghost_file, 
										ghost_buf, 3*frame*
										sizeof(float));
									al_fclose(ghost_file);
								}else{
									printf("Could not make"
										" ghost file\n");
								}
								al_change_directory(paths->data);
							}
							int frame_i = 0;
							
							
						}
						max_segment = 0;
					}
					else if(new_cur_segment-1==max_segment){
						max_segment=new_cur_segment;
					}
					cur_segment=new_cur_segment;
					karts[0].x=new_x_pos;
					karts[0].y=new_y_pos;
				}else{
					float angle_kart_track=InInterval(track_angle-karts[0].angle);
					float v_new;
					if(angle_kart_track>M_PI/2 || angle_kart_track<-M_PI/2){
						karts[0].angle = 2*track_angle-karts[0].angle;
						v_new=cos(angle_kart_track-M_PI)*v/4;
					}else{
						v_new=cos(angle_kart_track)*v/4;
						karts[0].angle = 2*track_angle-karts[0].angle;
					}
					damage+=abs(v-v_new);
					if(mode==PLAYING && damage>=config->death_crash){

						stopwatch=-1;
						mode=END;
					}
					v=v_new;
				}


				redraw=true;
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
					font = al_create_builtin_font();

					ALLEGRO_BITMAP* full_heart = 
						al_load_bitmap(data_dir sep_str"full heart.png");
					ALLEGRO_BITMAP* half_heart = 
						al_load_bitmap(data_dir sep_str"half heart.png");

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
		if(EndProgram)break;
		
		if(redraw && al_is_event_queue_empty(queue)){
			if(mode==PLAYING){
				/*add_element((void**)&ghost_buf, &frame, &ghost_buf_len, 
						sizeof(float)*2);*/
				frame+=1;
				ghost_buf = realloc(ghost_buf,3*sizeof(float)*frame);
				ghost_buf[(frame*3-3)] = karts[0].angle;
				ghost_buf[(frame*3-2)] = karts[0].x;
				ghost_buf[(frame*3-1)] = karts[0].y;
				stopwatch=al_get_time()-start_time;
			}
			al_clear_to_color(al_map_rgb(0,0,0));
			if(config->play_against_ghost){
				karts[1].angle = record_ghost_buf[frame*3];
				karts[1].x=record_ghost_buf[frame*3+1];
				karts[1].y=record_ghost_buf[frame*3+2];
				n_karts=2;
			}
			drawframe(n_karts, karts, scale,screen_width,screen_height,&track, track_angle, 
					config);
			//Draw hearts
			if(config->show_hearts){
				float heart_width = 10;
				float heart_height = 10;
				float heart_gap = 2;
				int am_hearts = 
					(int)ceil(
						(config->death_crash-damage)
						/config->life_per_heart
					);

				int heart_i=0;
				while(heart_i<am_hearts){
					al_draw_scaled_bitmap(full_heart, 0, 0, 
						al_get_bitmap_width(full_heart), 
						al_get_bitmap_height(full_heart),
						screen_width-heart_width*(heart_i+1)-heart_gap*heart_i,
						0,heart_width,heart_height,0);
					heart_i++;
				}
			}
			
			if(mode==COUNTDOWN){
				char text[10];
				sprintf(text, "%d", count_val);
				al_draw_text(splash, al_map_rgb(255, 255, 255), screen_width/2-
						al_get_text_width(splash,text)/2, 
					screen_height/2-al_get_font_ascent(splash)/2,0, text);
			}
				
			

			char infotext[200];
			infotext[0]='\0';
			if(config->show_speed){
				char speedtext[20];
				sprintf(speedtext, "speed=%f, ", v);
				strcat(infotext, speedtext);
			}
			if(config->show_kart_angle){
				char angletext[20];
				sprintf(angletext, "angle=%f, ", karts[0].angle);
				strcat(infotext, angletext);
			}
			if(config->show_scale){
				char scaletext[20];
				sprintf(scaletext, "scale=%f, ", scale);
				strcat(infotext, scaletext);
			}
			if(config->show_track_angle){
				char track_angletext[20];
				sprintf(track_angletext, "angle=%f, ", track_angle);
				strcat(infotext, track_angletext);
			}
			if(config->show_damage){
				char damagetext[20];
				sprintf(damagetext, "damage=%f, ", damage);
				strcat(infotext, damagetext);
			}
			if(config->show_x_pos){
				char x_postext[20];
				sprintf(x_postext, "xpos=%f, ", karts[0].x);
				strcat(infotext, x_postext);
			}
			if(config->show_y_pos){
				char y_postext[20];
				sprintf(y_postext, "ypos=%f, ", karts[0].y);
				strcat(infotext, y_postext);
			}
			if(config->show_segment){
				char segmenttext[20];
				sprintf(segmenttext, "segment=%d, ", cur_segment);
				strcat(infotext, segmenttext);
			}
			if(config->show_stopwatch){
				char timetext[20];
				sprintf(timetext, "time=%s, ", SecToString(stopwatch));
				strcat(infotext, timetext);
			}
			if(config->show_lap){
				char lap_text[20];
				sprintf(lap_text, "lap=%d, ", lap);
				strcat(infotext, lap_text);
			}
			if(config->show_record &&am_records>0){
				char recordtext[20];
				sprintf(recordtext, "record=%s, ", 
						SecToString(records[0].time));
				strcat(infotext, recordtext);
			}

			al_draw_text(font, al_map_rgb(255, 255, 255), 0, 0, 0, infotext);

			if(mode==END){
				char* complete_text;
				if(stopwatch==-1)complete_text="YOU CRASHED!";
				else
					complete_text=SecToString(stopwatch);

				al_draw_text(splash, al_map_rgb(255, 255, 255), screen_width/2-
						al_get_text_width(splash,complete_text)/2, 
						screen_height/2-al_get_font_ascent(splash)/2,0, 
						complete_text);
			}
			al_flip_display();

		}
	}
	al_destroy_bitmap(half_heart);
	al_destroy_bitmap(full_heart);

	al_destroy_event_queue(queue);
	
	al_destroy_timer(countdown);
	al_destroy_timer(timer);

	al_destroy_font(splash);
	al_destroy_font(font);

	free(records);
	free(record_ghost_buf);
	
}


char * SecToString(double  secs){
	int milsecs  = (int)(secs*1000);
	char *ret=malloc(sizeof(char)*20);
	sprintf(ret, "%02d:%02d.%03d", (int)(secs/60), ((int)secs)%60, milsecs%1000);
	return ret;
}

// vim: cc=100
