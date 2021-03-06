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
#include "kart.h"
#include "config.h"
#include "file_paths.h"
#include "race.h"
#include "misc.h"
#include "record.h"

//the different modes of play
#define COUNTDOWN 0
#define PLAYING 1
#define END 2


char * SecToString(double  secs);//Easily display time
int inc_circ_count(int i, int max);//Make it loop round forwards
int dec_circ_count(int i, int max);//Make it loop round backwards
int GetCurSegment(float x, float y, float* track_angle, int cur_segment, TRACK_DATA track_data);
int PointAndLine(float x, float y, float x1, float y1, float x2, float y2);
float InInterval(float a);//Have angle between -pi and pi
void SaveRace(float *buf, int am_frames, FILE *save_file, float fps);//Saves the race in a file

void race(ALLEGRO_FS_ENTRY *track_file_entry, char* filename, CONFIG* config, 
		ALLEGRO_DISPLAY* disp, PATHS *paths){

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

	al_change_directory(data_dir);

	record *records;
	int am_records = load_record(record_file, &records, false);
	
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

	ALLEGRO_BITMAP* full_heart = al_load_bitmap(data_dir sep_str "full heart.png");
	ALLEGRO_BITMAP* half_heart = al_load_bitmap(data_dir sep_str "half heart.png");

	#define KEY_SEEN     1
	#define KEY_RELEASED 2

	unsigned char key[ALLEGRO_KEY_MAX];
	memset(key, 0, sizeof(key));

	float v=0;
	float angle=M_PI/2;
	float min_radius = sqrt(config->kart_height*config->kart_height/4+
		pow(config->kart_height/tan(config->max_wheel_angle)+config->kart_width/2,2));
	float scale=1;
	float track_angle;
	float damage=0;

	//the position of the middle of the kart
	float x_pos=0;
	float y_pos=0;
	
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

				full_heart = al_load_bitmap(data_dir sep_str"full heart.png");
				half_heart = al_load_bitmap(data_dir sep_str"half heart.png");

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
					angle-=v/radius/config->fps;
				}
				if(key[ALLEGRO_KEY_RIGHT]&&mode==PLAYING){
					if(radius<min_radius)radius=min_radius;
					angle+=v/radius/config->fps;
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

				float new_x_pos=x_pos+cos(angle)*v/config->fps;
				float new_y_pos=y_pos+sin(angle)*v/config->fps;

				int new_cur_segment=GetCurSegment(new_x_pos, new_y_pos, 
						&track_angle, cur_segment, track);
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

							sprintf(record_file_text, "%f\n", stopwatch);
							al_fputs(record_file, record_file_text);
							al_fclose(record_file);

							//Store the ghost in a bin file at
							//local_dir/ghosts/%trackname%/%time%.bin

							//remove \n
							date_string[date_string_len-1]='\0';
							
							char ghost_filename[sizeof(local_dir) + 1+
								sizeof("ghosts")+1+strlen(filename)+
								1+date_string_len+sizeof(".bin")];
							strcpy(ghost_filename, paths->home);
							strcat(ghost_filename, local_dir sep_str
									"ghosts" sep_str);
							strcat(ghost_filename, filename);
							strcat(ghost_filename, sep_str);
							if(!al_make_directory(ghost_filename)){
								fprintf(stderr, 
									"Error: Could not create "
									"\"%s\"\n",ghost_filename);
								exit(1);
							}
							strcat(ghost_filename, date_string);
							strcat(ghost_filename, ".bin");
							ALLEGRO_FILE* ghost_file = 
								al_fopen(ghost_filename, "wb");
							if(ghost_file){
								al_fwrite(ghost_file, &frame, 
										sizeof(int));
								al_fwrite(ghost_file,&(config->fps),
										sizeof(float));
								al_fwrite(ghost_file, ghost_buf, 
									3*frame*sizeof(float));
								al_fclose(ghost_file);
							}else{
								printf("Could not make ghost" 
										" file\n");
							}
							int frame_i = 0;
							
							
						}
						max_segment = 0;
					}
					else if(new_cur_segment-1==max_segment){
						max_segment=new_cur_segment;
					}
					cur_segment=new_cur_segment;
					x_pos=new_x_pos;
					y_pos=new_y_pos;
				}else{
					float angle_kart_track=InInterval(track_angle-angle);
					float v_new;
					if(angle_kart_track>M_PI/2 || angle_kart_track<-M_PI/2){
						angle = 2*track_angle-angle;
						v_new=cos(angle_kart_track-M_PI)*v/4;
					}else{
						v_new=cos(angle_kart_track)*v/4;
						angle = 2*track_angle-angle;
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
				ghost_buf[(frame*3-3)] = angle;
				ghost_buf[(frame*3-2)] = x_pos;
				ghost_buf[(frame*3-1)] = y_pos;
				stopwatch=al_get_time()-start_time;
			}
			al_clear_to_color(al_map_rgb(0,0,0));
			drawtrack(x_pos,y_pos, -track_angle-M_PI/2,scale,screen_width, 
					screen_height,&track);
			
			
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
			
			//draw player karts
			ALLEGRO_COLOR kart_color=al_map_rgb(config->kart_color_r,
					config->kart_color_g,config->kart_color_b);
			kart_t main_kart = {angle, x_pos, y_pos, config->kart_width, 
				config->kart_height, kart_color};
			drawkart(x_pos, y_pos,-track_angle-M_PI/2, scale, screen_width, 
					screen_height,  main_kart);

			//display the map
			if(config->show_map){
				float map_max_height=screen_height-10;
				float map_max_width=screen_width/24;
				float map_x=screen_width-map_max_width/2-20;
				float map_y=screen_height/2;

				float midx=(track.max_x+track.min_x)/2;
				float midy=(track.max_y+track.min_y)/2;
				float map_scale;
				map_scale=map_max_width/(track.max_x-track.min_x);
				if(map_max_height/(track.max_y - track.min_y)<map_scale)map_scale=
					map_max_height/(track.max_y-track.min_y);

				float map_c_x=midx-(map_x-screen_width/2)/map_scale;
				float map_c_y=midy-(map_y-screen_height/2)/map_scale;

				drawtrack(map_c_x, map_c_y, 0, map_scale, screen_width, 
						screen_height, &track);
				//draw the dot that represents the player
				
				ALLEGRO_TRANSFORM transform;
				al_identity_transform(&transform);
				al_translate_transform(&transform, -map_c_x, -map_c_y );
				al_scale_transform(&transform, map_scale,map_scale);
				al_translate_transform(&transform, screen_width/2,screen_height/2);
				
				
				float dotx=x_pos;
				float doty=y_pos;
				al_transform_coordinates(&transform, &dotx, &doty);

				al_draw_filled_circle(dotx, doty, 3, kart_color);
				al_identity_transform(&transform);
				al_use_transform(&transform);

					

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
				sprintf(angletext, "angle=%f, ", angle);
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
				sprintf(x_postext, "xpos=%f, ", x_pos);
				strcat(infotext, x_postext);
			}
			if(config->show_y_pos){
				char y_postext[20];
				sprintf(y_postext, "ypos=%f, ", y_pos);
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

	free(records);
	
}


char * SecToString(double  secs){
	int milsecs  = (int)(secs*1000);
	char *ret=malloc(sizeof(char)*20);
	sprintf(ret, "%02d:%02d.%03d", (int)(secs/60), ((int)secs)%60, milsecs%1000);
	return ret;
}





const char* GetConfVal(ALLEGRO_CONFIG* cfg, char* key){
	const char * value = al_get_config_value(cfg, "", key);
	if(value==NULL){
		fprintf(stderr, "%s is not a key in \"config.cfg\"\n", key);
		exit(1);
	}
	return value;
}

_Bool GetConfBool(ALLEGRO_CONFIG* cfg, char*key){
	const char * string= GetConfVal(cfg, key);
	if(!strcmp(string, "true"))return(1);
	else if(!strcmp(string, "false"))return(0);
	else{
		fprintf(stderr, "%s Should be true or false\n", key);
		exit(1);
	}
}




int inc_circ_count(int i, int max){
	i++;
	if(i>max)i=0;
	return i;
}
int dec_circ_count(int i, int max){
	i--;
	if(i<0)i=max;
	return i;
}

int GetCurSegment(float x, float y, float* track_angle, int cur_segment, TRACK_DATA track_data){
	if(cur_segment==-1){
		return -1;
	}
	int change=0;
	if(track_data.segment_types[cur_segment]){//circle
		CIRCLE_SEGMENT* segment = (CIRCLE_SEGMENT*)track_data.segments[cur_segment];
		float d_x = x-segment->midx;
		float d_y = y-segment->midy;
		float angle;
		float dist;
		cart2pol(d_x, d_y, &angle, &dist);
		
		angle =InInterval(angle);
		float seg_angle= InInterval(segment->start_angle);
		float end_angle= InInterval(segment->start_angle+segment->delta_angle);
		_Bool direction;

		//you can split the circle in 3 parts
		//	* -pi untill seg_angle or end_angle depending on which one
		//		has the smallest value: part=0
		//	* between seg_angle and end_angle: part=1
		//	* seg_angle or end_angle untill pi depending on which one
		//		has the greatest value: part=0
		
		//order=1 means that end_angle is greater than seg_angle
		//order=0 means that seg_angle is greater than end_angle
		
		float a, b;
		_Bool order;
		if(seg_angle>end_angle){
			a=end_angle;b=seg_angle;order=0;
		}else{b=end_angle;a=seg_angle;order=1;}
		_Bool part;
		if(angle<a)part=0;
		else if(angle>b)part=0;
		else part=1;



		//TODO: have it as a tertiary expression
		if(segment->delta_angle>0) direction=1;
	        else direction=0;
		
		if(direction)*track_angle=angle+M_PI/2;
		else *track_angle=angle-M_PI/2;

		if(!(order^direction^part)){
			//it is of the segment, but still on track
			//now see which way it went

			//TODO: have it as a tertiary expression
			if(fabs(InInterval(seg_angle-angle))>fabs(InInterval(end_angle-angle))){
				cur_segment = inc_circ_count(cur_segment, track_data.n_segments-1);
				change=1;
			}else{
				cur_segment = dec_circ_count(cur_segment, track_data.n_segments-1);
				change=1;
			}
		}

		if(change==0 && (dist>segment->r_outer||dist<segment->r_inner)){
			(cur_segment)=-1;
			change=1;
		}

	}else{//line
		LINE_SEGMENT* segment = (LINE_SEGMENT*)track_data.segments[cur_segment];
		//check if it is a horizontal rectangle
		if(abs(segment->outer.y1-segment->outer.y2)<1 || 
				abs(segment->outer.x1 - segment->inner.x1)<1){
			if(segment->outer.x1>segment->outer.x2)*track_angle=M_PI;
			else *track_angle=0;
			if(segment->outer.x1>segment->outer.x2)*track_angle=M_PI;
			if(segment->inner.x1<x ^ segment->inner.x2>segment->inner.x1){
				cur_segment = dec_circ_count(cur_segment, track_data.n_segments-1);
				change=1;
			}
			else if(segment->inner.x2<x ^ segment->inner.x2<segment->inner.x1){
				cur_segment = inc_circ_count(cur_segment, track_data.n_segments-1);
				change=1;
			}
			//off track
			else if(!(segment->outer.y1<y ^ segment->inner.y1<y)){
				(cur_segment)=-1;
				change=1;
			}
		}
		//check if it is a vertical rectangle
		else if(abs(segment->outer.y1-segment->inner.y1)<1 || 
				abs(segment->outer.x1 - segment->outer.x2)<1){
			if(segment->outer.y2>segment->outer.y1)*track_angle=M_PI/2;
			else *track_angle=-M_PI/2;
			if(segment->inner.y1<y ^ segment->inner.y2>segment->inner.y1){
				cur_segment = dec_circ_count(cur_segment, track_data.n_segments-1);
				change=1;
			}
			else if(segment->inner.y2<y ^ segment->inner.y2<segment->inner.y1){
				cur_segment = inc_circ_count(cur_segment, track_data.n_segments-1);
				change=1;
			}
			//off track
			else if(!(segment->outer.x1<x ^ segment->inner.x1<x)){
				(cur_segment)=-1;
				change=1;
			}
		
			

		}else{
			float d_y = segment->inner.y2-segment->inner.y1;
			float d_x = segment->inner.x2-segment->inner.x1;
			*track_angle = atan(d_y/d_x);
			if(d_x<0)*track_angle+=M_PI;
			//check if it is outside of the track, but after its endings.
			if(PointAndLine(segment->inner.x1,segment->inner.y1,segment->inner.x2,
							segment->inner.y2, segment->outer.x2, 
							segment->outer.y2)==2){
				//y2<y1
				if(PointAndLine(x,y,segment->inner.x1,segment->inner.y1,
							segment->outer.x1
							,segment->outer.y1)==2){
					cur_segment = dec_circ_count(cur_segment, 
							track_data.n_segments-1);
					change=1;
				}
				else if(PointAndLine(x,y,segment->inner.x2,
							segment->inner.y2,segment->outer.x2
							,segment->outer.y2)==0){
					cur_segment = inc_circ_count(cur_segment, 
							track_data.n_segments-1);
					change=1;
				}
			}else{
				//y2>y1
				if(PointAndLine(x,y,segment->inner.x1,segment->inner.y1,
							segment->outer.x1
							,segment->outer.y1)==0){
					cur_segment = dec_circ_count(cur_segment, 
							track_data.n_segments-1);
					change=1;
				}
				else if(PointAndLine(x,y,segment->outer.x2,segment->outer.y2,
							segment->inner.x2
							,segment->inner.y2)==2){
					cur_segment = inc_circ_count(cur_segment, 
							track_data.n_segments-1);
					change=1;
				}
			}
		}

		//check if it is outside of the track sideways
		if(change==0 && (PointAndLine(x, y, segment->outer.x1,segment->outer.y1,
				segment->outer.x2,segment->outer.y2)
				==PointAndLine(x, y, segment->inner.x1,segment->inner.y1,
				segment->inner.x2,segment->inner.y2))){
			cur_segment=-1;
			change=1;
		}
	}

	//if the segment has changed it has to check if it is in a valid position in the
	//new segment.
	if(change==1){
		cur_segment=GetCurSegment(x, y, track_angle, cur_segment, track_data); 
	}
	return(cur_segment);
}

//check if the point is under, above or on the line
//
//Returns:
//0: below the line
//1: on the line
//2: above the line
int PointAndLine(float x, float y, float x1, float y1, float x2, float y2){
	float a = (y2-y1)/(x2-x1);
	float b = y2-a*x2;
	float line_val = a*x+b;
	return (line_val==y) + (line_val<y)*2;
}


float InInterval(float a){
	while(a>M_PI){
		a-=2*M_PI;
	}
	while(a<-M_PI){
		a+=2*M_PI;
	}
	return a;

}
// vim: cc=100
