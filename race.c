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




#include <allegro5/allegro_audio.h>
#include <allegro5/allegro5.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_image.h>
#include <math.h>

#include <stdio.h>
#include "drawtrack.h"
#include "kart.h"
#include "config.h"
#include "race.h"

//the modes of play
#define COUNTDOWN 0
#define PLAYING 1
#define END 2


int compare(const void* a, const void* b);//Function used by sort function
char * SecToString(float  secs);//Easily display time
int inc_circ_count(int i, int max);//Make it loop round forwards
int dec_circ_count(int i, int max);//Make it loop round backwards
int GetCurSegment(float x, float y, float* track_angle, int cur_segment, TRACK_DATA track_data);
int PointAndLine(float x, float y, float x1, float y1, float x2, float y2);
float InInterval(float a);//Have angle between -pi and pi
void SaveRace(float *buf, int am_frames, FILE *save_file, float fps);//Saves the race in a file
void DrawKart(kart_t kart);

void race(ALLEGRO_FS_ENTRY *track_file_entry, CONFIG* config, ALLEGRO_DISPLAY* disp, ALLEGRO_AUDIO_STREAM* audio_stream){
	TRACK_DATA track;
	
	ALLEGRO_FILE* track_file = al_open_fs_entry(track_file_entry, "r");
	if(!track_file){
		fprintf(stderr, "Could not open track file\n");
		return;
	}


	loadtrack(track_file, &track);
	al_fclose(track_file);

	//TODO: track settings should be stored in the track file not defined in source
	track.border_color = al_map_rgb(255,0,0);
	track.milestones = true;
	track.milestone_interval = 20;
	track.milestone_size = 4;
	track.border_width = 4;
	
	//timers
	ALLEGRO_TIMER* timer = al_create_timer(1.0 / config->fps);
	must_init(timer,"timer");
	ALLEGRO_TIMER* stopwatch = al_create_timer(0.001);
	must_init(stopwatch,"stopwatch");
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

    	al_register_event_source(queue, al_get_audio_stream_event_source(audio_stream));
	al_register_event_source(queue, al_get_keyboard_event_source());
    	al_register_event_source(queue, al_get_display_event_source(disp));
    	al_register_event_source(queue, al_get_timer_event_source(timer));

	ALLEGRO_BITMAP* full_heart = al_load_bitmap("full heart.png");
	ALLEGRO_BITMAP* half_heart = al_load_bitmap("half heart.png");

	#define KEY_SEEN     1
	#define KEY_RELEASED 2

	unsigned char key[ALLEGRO_KEY_MAX];
	memset(key, 0, sizeof(key));

	float v=0;
	float angle=M_PI/2;
	float min_radius = sqrt(config->kart_height*config->kart_height/4+pow(config->kart_height/tan(config->max_wheel_angle)+config->kart_width/2,2));
	float scale=1;
	float track_angle;
	float damage=0;

	//the position of the middle of the kart
	float x_pos=0;
	float y_pos=0;
	
	ALLEGRO_COLOR kart_color=al_map_rgb(config->kart_color_r,config->kart_color_g,config->kart_color_b);


	//keeps track in wich segment the kart is in.
	//negative means outside the track
	int cur_segment =0;
	int max_segment =cur_segment;//the highest segment it has fairly reached
	double time=0;
	int mode = COUNTDOWN;
	int lap = 1;

	int frame;
	float audio_period_pos=0;
	while(1){
		al_acknowledge_resize(disp);
		al_wait_for_event(queue,&event);
		int screen_width = al_get_display_width(disp);
		int screen_height = al_get_display_height(disp);

		_Bool EndProgram=false;
		_Bool redraw=false;
		
		int count_val;
		
		if(mode==COUNTDOWN){
			count_val = (int)(config->sec_before_start-al_get_timer_count(countdown));
			if( count_val<= 0){
				mode=PLAYING;
				frame=0;
				al_start_timer(stopwatch);
			}
		}

		switch(event.type){
			case(ALLEGRO_EVENT_AUDIO_STREAM_FRAGMENT):
				
				;void* sample_buf = al_get_audio_stream_fragment(audio_stream);
				if(sample_buf == NULL){
					printf("Could not get audio stream fragment\n");
					break;
				}
				
				int tone_freq = v/(2*M_PI*config->kart_wheel_radius);
				
				int i =0;
				while(i<(int)al_get_audio_stream_length(audio_stream)){
					*((float*)sample_buf + i) = sin(audio_period_pos+((float)tone_freq*(float)i/(float)al_get_audio_stream_frequency(audio_stream)));
					i++;
				}
				audio_period_pos+=((float)tone_freq*(float)i/(float)al_get_audio_stream_frequency(audio_stream));
				if(!al_set_audio_stream_fragment( audio_stream, sample_buf))
					printf("Could not set audio stream fragment\n");
				break;
		


			case(ALLEGRO_EVENT_TIMER):
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

				if(key[ALLEGRO_KEY_ESCAPE]){
					EndProgram = true;
				}


				for(int i = 0; i < ALLEGRO_KEY_MAX; i++)
			    		key[i] &= KEY_SEEN;

				float new_x_pos=x_pos+cos(angle)*v/config->fps;
				float new_y_pos=y_pos+sin(angle)*v/config->fps;


				int new_cur_segment=GetCurSegment(new_x_pos, new_y_pos, &track_angle, cur_segment, track);
				if(new_cur_segment!=-1){
					if(mode==PLAYING&&max_segment==track.n_segments-1 && 
							cur_segment==track.n_segments-1 && new_cur_segment==0){
						lap++;
						if(lap>config->laps){
							//TODO: add this feature
							//if(save_race)SaveRace(pos_buf,frame, save_file, fps);
							mode=END;
							//TODO: add this feature
							/*if(config->record_file_available){//maybe there is no record file
								fprintf(config->record_file, "%f\n", time);
								fclose(record_file);
							}*/
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
					//Work untill here
					float v_new;
					if(angle_kart_track>M_PI/2 || angle_kart_track<-M_PI/2){
						//angle = track_angle-M_PI;
						angle = 2*track_angle-angle;
						v_new=cos(angle_kart_track-M_PI)*v/4;
					}else{
						v_new=cos(angle_kart_track)*v/4;
						//angle = track_angle;
						angle = 2*track_angle-angle;
					}
					/*
					if((angle_kart_track>0.5*M_PI) || angle_kart_track<0 && angle_kart_track>-0.5*M_PI)angle-=0.1;
					else angle+=0.1;
					*/
					damage+=abs(v-v_new);
					if(mode==PLAYING && damage>=config->death_crash){
						//if(save_race)SaveRace(pos_buf,frame, save_file, fps);

						time=-1;
						mode=END;
					}
					v=v_new;
				}

				/*if(save_race&&mode==PLAYING){
					if(frame==0)pos_buf=malloc(sizeof(float)*3);
					else pos_buf=realloc(pos_buf, sizeof(float)*3*(frame+1));
					pos_buf[frame*3]=x_pos;
					pos_buf[frame*3+1]=y_pos;
					pos_buf[frame*3+2]=angle;
				}*/

				redraw=true;
				break;
			case(ALLEGRO_EVENT_KEY_DOWN):
				key[event.keyboard.keycode] = KEY_SEEN | KEY_RELEASED;
				if(key[ALLEGRO_KEY_F11]){
					//Toggle full screen
					al_set_display_flag(disp, ALLEGRO_FULLSCREEN_WINDOW, !(_Bool)(al_get_display_flags(disp) & ALLEGRO_FULLSCREEN_WINDOW));
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
				time=(double)al_get_timer_count(stopwatch)/1000;
			}
			al_clear_to_color(al_map_rgb(0,0,0));
			drawtrack(x_pos,y_pos, -track_angle-M_PI/2,scale,screen_width, screen_height,track);
			
			
			//Draw hearts
			float heart_width = 10;
			float heart_height = 10;
			float heart_gap = 2;
			int am_hearts = (int)ceil((config->death_crash-damage)/config->life_per_heart);

			int heart_i=0;
			while(heart_i<am_hearts){
				al_draw_scaled_bitmap(full_heart, 0, 0, al_get_bitmap_width(full_heart), 
					al_get_bitmap_height(full_heart),
					screen_width-heart_width*(heart_i+1)-heart_gap*heart_i,
					0,heart_width,heart_height,0);
				heart_i++;
			}
			
			//draw player karts
			ALLEGRO_COLOR kart_color=al_map_rgb(config->kart_color_r,config->kart_color_g,config->kart_color_b);
			kart_t main_kart = {angle, x_pos, y_pos, config->kart_width, config->kart_height, kart_color};
			drawkart(x_pos, y_pos,-track_angle-M_PI/2, scale, screen_width, screen_height,  main_kart);
			//		-track_angle-M_PI/2+angle, kart_color);

			int play_frame;
			//draw replay kart
			ALLEGRO_COLOR ghost_color=al_map_rgb(config->ghost_color_r,config->ghost_color_g,config->ghost_color_b);

			//TODO: add the replay feature
/*			if(config->replay_race){
				//play_frame=(int)((float)frame/fps*replay_fps);
				play_frame = (int)(config->replay_fps*time);
				if(play_frame<config->am_frames){
					float play_pos[]={replay_buf[play_frame*3],replay_buf[play_frame*3+1]};
					kart_t ghost_kart = {replay_buf[play_frame*3+2],
						replay_buf[play_frame*3], replay_buf[play_frame*3+1], 
						kart_width, kart_height, ghost_color};

					drawkart(x_pos, y_pos, -track_angle-M_PI/2, scale, screen_width, screen_height, ghost_kart);

				}
			}*/

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
				if(map_max_height/(track.max_y - track.min_y)<map_scale)map_scale=map_max_height/(track.max_y-track.min_y);

				float map_c_x=midx-(map_x-screen_width/2)/map_scale;
				float map_c_y=midy-(map_y-screen_height/2)/map_scale;

				drawtrack(map_c_x, map_c_y, 0, map_scale, screen_width, screen_height, track);
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
				/*if(replay_race){
					float ghost_dotx=replay_buf[play_frame*3];
					float ghost_doty=replay_buf[play_frame*3+1];
					al_transform_coordinates(&transform, &ghost_dotx, &ghost_doty);
					al_draw_filled_circle(ghost_dotx, ghost_doty, 3, ghost_color);
				}*/
				al_identity_transform(&transform);
				al_use_transform(&transform);

					

			}
			if(mode==COUNTDOWN){
				char text[10];
				sprintf(text, "%d", count_val);
				al_draw_text(splash, al_map_rgb(255, 255, 255), screen_width/2-al_get_text_width(splash,text)/2, 
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
				sprintf(timetext, "time=%s, ", SecToString(time));
				strcat(infotext, timetext);
			}
			/*if(show_record ){
				if(am_records>0){
					char recordtext[20];
					sprintf(recordtext, "record=%s, ", SecToString(records[0]));
					strcat(infotext, recordtext);
				}
			}*/
			if(config->show_lap){
				char lap_text[20];
				sprintf(lap_text, "lap=%d", lap);
				strcat(infotext, lap_text);
			}


			al_draw_text(font, al_map_rgb(255, 255, 255), 0, 0, 0, infotext);

			if(mode==END){
				char* complete_text;
				_Bool shouldfree=0;
				if(time==-1)complete_text="YOU CRASHED!";
				//TODO: add this feature
				/*
				else if((am_records==0||time<records[0])&&record_file_available){
					complete_text=malloc(sizeof(char)*30);
					sprintf(complete_text, "You've got a new record:%s", SecToString(time));
					shouldfree=1;
				}*/
				else
					complete_text=SecToString(time);

				al_draw_text(splash, al_map_rgb(255, 255, 255), screen_width/2-al_get_text_width(splash,complete_text)/2, 
						screen_height/2-al_get_font_ascent(splash)/2,0, complete_text);
				if(shouldfree)free(complete_text);
			}
			al_flip_display();
			if(mode==PLAYING)
				frame++;

		}
	}
	al_destroy_bitmap(half_heart);
	al_destroy_bitmap(full_heart);
	al_shutdown_image_addon();



	al_destroy_event_queue(queue);
	
	al_destroy_timer(countdown);
	al_destroy_timer(stopwatch);
	al_destroy_timer(timer);

}

//Function used by sort function
int compare(const void* a, const void* b){
	float x = *(float*)a;
	float y = *(float*)b;
	return x<=y ? (x<y?-1 : 0) : 1;
}




char * SecToString(float  secs){
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
		if(abs(segment->outer.y1-segment->outer.y2)<1 || abs(segment->outer.x1 - segment->inner.x1)<1){
			if(segment->outer.x1>segment->outer.x2)*track_angle=M_PI;
			else *track_angle=0;
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
		else if(abs(segment->outer.y1-segment->inner.y1)<1 || abs(segment->outer.x1 - segment->outer.x2)<1){
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
							segment->inner.y2, segment->outer.x2, segment->outer.y2)==2){
				//y2<y1
				if(PointAndLine(x,y,segment->inner.x1,segment->inner.y1,segment->outer.x1
							,segment->outer.y1)==2){
					cur_segment = dec_circ_count(cur_segment, track_data.n_segments-1);
					change=1;
				}
				else if(PointAndLine(x,y,segment->inner.x2,segment->inner.y2,segment->outer.x2
							,segment->outer.y2)==0){
					cur_segment = inc_circ_count(cur_segment, track_data.n_segments-1);
					change=1;
				}
			}else{
				//y2>y1
				if(PointAndLine(x,y,segment->inner.x1,segment->inner.y1,segment->outer.x1
							,segment->outer.y1)==0){
					cur_segment = dec_circ_count(cur_segment, track_data.n_segments-1);
					change=1;
				}
				else if(PointAndLine(x,y,segment->outer.x2,segment->outer.y2,segment->inner.x2
							,segment->inner.y2)==2){
					cur_segment = inc_circ_count(cur_segment, track_data.n_segments-1);
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

void SaveRace(float *buf, int am_frames, FILE *save_file, float fps){
	printf("Storing %d frames in race file", am_frames);
	fwrite(&am_frames, sizeof(int), 1, save_file);
	fwrite(&fps, sizeof(float), 1, save_file);
	fwrite(buf, sizeof(float), am_frames*3, save_file);
}

void DrawKart(kart_t kart){
	//the distance between the middle of the square to the corners
	float point_distance = sqrt(kart.kart_height*kart.kart_height+kart.kart_width*kart.kart_width)/2;

	int i=0;
	float points[8];
	while(i<4){
		float slope;
		if(i%2==1)
			slope=kart.kart_height/kart.kart_width;
		else
			slope=kart.kart_width/kart.kart_height;
		double point_angle=-M_PI+atan(slope)+M_PI/2*i;
		points[2*i]=cos(point_angle+kart.angle)*point_distance+kart.x;
		points[2*i+1]=sin(point_angle+kart.angle)*point_distance+kart.y;
		i++;
	}
	//TODO make this drawing more efficient
	al_draw_filled_triangle(points[0],points[1],points[2],points[3],points[4],points[5],
			kart.color);

	al_draw_filled_triangle(points[4],points[5],points[0],points[1],points[6],points[7],
			kart.color);
}
