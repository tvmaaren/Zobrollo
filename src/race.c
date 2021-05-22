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

//networking
#ifdef _WIN32
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#include <math.h>
#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <signal.h>

#include "global.h"
#include "networking.h"
#include "drawtrack.h"
#include "config.h"
#include "kart.h"
#include "file_paths.h"
#include "misc.h"
#include "race.h"
#include "record.h"
#include "drawframe.h"

#define date_string_len 18

char * SecToString(double  secs);//Easily display time
int inc_circ_count(int i, int max);//Make it loop round forwards
int dec_circ_count(int i, int max);//Make it loop round backwards
int GetCurSegment(float x, float y, float* track_angle, int cur_segment, TRACK_DATA* track_data);
float InInterval(float a);//Have angle between -pi and pi
void SaveRace(float *buf, int am_frames, FILE *save_file, float fps);//Saves the race in a file


void get_date_string(char* string){
	time_t pSec = time(NULL);
	struct tm* local_time = localtime(&pSec);
	sprintf(string,
			"%d-%02d-%02d "
			"%02d%02d%02d\n", 
			local_time->tm_year+1900,
			local_time->tm_mon+1,
			local_time->tm_mday,
			local_time->tm_hour,
			local_time->tm_min,
			local_time->tm_sec);
}

void store_record(char * filename, double stopwatch, char* date_string){

	//store the time the race was made in the 
	//record file
	al_change_directory(paths.record);
	ALLEGRO_FILE* record_file = al_fopen(filename, "a");
	al_fputs(record_file,date_string);
	char record_file_text[20];

	sprintf(record_file_text,"%f\n",stopwatch);
	al_fputs(record_file, record_file_text);
	al_fclose(record_file);
}
void store_ghost(char* filename, char* date_string, int frames, int fps,float*ghost_buf){
	//remove \n
	date_string[date_string_len-1]='\0';
	if(!al_change_directory(paths.ghost)){
		al_make_directory(paths.ghost);
		if(!al_change_directory(paths.ghost)){
			fprintf(stderr,"Could not make directory: %s\n",paths.ghost);
			exit(1);
		}
	}
	
	char ghost_filename[strlen(filename)+1+date_string_len+sizeof(".bin")];
	strcpy(ghost_filename, filename);
	strcat(ghost_filename, sep_str);
	if(!al_make_directory(
			ghost_filename)){
		fprintf(stderr, "Error: Could not create \"%s\"\n",ghost_filename);
		exit(1);
	}
	strcat(ghost_filename,date_string);
	strcat(ghost_filename, ".bin");
	ALLEGRO_FILE* ghost_file = al_fopen(ghost_filename,"wb");
	if(ghost_file){
		al_fwrite(ghost_file,&frames,sizeof(int));
		al_fwrite(ghost_file,&fps,sizeof(float));
		al_fwrite(ghost_file, ghost_buf, 3*frames*sizeof(float));
		al_fclose(ghost_file);
	}else{
		fprintf(stderr, "Could not make ghost file\n");
	}
}

void sendtrack(TRACK_DATA* track, int socket){
	ssize_t ret = send(socket, (void*)track, sizeof(TRACK_DATA),0);
	if(ret!=sizeof(TRACK_DATA))
		fprintf(stderr, "Error: Did not send track correctly\n");
	int i =0;
	while(i<track->n_segments){
		_Bool type = track->segment_types[i];
		send(socket, (void*)&type, sizeof(_Bool),0);
		size_t segment_size = type ? sizeof(CIRCLE_SEGMENT) : sizeof(LINE_SEGMENT);
		send(socket, track->segments[i], segment_size, 0);
		i++;
	}
}

void recvtrack(TRACK_DATA* track, int socket){
	SetSocketBlocking(socket, 1);
	recv(socket, (void*)track, sizeof(TRACK_DATA), 0);
	
	track->segments = malloc(sizeof(void*)*track->n_segments);
	track->segment_types = malloc(sizeof(_Bool)*track->n_segments);
	if(!track->segments || !track->segment_types){
		fprintf(stderr, "Error: could not allocate memory\n");
		exit(1);
	}

	int i = 0;
	while(i<track->n_segments){
		_Bool type;
		recv(socket, (void*)&type, sizeof(_Bool), 0);
		track->segment_types[i] = type;
		size_t segment_size = type ? sizeof(CIRCLE_SEGMENT) : sizeof(LINE_SEGMENT);
		track->segments[i] = malloc(segment_size);
		recv(socket, track->segments[i], segment_size, 0);
		
		i++;
	}
	SetSocketBlocking(socket, 0);
}


void race(CONNECTION connection,int my_socket,int max_sd, node_t *sockets_head,
		TRACK_DATA *track, char* filename){


    ALLEGRO_FONT* font = al_create_builtin_font();
	ALLEGRO_TIMER* countdown = al_create_timer(1);
	must_init(countdown,"countdown");

	

	must_init(font,"couldn't initialize font\n");

	ALLEGRO_FONT* splash = al_load_ttf_font(config.font_name, 20, ALLEGRO_TTF_MONOCHROME);

	al_start_timer(countdown);

    	al_register_event_source(queue, al_get_timer_event_source(timer));

	ALLEGRO_BITMAP* full_heart = al_load_bitmap(data_dir sep_str"full heart.png");
	ALLEGRO_BITMAP* half_heart = al_load_bitmap(data_dir sep_str"half heart.png");

	#define KEY_SEEN     1
	#define KEY_RELEASED 2

	unsigned char key[ALLEGRO_KEY_MAX];
	memset(key, 0, sizeof(key));
	
	int frame = 0;
	int ghost_buf_len = 10;
	float* ghost_buf = malloc(sizeof(float)*ghost_buf_len);



	record *records;
	int am_records;

	//load ghost of the record
	int frames_record;
	float fps_record;
	float* record_ghost_buf;
	ALLEGRO_FILE* ghost_record_file;
	
	int n_karts;
	_Bool found_ghost_file=false;
	int player_number;
	kart_t karts[10];//All the karts
	kart_t kart;//The kart of this player


	float v=0;
	kart.angle=M_PI/2;
	float min_radius = sqrt(config.kart_height*config.kart_height/4+
		pow(config.kart_height/tan(config.max_wheel_angle)+config.kart_width/2,2));
	float scale=1;
	float track_angle;
	float damage=0;

	//the position of the middle of the kart
	kart.x=0;
	kart.y=0;

	kart.color = config.kart_color;

	//keeps track in wich segment the kart is in.
	//negative means outside the track
	int cur_segment =0;

	int max_segment =cur_segment;//the highest segment it has fairly reached
	double stopwatch=0;
	kart.mode = COUNTDOWN;
	int lap = 1;

	int screen_width = al_get_display_width(disp);
	int screen_height = al_get_display_height(disp);

	double start_time;

	int position;//the position the player is in at the end of the race

	//only used if connection==NONE
	ALLEGRO_FILE* record_file;

	TRACK_DATA connected_track;

	switch(connection){
		case(SERVER):
			//send track to clients
			player_number=0;
			n_karts=1;
			break;
		case(CONNECT_TO_SERVER):
			//receive track from server, but because that is a bit difficult it will
			//load it itself first
			recvtrack(&connected_track, my_socket);
			track 	= &connected_track;
			player_number=0;
			
			break;
		case(NONE):
			player_number=0;
			if(!al_make_directory(paths.record)){
				fprintf(stderr, "Error: Could not create \"%s\"\n",paths.record);
				exit(1);
			}
			if(!al_change_directory(paths.record)){
				fprintf(stderr, "Error: Could not open \"%s\"\n",paths.record);
				exit(1);
			}

			
			record_file =al_fopen(filename,"r+");
			if(!record_file){
			    //file does not exist yet
			    record_file = al_fopen(filename , "w+");
			    if(!record_file){
				    fprintf(stderr, "Error: Could not create \"%s\"\n", filename);
			    }
			}

			al_change_directory(paths.data);
			if(record_file)
				am_records = load_record(record_file, &records, true);
			al_fclose(record_file);
			n_karts=1;
			if(config.play_against_ghost){
				al_change_directory(paths.ghost);
				al_change_directory(filename);
				int i =0;
				while(i<am_records){
					char ghost_record_filename[strlen(records[i].date)+sizeof(".bin")];
					strcpy(ghost_record_filename, records[i].date);
					strcat(ghost_record_filename, ".bin");
					
					ghost_record_file = al_fopen(ghost_record_filename, "rb");
					if(ghost_record_file){
						found_ghost_file = true;
						break;
					}
					i++;
				}
				if(found_ghost_file){
					al_fread(ghost_record_file, &frames_record, sizeof(frames_record));
					al_fread(ghost_record_file, &fps_record, sizeof(fps_record));
					record_ghost_buf = malloc(sizeof(float)*frames_record*3);
					al_fread(ghost_record_file, record_ghost_buf, 
							3*sizeof(float)*frames_record);
					al_fclose(ghost_record_file);
					karts[1].color.r =kart.color.r/2;
					karts[1].color.g =kart.color.g/2;
					karts[1].color.b =kart.color.b/2;
					karts[1].color.a =kart.color.a/2;
					n_karts=2;
				}
				al_change_directory(paths.data);
			}
			break;

	}

	while(true){
		al_wait_for_event(queue,&event);

		_Bool EndProgram=false;
		_Bool redraw=false;
		
		int count_val;

		if(connection==SERVER){
			//check if someone sent information
			
			fd_set read;fd_set error;
			FD_ZERO(&read);FD_ZERO(&error);
			
			//add all sockets to set
			node_t* socket = sockets_head;
			while(true){
				FD_SET(socket->value, &read);
				FD_SET(socket->value, &error);
				if(socket->next)
					socket = socket->next;
				else
					break;
			}
			node_t* sockets_tail = socket;
			struct timeval time_s = {0,0};
			int ret =select(1+max_sd, &read, NULL, &error, &time_s);
			if(ret==-1){
				error_message("reading for network events");
				return;
			}
			if(FD_ISSET(my_socket, &read)){
				add_node(sockets_tail)->value = accept(my_socket, NULL, NULL);
				n_karts++;
				sockets_tail = sockets_tail->next;
				SetSocketBlocking(sockets_tail->value, 0);

				sendtrack(track, sockets_tail->value);
				if(sockets_tail->value>max_sd)max_sd = 
					sockets_tail->value;
			}
			//Else existing socket is sending a message
			socket = sockets_head->next;//should only check clients
			int i =1;
			while(socket){
				if(FD_ISSET(socket->value, &read)){
					while(1){//always get the newest message
						errno=0;
						int ret=recv(socket->value,(void*)(karts+i), 
								sizeof(kart_t),0);
						if(ret<=0)
							break;
					}
				}
				i++;
				socket = socket->next;
			}
			n_karts=i;
			node_t* prev_socket = sockets_head;
			socket = sockets_head->next;
			i=1;
			while(socket){
				//first send the amount of karts
				uint16_t net_n_karts =  htons(n_karts);
				ssize_t ret;
				ret=send(socket->value,(void*) &net_n_karts, sizeof(uint16_t), 0);
				if(ret <= 0){
					error_message("to send n_karts");
					del_node(prev_socket);
					socket = prev_socket;
					n_karts--;
					goto endofwile;
				}

				//send player number
				uint16_t net_i = htons(i);
				ret = send(socket->value,(void*) &net_i, sizeof(uint16_t), 0);
				if(ret <= 0){
					error_message("to send player_number");
					del_node(prev_socket);
					socket = prev_socket;
					n_karts--;
					goto endofwile;
				}
				
				karts[0] = kart;
				ret = send(socket->value,(void*) karts, sizeof(kart_t)*n_karts,0);
				if(ret <= 0){
					error_message("to send player_number");
					del_node(prev_socket);
					socket = prev_socket;
					n_karts--;
					goto endofwile;
				}

endofwile:			prev_socket = socket;
				socket=socket->next;
				i++;
			}
		}
		if(connection==CONNECT_TO_SERVER){
			//send own kart data
			kart_t msg;
			msg=kart;
			send(my_socket,(void*)&msg, sizeof(kart_t),0);
			int c =0;
			while(1){
				errno =0;
				//receives the amount of karts
				
				/*Sometimes an error occurres that the received amount of sockets is 
				 * very big value.I don't know why this happens, but the simple fix 
				 * that I have worked out is that it will not accept more than ten
				 * player more than the previous round*/	
				uint16_t recv_n_karts;
				if((recv(my_socket,(void*)&recv_n_karts, sizeof(uint16_t),0))==-1)
					break;

				
				recv_n_karts = ntohs(recv_n_karts);
				if(recv_n_karts-n_karts<=10)n_karts=recv_n_karts;

				//Get player-number
				uint16_t net_player_number;
				recv(my_socket, (void*)&net_player_number, sizeof(uint16_t),0);
				if(ntohs(net_player_number)>=0&&ntohs(net_player_number)<10){
					player_number = ntohs(net_player_number);
				}
				


				//receives all kart data
				if(recv(my_socket,(void*) karts, sizeof(kart_t)*n_karts,0)==-1)
					break;
				c++;
			}
			char r,g,b,a;
			al_unmap_rgba(karts[0].color, &r, &g, &b, &a);
			karts[player_number] = msg;
		}
		
		if(kart.mode==COUNTDOWN){
			count_val = (int)(config.sec_before_start-al_get_timer_count(countdown));
			if( count_val<= 0){
				kart.mode=PLAYING;
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
				v=v/pow(config.speed_decrease, 1.0/(float)config.fps);
				if(key[ALLEGRO_KEY_UP]&&kart.mode==PLAYING){
					if(v>=0)v+=config.accelleration/config.fps;
					if(v<0)v+=config.break_speed/config.fps;
				}
				if(key[ALLEGRO_KEY_DOWN]&&kart.mode==PLAYING){
					if(v>0)v-=config.break_speed/config.fps;
					if(v<=0)v-=config.back_accelleration/config.fps;
				}
				
				
				float radius = config.mass*v*v/config.max_F;
				if(key[ALLEGRO_KEY_LEFT]&&kart.mode==PLAYING){
					if(radius<min_radius)radius=min_radius;
					kart.angle-=v/radius/config.fps;
				}
				if(key[ALLEGRO_KEY_RIGHT]&&kart.mode==PLAYING){
					if(radius<min_radius)radius=min_radius;
					kart.angle+=v/radius/config.fps;
				}
				if(key[ALLEGRO_KEY_EQUALS]){
					scale*=pow(2,1/config.fps);
				}
				if(key[ALLEGRO_KEY_MINUS]){
					scale*=pow(2,-1/config.fps);
				}

				if(key[ALLEGRO_KEY_ESCAPE]){
					EndProgram = true;
				}


				int i = 0;
				while(i < ALLEGRO_KEY_MAX){
			    		key[i] &= KEY_SEEN;
					i++;
				}

				float new_x_pos=kart.x+cos(kart.angle)*v/config.fps;
				float new_y_pos=kart.y+sin(kart.angle)*v/config.fps;

				int new_cur_segment=get_cur_segment(new_x_pos, new_y_pos,
						&track_angle, cur_segment, track);
				if(new_cur_segment!=-1){
					
					if(kart.mode==PLAYING&&max_segment==track->n_segments-1 && 
							cur_segment==track->n_segments-1 && 
							new_cur_segment==0){
						lap++;
						if(lap>config.laps){
							kart.mode=END;
							if(connection==NONE){
							
							char date_string[date_string_len];
							get_date_string(date_string);
							store_record(filename,stopwatch,
									date_string);

							//Store the ghost in a bin file at
							//local_dir/ghosts/%trackname%/%time%.bin
							if(config.save_ghost){


								store_ghost(filename,date_string,frame,
									config.fps, ghost_buf);
							}
							al_change_directory(paths.data);
							int frame_i = 0;
							}else{
								int i = 0;
								position=1;
								while(i<n_karts){
									position+=karts[i].mode
										==END;
									i++;
								}
							}
						}
						max_segment = 0;
					}
					else if(new_cur_segment-1==max_segment){
						max_segment=new_cur_segment;
					}
					cur_segment=new_cur_segment;
					kart.x=new_x_pos;
					kart.y=new_y_pos;
				}else{
					float angle_kart_track=InInterval(track_angle-kart.angle);
					float v_new;
					if(angle_kart_track>M_PI/2 || angle_kart_track<-M_PI/2){
						kart.angle = 2*track_angle-kart.angle;
						v_new=cos(angle_kart_track-M_PI)*v/4;
					}else{
						v_new=cos(angle_kart_track)*v/4;
						kart.angle = 2*track_angle-kart.angle;
					}
					damage+=abs(v-v_new);
					if(kart.mode==PLAYING && damage>=config.death_crash){

						stopwatch=-1;
						kart.mode=END;
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
			if(kart.mode==PLAYING){
				/*add_element((void**)&ghost_buf, &frame, &ghost_buf_len, 
						sizeof(float)*2);*/
				frame+=1;
				ghost_buf = realloc(ghost_buf,3*sizeof(float)*frame);
				ghost_buf[(frame*3-3)] = kart.angle;
				ghost_buf[(frame*3-2)] = kart.x;
				ghost_buf[(frame*3-1)] = kart.y;
				stopwatch=al_get_time()-start_time;
			}
			al_clear_to_color(al_map_rgb(0,0,0));
			if(found_ghost_file){
				karts[1].angle = record_ghost_buf[frame*3];
				karts[1].x=record_ghost_buf[frame*3+1];
				karts[1].y=record_ghost_buf[frame*3+2];
				n_karts=2;
			}
			karts[player_number]=kart;
			drawframe(n_karts,player_number, karts, scale,track, track_angle);
			//Draw hearts
			if(config.show_hearts){
				float heart_width = 10;
				float heart_height = 10;
				float heart_gap = 2;
				int am_hearts = 
					(int)ceil(
						(config.death_crash-damage)
						/config.life_per_heart
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
			
			if(kart.mode==COUNTDOWN){
				char text[10];
				sprintf(text, "%d", count_val);
				al_draw_text(splash, al_map_rgb(255, 255, 255), screen_width/2-
						al_get_text_width(splash,text)/2, 
					screen_height/2-al_get_font_ascent(splash)/2,0, text);
			}
				
			

			char infotext[200];
			infotext[0]='\0';
			if(config.show_speed){
				char speedtext[20];
				sprintf(speedtext, "speed=%f, ", v);
				strcat(infotext, speedtext);
			}
			if(config.show_kart_angle){
				char angletext[20];
				sprintf(angletext, "angle=%f, ", kart.angle);
				strcat(infotext, angletext);
			}
			if(config.show_scale){
				char scaletext[20];
				sprintf(scaletext, "scale=%f, ", scale);
				strcat(infotext, scaletext);
			}
			if(config.show_track_angle){
				char track_angletext[20];
				sprintf(track_angletext, "angle=%f, ", track_angle);
				strcat(infotext, track_angletext);
			}
			if(config.show_damage){
				char damagetext[20];
				sprintf(damagetext, "damage=%f, ", damage);
				strcat(infotext, damagetext);
			}
			if(config.show_x_pos){
				char x_postext[20];
				sprintf(x_postext, "xpos=%f, ", kart.x);
				strcat(infotext, x_postext);
			}
			if(config.show_y_pos){
				char y_postext[20];
				sprintf(y_postext, "ypos=%f, ", kart.y);
				strcat(infotext, y_postext);
			}
			if(config.show_segment){
				char segmenttext[20];
				sprintf(segmenttext, "segment=%d, ", cur_segment);
				strcat(infotext, segmenttext);
			}
			if(config.show_stopwatch){
				char timetext[20];
				sprintf(timetext, "time=%s, ", SecToString(stopwatch));
				strcat(infotext, timetext);
			}
			if(config.show_lap){
				char lap_text[20];
				sprintf(lap_text, "lap=%d, ", lap);
				strcat(infotext, lap_text);
			}
			if(connection==NONE && config.show_record &&am_records>0){
				char recordtext[20];
				sprintf(recordtext, "record=%s, ", 
						SecToString(records[0].time));
				strcat(infotext, recordtext);
			}

			al_draw_text(font, al_map_rgb(255, 255, 255), 0, 0, 0, infotext);

			if(kart.mode==END){
				char* complete_text;
				if(stopwatch==-1)complete_text="YOU CRASHED!";
				else if(connection==NONE)
					complete_text=SecToString(stopwatch);
				else{
					complete_text = malloc(sizeof(char)*2);
					sprintf(complete_text, "%d", position);
				}
						

				al_draw_text(splash, al_map_rgb(255, 255, 255), screen_width/2-
						al_get_text_width(splash,complete_text)/2, 
						screen_height/2-al_get_font_ascent(splash)/2,0, 
						complete_text);
				free(complete_text);
			}
			al_flip_display();

		}
	}
	al_destroy_bitmap(half_heart);
	al_destroy_bitmap(full_heart);

	
	al_destroy_timer(countdown);

	al_destroy_font(splash);

	if(connection==NONE)
		free(records);
	if(found_ghost_file){
		free(record_ghost_buf);
	}
	
}

void singleplayer_race(TRACK_DATA *track, char* filename){
	race(NONE,0,0,NULL,track, filename);
}

void server_race(int server_socket, int max_sd, node_t*sockets_head,
		TRACK_DATA *track, char* filename){
	race(SERVER, server_socket, max_sd, sockets_head, track, filename);
}

void connect_server_race(int client_socket, uint16_t player_number){
	race(CONNECT_TO_SERVER, client_socket,0,NULL,NULL,NULL);
}


char * SecToString(double  secs){
	int milsecs  = (int)(secs*1000);
	char *ret=malloc(sizeof(char)*20);
	sprintf(ret, "%02d:%02d.%03d", (int)(secs/60), ((int)secs)%60, milsecs%1000);
	return ret;
}

// vim: cc=100
