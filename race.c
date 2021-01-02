//You can now race together!!
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

//the modes of play
#define COUNTDOWN 0
#define PLAYING 1
#define END 2


int compare(const void* a, const void* b);//Function used by sort function
void must_init(bool test, const char *description);//Error messages
char * SecToString(float  secs);//Easily display time
int inc_circ_count(int i, int max);//Make it loop round forwards
int dec_circ_count(int i, int max);//Make it loop round backwards
int GetCurSegment(float x, float y, float* track_angle, int cur_segment, TRACK_DATA track_data);
int PointAndLine(float x, float y, float x1, float y1, float x2, float y2);
float InInterval(float a);//Have angle between -pi and pi
void SaveRace(float *buf, int am_frames, FILE *save_file, float fps);//Saves the race in a file
void DrawKart(kart_t kart);
_Bool get_config_bool(const ALLEGRO_CONFIG *config, const char *section, const char *key);

void main(int argc, char**argv){
	TRACK_DATA track;

	//get info from config file
	//TODO: Have better error detection
	ALLEGRO_CONFIG* cfg = al_load_config_file("config.cfg");

	const char*font_name= 	al_get_config_value(cfg, "", "font");
	
	int 	laps= 		atoi(al_get_config_value(cfg,"", "laps"));
	float 	fps=		atof(al_get_config_value(cfg, "", "fps"));
	int 	sec_before_start=atoi(al_get_config_value(cfg, "", "sec_before_start"));

	//window
	int 	window_width= 	atoi(al_get_config_value(cfg, "window", "width"));
	int	window_height= 	atoi(al_get_config_value(cfg, "window", "height"));

	//kart
	float	kart_width= 	atof(al_get_config_value(cfg, "kart", "width"));
	float	kart_height= 	atof(al_get_config_value(cfg, "kart", "height"));

	float	accelleration= 	atof(al_get_config_value(cfg, "kart", "accelleration"));
	float	back_accelleration=atof(al_get_config_value(cfg,"kart", "back_accelleration"));
	float	break_speed= 	atof(al_get_config_value(cfg,"kart", "break_speed"));
	float	death_crash= 	atof(al_get_config_value(cfg,"kart", "death_crash"));

	float	max_wheel_angle= atof(al_get_config_value(cfg,"kart", "max_wheel_angle"));

	float	max_F=atof(al_get_config_value(cfg,"kart", "max_F"));
	float	mass=atof(al_get_config_value(cfg,"kart", "mass"));

	int	kart_color_r=atoi(al_get_config_value(cfg,"kart", "color_r"));
	int	kart_color_g=atoi(al_get_config_value(cfg,"kart", "color_g"));
	int	kart_color_b=atoi(al_get_config_value(cfg,"kart", "color_b"));
	
	float	kart_wheel_radius=atof(al_get_config_value(cfg,"kart", "wheel_radius"));

	//track
	track.border_width=atof(al_get_config_value(cfg, "track", "border_width"));
	track.border_color.r=(float)atoi(al_get_config_value(cfg, "track", "color_r"))/255;
	track.border_color.g=atoi(al_get_config_value(cfg, "track", "color_g"))/255;
	track.border_color.b=atoi(al_get_config_value(cfg, "track", "color_b"))/255;
	track.border_color.a=1;
	

	//milestones
	track.milestones=get_config_bool(cfg, "milestones", "milestones");
	track.milestone_interval = atoi(al_get_config_value(cfg, "milestones", "interval"));
	if(track.milestone_interval<=0){
		fprintf(stderr, "Milestone interval is not allowed to be zero or smaller\n");
		return;
	}
	track.milestone_size = atoi(al_get_config_value(cfg, "milestones", "size"));

	//information
	_Bool show_speed=get_config_bool(cfg, "information", "speed");
	_Bool show_kart_angle=get_config_bool(cfg, "information", "kart_angle");
	_Bool show_scale=get_config_bool(cfg, "information", "scale");
	_Bool show_track_angle=get_config_bool(cfg, "information", "track_angle");
	_Bool show_damage=get_config_bool(cfg, "information", "damage");
	_Bool show_x_pos=get_config_bool(cfg, "information", "x_pos");
	_Bool show_y_pos=get_config_bool(cfg, "information", "y_pos");
	_Bool show_segment=get_config_bool(cfg, "information", "segment");
	_Bool show_stopwatch=get_config_bool(cfg, "information", "stopwatch");
	_Bool show_record=get_config_bool(cfg, "information", "record");
	_Bool show_map=get_config_bool(cfg, "information", "map");
	_Bool show_lap=get_config_bool(cfg, "information", "lap");

	float	life_per_heart= atof(al_get_config_value(cfg, "information", "life_per_heart"));	

	//ghost
	int	ghost_color_r=atoi(al_get_config_value(cfg,"ghost", "color_r"));
	int	ghost_color_g=atoi(al_get_config_value(cfg,"ghost", "color_g"));
	int	ghost_color_b=atoi(al_get_config_value(cfg,"ghost", "color_b"));

	_Bool record_file_available=false;
	FILE *record_file;
	int am_records=0;
	float* records;
	

	/*The save file starts with a value of the type size_t that sais how many bytes long the position array is.
	 * 
	 * After that comes the position array which is just an array of floats*/

	_Bool save_race=false;
	FILE* save_file;
	float* pos_buf;

	_Bool replay_race=false;
	float replay_fps;
	FILE* replay_file;
	float* replay_buf;
	int am_frames;

	FILE* track_file;

	//read the parameters given by the user
	int i=1;
	_Bool got_track=false;
	while(i<argc){
		if(argv[i][0] == '-')
			switch(argv[i][1]){
				//load record file
				case('r'):
					i++;
					int record_list_len = 10;
					records=malloc(sizeof(float)*record_list_len);
					if(i==argc){
						fprintf(stderr, "a \'-r\' flag must be accompanied by a filename\n");
						return;
					}
					//load records in 
					record_file=fopen(argv[i], "r+");
					if(record_file==NULL){
						//the file does not exist yet
						record_file=fopen(argv[i], "w+");
					}
					else{
						record_file_available=true;

						while(1){
							add_element((void*)records, &am_records, &record_list_len, sizeof(float));
							int ret=fscanf(record_file,"%f\n",records+am_records-1);
							if(0==ret || EOF ==ret)break;
						}
						qsort((void*)records, am_records-1, sizeof(float), compare);
					}
					break;
				
				//save this race
				case('s'):
					save_race=true;
					i++;
					if(i==argc){
						fprintf(stderr, "a \'-s\' flag must be accompanied by a filename\n");
						return;
					}

					
					save_file=fopen(argv[i], "wb");
					
				
					break;
				//play previous race together with this race
				case('p'):
					replay_race=true;
					i++;
					if(i==argc){
						fprintf(stderr, "a \'-p\' flag must be accompanied by a filename\n");
						return;
					}
					replay_file = fopen(argv[i], "rb");
					fread(&am_frames, sizeof(int), 1,replay_file);
					fread(&replay_fps, sizeof(float), 1,replay_file);
					replay_buf=malloc(sizeof(int)*am_frames*3);
					fread(replay_buf, sizeof(float), 3*am_frames, replay_file);
					break;

			}
		//means the next argument is the track file
		else if(!got_track){
			got_track=true;
			track_file = fopen(argv[i], "r");
			if(track_file==NULL){
				fprintf(stderr,"Could not open %s\n", argv[1]);
				return;
			}
		}
		else{
			fprintf(stderr, "Invalid amount of parameters: %s [OPTION] [Track file]\n", argv[0]);
			exit(1);
		}
		i++;
	}
	if(!got_track){
		fprintf(stderr, "No track was specified: %s [OPTION] [Track file]\n", argv[0]);
		return;
	}
	



	loadtrack(track_file, &track);
	fclose(track_file);


	//initialize and check for errors
	must_init(al_init(),"allegro");
	must_init(al_install_keyboard(),"couldn't initialize keyboard\n");

	ALLEGRO_TIMER* timer = al_create_timer(1.0 / fps);
	must_init(timer,"timer");
	ALLEGRO_TIMER* stopwatch = al_create_timer(0.001);
	must_init(stopwatch,"stopwatch");
	ALLEGRO_TIMER* countdown = al_create_timer(1);
	must_init(countdown,"countdown");

	
	ALLEGRO_EVENT_QUEUE* queue = al_create_event_queue();
	must_init(queue,"queue");


	//set up audio
	if(!al_install_audio())
		fprintf(stderr, "Failed to install audio\n");
	al_reserve_samples(1);

	int audio_freq = 44100;
	float kart_sample_len_sec = 1.0/20.0;
	
	ALLEGRO_AUDIO_STREAM* audio_stream = al_create_audio_stream(2, audio_freq*kart_sample_len_sec, audio_freq, 
			ALLEGRO_AUDIO_DEPTH_FLOAT32, ALLEGRO_CHANNEL_CONF_1);
	if(audio_stream==NULL)
		printf("Could not create a stream\n");
	if(!al_attach_audio_stream_to_mixer(audio_stream, al_get_default_mixer()))
		printf("Could not attach audio stream to mixer\n");
	if(!al_set_audio_stream_playing(audio_stream, true))
		printf("Could not play audio stream\n");
	al_set_audio_stream_playmode(audio_stream, ALLEGRO_PLAYMODE_ONCE);
	

	al_set_new_display_flags(ALLEGRO_RESIZABLE|ALLEGRO_WINDOWED);
	ALLEGRO_DISPLAY* disp = al_create_display(window_width, window_height);
	must_init(disp,"couldn't initialize display\n");
	
	ALLEGRO_FONT* font = al_create_builtin_font();
	must_init(font,"couldn't initialize font\n");

	must_init(al_init_font_addon(), "font addon");
	must_init(al_init_ttf_addon(), "ttf");
	printf("Using font: %s\n", font_name);
	ALLEGRO_FONT* splash = al_load_ttf_font(font_name, 20, ALLEGRO_TTF_MONOCHROME);

	ALLEGRO_EVENT event;
	al_start_timer(timer);
	al_start_timer(countdown);

	must_init(al_init_primitives_addon(), "primitives");

    	al_register_event_source(queue, al_get_audio_stream_event_source(audio_stream));
	al_register_event_source(queue, al_get_keyboard_event_source());
    	al_register_event_source(queue, al_get_display_event_source(disp));
    	al_register_event_source(queue, al_get_timer_event_source(timer));

	must_init(al_init_image_addon(), "image");
	ALLEGRO_BITMAP* full_heart = al_load_bitmap("full heart.png");
	ALLEGRO_BITMAP* half_heart = al_load_bitmap("half heart.png");

	#define KEY_SEEN     1
	#define KEY_RELEASED 2

	unsigned char key[ALLEGRO_KEY_MAX];
	memset(key, 0, sizeof(key));

	float v=0;
	float angle=M_PI/2;
	float min_radius = sqrt(kart_height*kart_height/4+pow(kart_height/tan(max_wheel_angle)+kart_width/2,2));
	float scale=1;
	float track_angle;
	float damage=0;

	//the position of the middle of the kart
	float x_pos=0;
	float y_pos=0;
	
	ALLEGRO_COLOR kart_color=al_map_rgb(kart_color_r,kart_color_g,kart_color_b);


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
			count_val = (int)(sec_before_start-al_get_timer_count(countdown));
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
				
				int tone_freq = v/(2*M_PI*kart_wheel_radius);
				
				int i =0;
				while(i<(int)audio_freq*kart_sample_len_sec){
					*((float*)sample_buf + i) = sin(audio_period_pos+((float)tone_freq*(float)i/(float)audio_freq));
					i++;
				}
				audio_period_pos+=((float)tone_freq*(float)i/(float)audio_freq);
				if(!al_set_audio_stream_fragment( audio_stream, sample_buf))
					printf("Could not set audio stream fragment\n");
				break;
		


			case(ALLEGRO_EVENT_TIMER):
				if(key[ALLEGRO_KEY_UP]&&mode==PLAYING){
					if(v>=0)v+=accelleration/fps;
					if(v<0)v+=break_speed/fps;
				}
				if(key[ALLEGRO_KEY_DOWN]&&mode==PLAYING){
					if(v>0)v-=break_speed/fps;
					if(v<=0)v-=back_accelleration/fps;
				}
				float radius = mass*v*v/max_F;
				if(key[ALLEGRO_KEY_LEFT]&&mode==PLAYING){
					if(radius<min_radius)radius=min_radius;
					angle-=v/radius/fps;
				}
				if(key[ALLEGRO_KEY_RIGHT]&&mode==PLAYING){
					if(radius<min_radius)radius=min_radius;
					angle+=v/radius/fps;
				}

				if(key[ALLEGRO_KEY_ESCAPE]){
					EndProgram=true;
				}


				for(int i = 0; i < ALLEGRO_KEY_MAX; i++)
			    		key[i] &= KEY_SEEN;

				float new_x_pos=x_pos+cos(angle)*v/fps;
				float new_y_pos=y_pos+sin(angle)*v/fps;


				int new_cur_segment=GetCurSegment(new_x_pos, new_y_pos, &track_angle, cur_segment, track);
				if(new_cur_segment!=-1){
					if(mode==PLAYING&&max_segment==track.n_segments-1 && 
							cur_segment==track.n_segments-1 && new_cur_segment==0){
						lap++;
						if(lap>laps){
							if(save_race)SaveRace(pos_buf,frame, save_file, fps);
							mode=END;
							if(record_file_available){//maybe there is no record file
								fprintf(record_file, "%f\n", time);
								fclose(record_file);
							}
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
					if(mode==PLAYING && damage>=death_crash){
						if(save_race)SaveRace(pos_buf,frame, save_file, fps);

						time=-1;
						mode=END;
					}
					v=v_new;
				}

				if(save_race&&mode==PLAYING){
					if(frame==0)pos_buf=malloc(sizeof(float)*3);
					else pos_buf=realloc(pos_buf, sizeof(float)*3*(frame+1));
					pos_buf[frame*3]=x_pos;
					pos_buf[frame*3+1]=y_pos;
					pos_buf[frame*3+2]=angle;
				}


				redraw=1;
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
				EndProgram=1;	
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
			int am_hearts = (int)ceil((death_crash-damage)/life_per_heart);

			int heart_i=0;
			while(heart_i<am_hearts){
				al_draw_scaled_bitmap(full_heart, 0, 0, al_get_bitmap_width(full_heart), 
					al_get_bitmap_height(full_heart),
					screen_width-heart_width*(heart_i+1)-heart_gap*heart_i,
					0,heart_width,heart_height,0);
				heart_i++;
			}
			
			//draw player karts
			ALLEGRO_COLOR kart_color=al_map_rgb(kart_color_r,kart_color_g,kart_color_b);
			kart_t main_kart = {angle, x_pos, y_pos, kart_width, kart_height, kart_color};
			drawkart(x_pos, y_pos,-track_angle-M_PI/2, scale, screen_width, screen_height,  main_kart);
			//		-track_angle-M_PI/2+angle, kart_color);

			int play_frame;
			//draw replay kart
			ALLEGRO_COLOR ghost_color=al_map_rgb(ghost_color_r,ghost_color_g,ghost_color_b);
			if(replay_race){
				//play_frame=(int)((float)frame/fps*replay_fps);
				play_frame = (int)(replay_fps*time);
				if(play_frame<am_frames){
					float play_pos[]={replay_buf[play_frame*3],replay_buf[play_frame*3+1]};
					kart_t ghost_kart = {replay_buf[play_frame*3+2],
						replay_buf[play_frame*3], replay_buf[play_frame*3+1], 
						kart_width, kart_height, ghost_color};

					drawkart(x_pos, y_pos, -track_angle-M_PI/2, scale, screen_width, screen_height, ghost_kart);

				}
			}

			//display the map
			if(show_map){
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
				printf("map_c1: x=%f, y=%f\n", map_c_x, map_c_y);

				drawtrack(map_c_x, map_c_y, 0, map_scale, screen_width, screen_height, track);
				//draw the dot that represents the player
				
				ALLEGRO_TRANSFORM transform;
				al_identity_transform(&transform);
				al_translate_transform(&transform, -map_c_x, -map_c_y );
				al_scale_transform(&transform, map_scale,map_scale);
				printf("map_c2: x=%f, y=%f\n", map_c_x, map_c_y);
				al_translate_transform(&transform, screen_width/2,screen_height/2);
				
				
				float dotx=x_pos;
				float doty=y_pos;
				al_transform_coordinates(&transform, &dotx, &doty);

				al_draw_filled_circle(dotx, doty, 3, kart_color);
				if(replay_race){
					float ghost_dotx=replay_buf[play_frame*3];
					float ghost_doty=replay_buf[play_frame*3+1];
					al_transform_coordinates(&transform, &ghost_dotx, &ghost_doty);
					al_draw_filled_circle(ghost_dotx, ghost_doty, 3, ghost_color);
				}
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
			if(show_speed){
				char speedtext[20];
				sprintf(speedtext, "speed=%f, ", v);
				strcat(infotext, speedtext);
			}
			if(show_kart_angle){
				char angletext[20];
				sprintf(angletext, "angle=%f, ", angle);
				strcat(infotext, angletext);
			}
			if(show_scale){
				char scaletext[20];
				sprintf(scaletext, "scale=%f, ", scale);
				strcat(infotext, scaletext);
			}
			if(show_track_angle){
				char track_angletext[20];
				sprintf(track_angletext, "angle=%f, ", track_angle);
				strcat(infotext, track_angletext);
			}
			if(show_damage){
				char damagetext[20];
				sprintf(damagetext, "damage=%f, ", damage);
				strcat(infotext, damagetext);
			}
			if(show_x_pos){
				char x_postext[20];
				sprintf(x_postext, "xpos=%f, ", x_pos);
				strcat(infotext, x_postext);
			}
			if(show_y_pos){
				char y_postext[20];
				sprintf(y_postext, "ypos=%f, ", y_pos);
				strcat(infotext, y_postext);
			}
			if(show_segment){
				char segmenttext[20];
				sprintf(segmenttext, "segment=%d, ", cur_segment);
				strcat(infotext, segmenttext);
			}
			if(show_stopwatch){
				char timetext[20];
				sprintf(timetext, "time=%s, ", SecToString(time));
				strcat(infotext, timetext);
			}
			if(show_record ){
				if(am_records>0){
					char recordtext[20];
					sprintf(recordtext, "record=%s, ", SecToString(records[0]));
					strcat(infotext, recordtext);
				}
			}
			if(show_lap){
				char lap_text[20];
				sprintf(lap_text, "lap=%d", lap);
				strcat(infotext, lap_text);
			}


			al_draw_text(font, al_map_rgb(255, 255, 255), 0, 0, 0, infotext);

			if(mode==END){
				char* complete_text;
				_Bool shouldfree=0;
				if(time==-1)complete_text="YOU CRASHED!";
				else if((am_records==0||time<records[0])&&record_file_available){
					complete_text=malloc(sizeof(char)*30);
					sprintf(complete_text, "You've got a new record:%s", SecToString(time));
					shouldfree=1;
				}
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
}

//Function used by sort function
int compare(const void* a, const void* b){
	float x = *(float*)a;
	float y = *(float*)b;
	return x<=y ? (x<y?-1 : 0) : 1;
}



void must_init(bool test, const char *description)
{
    if(test) return;

    fprintf(stderr,"couldn't initialize %s\n", description);
    exit(1);
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
		

		/*//check if if angle is between seg_angle and end_angle
		if(direction && (angle<seg_angle || angle>end_angle) ||
				!direction && (angle>seg_angle || angle<end_angle)){
			//it is of the segment, but still on track
			//now see which way it went
			
			//TODO: have it as a tertiary expression
			if(InInterval(absolute(seg_angle-angle))>InInterval(absolute(end_angle-angle))){
				(cur_segment)++;
				change=1;
			}else{
				(cur_segment)--;
				change=1;
			}
		}*/
		/*if(segment->delta_angle<0){
			if(angle>segment->start_angle){
				(cur_segment)--;
				change=1;
			}else if(angle<segment->start_angle+segment->delta_angle){
				(cur_segment)++;
				change=1;
			}
		}else{
			if(angle<segment->start_angle){
				(cur_segment)--;
				change=1;
			}else if(angle>segment->start_angle+segment->delta_angle){
				(cur_segment)++;
				change=1;
			}
		}*/
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

_Bool get_config_bool(const ALLEGRO_CONFIG *config, const char *section, const char *key){
	const char * string = al_get_config_value(config, section, key);
	if(!strcmp(string, "true"))return(true);
	else if(!strcmp(string, "false"))return(false);
	else{
		fprintf(stderr, "\"%s\" is an invalid option for \"%s\" in section \"%s\".\n" ,string, 
				key, section);
		exit(1);
	}
}
