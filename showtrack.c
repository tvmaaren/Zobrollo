#include <stdio.h>
#include <stdlib.h>

#include <allegro5/allegro5.h>
#include <allegro5/allegro_primitives.h>

#include "drawtrack.h"

#include <math.h>


#define move_per_sec_per_scale 500.0
#define rotate_per_sec M_PI/2
#define scale_per_sec 1.5

#define fps 30.0
#define screen_width 800
#define screen_height 800

#define track_color_r 255
#define track_color_g 0
#define track_color_b 0
#define track_border_width 2

typedef struct{
	float radius;
	float angle;
	_Bool left_right;
}circle_segment;

typedef struct{
	float length;
}line_segment;

void must_init(bool test, const char *description)
{
    if(test) return;

    fprintf(stderr,"couldn't initialize %s\n", description);
    exit(1);
}

//file read functions
//Returns:
//	0:	Everything went well
//	1:	Hit a newline before word
//	2:	End of file
//	3:	Hit a newline after word


//goes back one step in the file
void goback(FILE*file);

//automaticly chooses between realloc or malloc
void* add_one_element(void *in, int am_elements, size_t size_element);

int getnum(FILE *file, float*out, int*line_num);
int getword(FILE *file, char**out, int*line_num);
void prerror(char *message, int line_number);

void rotate(float* point, float angle, float width, float height);

int GetSegments(void*** segments, _Bool** segment_types, float* trackwidth, FILE *file);

void main(int argc, char**argv){
	ALLEGRO_CONFIG* cfg = al_load_config_file("config.cfg");
	if(argc!=2){
		fprintf(stderr, "Invalid amount of parameters: %s <file>\n", argv[0]);
		exit(1);
	}
	void** segments;
	_Bool* segment_types;//To know which segment type it has to be cast to
	int n_segments;
	
	FILE *file = fopen(argv[1], "r");
	if(file==NULL){
		fprintf(stderr,"Could not open %s\n", argv[1]);
		return;
	}

	float trackwidth;
	TRACK_DATA track;
	track.milestone_interval = 5;
	track.milestone_size = 2;
	//track.border_color = al_map_rgb(track_color_r,track_color_g,track_color_b);
	track.border_color.r= 1;
	track.border_color.g= 0;
	track.border_color.b= 0;
	track.border_color.a= 1.0;
	track.border_width = track_border_width;
	loadtrack(file, &track);
	
	float scale =1.0;
	//finish line is always at (0,0) and the track there always points upwards
	float x=0;
	float y=0;
	float angle=0;

	//alegro stuff

	//initialize and check for prerrors
	must_init(al_init(),"allegro");
	must_init(al_install_keyboard(),"couldn't initialize keyboard\n");

	ALLEGRO_TIMER* timer = al_create_timer(1.0 / fps);
	must_init(timer,"timer");
	
	ALLEGRO_EVENT_QUEUE* queue = al_create_event_queue();
	must_init(queue,"queue");

	ALLEGRO_DISPLAY* disp = al_create_display(screen_width, screen_height);
	must_init(disp,"couldn't initialize display\n");

	ALLEGRO_EVENT event;
	al_start_timer(timer);

	must_init(al_init_primitives_addon(), "primitives");

	al_register_event_source(queue, al_get_keyboard_event_source());
    	al_register_event_source(queue, al_get_display_event_source(disp));
    	al_register_event_source(queue, al_get_timer_event_source(timer));

	#define KEY_SEEN     1
	#define KEY_RELEASED 2

	unsigned char key[ALLEGRO_KEY_MAX];
	memset(key, 0, sizeof(key));


	while(1){
		al_wait_for_event(queue,&event);
		_Bool EndProgram=0;
		_Bool redraw=0;
		switch(event.type){
			case(ALLEGRO_EVENT_TIMER):
				redraw = 1;
				if(key[ALLEGRO_KEY_UP]){
					float distance=move_per_sec_per_scale/fps*scale;
					y+=sin(-angle-M_PI/2)*distance;
					x+=cos(-angle-M_PI/2)*distance;
				}
				if(key[ALLEGRO_KEY_DOWN]){
					float distance=move_per_sec_per_scale/fps*scale;
					y+=sin(-angle+M_PI/2)*distance;
					x+=cos(-angle+M_PI/2)*distance;
				}
				if(key[ALLEGRO_KEY_LEFT]){
					float distance=move_per_sec_per_scale/fps*scale;
					y+=sin(-angle+M_PI)*distance;
					x+=cos(-angle+M_PI)*distance;

				}
				if(key[ALLEGRO_KEY_RIGHT]){
					float distance=move_per_sec_per_scale/fps*scale;
					y+=sin(-angle)*distance;
					x+=cos(-angle)*distance;
				}
				if(key[ALLEGRO_KEY_ESCAPE]){
					EndProgram=1;
				}
				if(key[ALLEGRO_KEY_MINUS]){
					scale=scale*powf(scale_per_sec, -1.0/fps);
				}
				if(key[ALLEGRO_KEY_EQUALS]){
					scale=scale*powf(scale_per_sec, 1.0/fps);
				}
				if(key[ALLEGRO_KEY_R]){
					angle-=rotate_per_sec/fps;
				}
				if(key[ALLEGRO_KEY_E]){
					angle+=rotate_per_sec/fps;
				}
				for(int i = 0; i < ALLEGRO_KEY_MAX; i++)
			    		key[i] &= KEY_SEEN;
				break;
			case(ALLEGRO_EVENT_KEY_DOWN):
				key[event.keyboard.keycode] = KEY_SEEN | KEY_RELEASED;
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
			al_clear_to_color(al_map_rgb(0,0,0));
			//Draw entire track
			//TODO Draw only what is neccessary
			//printf("%f,%f,%f,%f\n", track.border_color.r, track.border_color.g,track.border_color.b,track.border_color.a);
			drawtrack(x,y,angle,scale,screen_width,screen_height,track);


			al_flip_display();
		}
	}
}
	
int GetSegments(void*** segments, _Bool** segment_types, float* trackwidth, FILE *file){
	int line_num=1;
	int am_segments =0;
	if(getnum(file, trackwidth, &line_num)==2){
		return am_segments;
	}

	int segmenti=0;
	//loop through the segments
	while(1){
		//segment type
		char* type;
		int ret = getword(file, &type, &line_num);
		if(ret==2){
			return am_segments;
		}
		if(ret!=1)prerror("Too many words",line_num);
		if(type[0]=='s'){
			//straight
			*segments=add_one_element((void*)*segments, am_segments, sizeof(void*));
			*segment_types=add_one_element((void*)*segment_types, am_segments, sizeof(_Bool));
			am_segments++;

			(*segment_types)[am_segments-1]=0;

			line_segment* straight=malloc(sizeof(line_segment));
			ret =getnum(file,&(straight->length),&line_num);
			if(ret>0)prerror("Not enough parameters for straight", line_num);
			//add it to the segment list
			(*segments)[segmenti]=(void*)straight;
		}else if(type[0]=='c'){
			//circle
			*segments=add_one_element((void*)*segments, am_segments, sizeof(void*));
			*segment_types=add_one_element((void*)*segment_types, am_segments, sizeof(_Bool));
			am_segments++;
			
			(*segment_types)[am_segments-1]=1;

			circle_segment* circle=malloc(sizeof(circle_segment));


			//get radius
			ret =getnum(file,&(circle->radius),&line_num);
			if(ret>0)prerror("Not enough parameters for circle", line_num);

			//get delta angle
			float angle_degrees;
			ret =getnum(file,&angle_degrees,&line_num);
			circle->angle=angle_degrees/180*M_PI;
			if(ret>0)prerror("Not enough parameters for circle", line_num);

			//get left or right
			char *left_right;
			ret =getword(file,&left_right,&line_num);
			if(ret>0)prerror("Not enough parameters for circle", line_num);
			
			if (left_right[0]=='l' || left_right[0]=='L')circle->left_right=0;
			else if(left_right[0]='r' || left_right[0]=='R')circle->left_right=1;
			else prerror("Invalid direction", line_num);


			(*segments)[segmenti]=(void*)circle;
		}else{
			prerror("Invalid segment type", line_num);
		}
		segmenti++;
	}
}
void* add_one_element(void *in, int am_elements, size_t size_element){
	//malloc
	if(am_elements==0)return(malloc(size_element));
	else return(realloc(in, (am_elements+1)*size_element));
}



void goback(FILE*file){
	fseek(file, -1, SEEK_CUR);
}

void rotate(float* point, float angle, float width, float height){
	float x=point[0];
	float y=point[1];

	float d_x=point[0]-width/2;
	float d_y=point[1]-height/2;
	float dist_mid=sqrt(d_x*d_x+d_y*d_y);
	float or_angle=atan(d_y/d_x);
	if(d_x<0)or_angle+=M_PI;
	point[0]=cos(angle+or_angle)*dist_mid+width/2;
	point[1]=sin(angle+or_angle)*dist_mid+height/2;
}
