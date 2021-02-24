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
long with this program.  If not, see <https://www.gnu.org/licenses/>.

e-mail:thomas.v.maaren@outlook.com
*/

#include <stdio.h>
#include <stdlib.h>

#include <allegro5/allegro5.h>
#include <allegro5/allegro_primitives.h>

#include <math.h>

#include "drawtrack.h"
#include "misc.h"


typedef struct{
	float length;
}SIMPLE_LINE_SEGMENT;

typedef struct{
	float radius;
	float angle;
	_Bool left_right;
}SIMPLE_CIRCLE_SEGMENT;

void cart2pol(float x, float y, float *angle, float *dist){
	*dist=sqrt(x*x+y*y);
	*angle=atan(y/x);
	if(x<0)*angle+=M_PI;
}

void prerror(ALLEGRO_FILE *file, char *message, int line_number){
	fprintf(stderr, "%s on line %d of the track file\n", message, line_number);
	exit(1);
}


//Gets the next word in the file. It also gives the line number that it is currently on
//returns 0 if it didn't hit a newline or end of file
//returns 1 if it hit at least one newline before hitting a word
//returns 2 if it hit end of the file before hitting a word
int getword(ALLEGRO_FILE *file, char**pword, int*pline_num){
	int ret=0;
	
	int word_len=0;
	int str_len=10;
	*pword = malloc(sizeof(char)*str_len);

	_Bool start_word=false;
	while(true){
		int ch = al_fgetc(file);
		if(ch=='\n'){
			if(start_word){
				//It undoes the getc as it is necessary to
				//check if a \n happened before the word.
				//We don't really need to know if it happened after.
				al_fungetc(file, ch);
				break;
			}else{
				ret=1;
				*pline_num++;
			}
		}
		else if(ch==' ' && start_word)break;
		else if(ch==EOF){
			if(!start_word)return(2);
			break;
		}else if(ch!=' '){
			start_word=true;
			add_element((void*)pword, &word_len, &str_len, sizeof(char));
			(*pword)[word_len-1]=(char)ch;
		}
	}
	//end the string
	add_element((void**)pword, &word_len, &str_len, sizeof(char));
	(*pword)[word_len-1]='\0';
	return ret;
}
int getnum(ALLEGRO_FILE *file, float*pnum, int*line_num){
	int ret;
	char *word;
	ret = getword(file, &word, line_num);
	if(ret!=2){
		errno = 0;
		*pnum = strtof(word,NULL);
		if(errno!=0)prerror(file, "Invalid number", *line_num);
	}
	return ret;
}


int GetSimpleSegments(void*** segments, _Bool** segment_types, float* ptrackwidth, ALLEGRO_FILE *file){
	
	int line_num=1;

	int am_segments =0;
	int segment_list_size = 10;
	int types_list_size = 10;
	*segments = malloc(sizeof(void*)*segment_list_size);
	*segment_types = malloc(sizeof(_Bool)*types_list_size);

	//get the width of the track
	if(getnum(file, ptrackwidth, &line_num)==2){
		return am_segments;
	}

	int segment_i=0;
	//loop through the segments
	while(true){
		//get the segment type
		char* type;
		int ret = getword(file, &type, &line_num);
		if(ret==2){
			return am_segments;
		}
		if(ret!=1)prerror(file, "Too many words",line_num);
			
		add_element((void*)segments, &am_segments, &segment_list_size, sizeof(void*));
		am_segments--;//simple hack to make sure am_segments keeps the same value
		add_element((void*)segment_types, &am_segments, &types_list_size, sizeof(_Bool));

		if(type[0]=='s'){

			//straight
			(*segment_types)[am_segments-1]=STRAIGHT;

			//get the length of the straight
			SIMPLE_LINE_SEGMENT* straight=malloc(sizeof(SIMPLE_LINE_SEGMENT));
			ret =getnum(file,&(straight->length),&line_num);
			if(ret>0)prerror(file, "Not enough parameters for straight", line_num);
			//add it to the segment list
			(*segments)[segment_i]=(void*)straight;

		}else if(type[0]=='c'){

			//circle
			(*segment_types)[am_segments-1]=CIRCLE;

			SIMPLE_CIRCLE_SEGMENT* circle=malloc(sizeof(SIMPLE_CIRCLE_SEGMENT));


			//get radius
			ret =getnum(file,&(circle->radius),&line_num);
			if(ret>0)prerror(file, "Not enough parameters for circle", line_num);

			//get delta angle
			float angle_degrees;
			ret =getnum(file,&angle_degrees,&line_num);
			circle->angle=angle_degrees/180*M_PI;
			if(ret>0)prerror(file, "Not enough parameters for circle", line_num);

			//get left or right
			char *left_right;
			ret =getword(file,&left_right,&line_num);
			if(ret>0)prerror(file, "Not enough parameters for circle", line_num);
			
			if (left_right[0]=='l' || left_right[0]=='L')circle->left_right=RIGHT;
			else if(left_right[0]='r' || left_right[0]=='R')circle->left_right=LEFT;
			else prerror(file,"Invalid direction", line_num);


			(*segments)[segment_i]=(void*)circle;
		}else{
			prerror(file,"Invalid segment type", line_num);
		}
		segment_i++;
	}
}
void loadtrack(ALLEGRO_FILE* file, TRACK_DATA* track_data){
		void ** simple_segments;
		track_data->n_segments = GetSimpleSegments(&simple_segments, 
				&(track_data->segment_types), &(track_data->trackwidth), file);
		track_data->segments = malloc(sizeof(void*)*track_data->n_segments);
		
		//the track starts at (0,0) pointing upwards
		float track_y=0;
		float track_x=0;
		float track_angle=M_PI/2;

		//this function shall approximate these values, because this will make it easier 
		//to draw the map of the track correctly
		track_data->max_x=0;
		track_data->max_y=0;
		track_data->min_x=0;
		track_data->min_y=0;

		//go through all the segments
		int i=0;
		while(i<track_data->n_segments){
			if(track_x>track_data->max_x)track_data->max_x=track_x;
			if(track_x<track_data->min_x)track_data->min_x=track_x;
			if(track_y>track_data->max_x)track_data->max_y=track_y;
			if(track_y<track_data->min_x)track_data->min_y=track_y;

			if(track_data->segment_types[i]){//circle
				SIMPLE_CIRCLE_SEGMENT*simple_segment=(SIMPLE_CIRCLE_SEGMENT*)
					simple_segments[i];
				CIRCLE_SEGMENT* circle = malloc(sizeof(CIRCLE_SEGMENT));
				if(simple_segment->left_right){
					//left
					circle->start_angle=track_angle-M_PI/2;
					track_angle+=simple_segment->angle;
					circle->delta_angle=simple_segment->angle;
				}else{
					//right
					circle->start_angle=track_angle+M_PI/2;
					track_angle-=simple_segment->angle;
					circle->delta_angle=-simple_segment->angle;
				}
				
				circle->r_inner=simple_segment->radius-track_data->trackwidth/2;
				circle->r_outer=simple_segment->radius+track_data->trackwidth/2;
				circle->r_mid  =simple_segment->radius;

				circle->midx=cos(circle->start_angle-M_PI)*
					simple_segment->radius+track_x;
				circle->midy=sin(circle->start_angle-M_PI)*
					simple_segment->radius+track_y;

				track_x=circle->midx+cos(circle->start_angle+circle->delta_angle)
					*simple_segment->radius;
				track_y=circle->midy+sin(circle->start_angle+circle->delta_angle)
					*simple_segment->radius;

				(track_data->segments)[i]=circle;

			}else{//line
				SIMPLE_LINE_SEGMENT*simple_segment=(SIMPLE_LINE_SEGMENT*)
					simple_segments[i];
				LINE_SEGMENT* line = malloc(sizeof(LINE_SEGMENT));

				float d_x=cos(track_angle+M_PI/2)* track_data->trackwidth/2;
				float d_y=sin(track_angle+M_PI/2)* track_data->trackwidth/2;

				//begin of segment
				line->inner.x1=track_x+d_x;
				line->inner.y1=track_y+d_y;

				line->outer.x1=track_x-d_x;
				line->outer.y1=track_y-d_y;

				track_x+=cos(track_angle)*simple_segment->length;
				track_y+=sin(track_angle)*simple_segment->length;
				line->length = simple_segment->length;
				line->angle = track_angle;

				//end of segment
				line->inner.x2=track_x+d_x;
				line->inner.y2=track_y+d_y;

				line->outer.x2=track_x-d_x;
				line->outer.y2=track_y-d_y;

				track_data->segments[i]=line;
			}
			//free the simple segment
			free(simple_segments[i]);

			i++;
		}
		

		free(simple_segments);
}



//make sure you have initialised allegro before running this function
void drawtrack(float x, float y, float angle, float scale, int width, int height, 
		TRACK_DATA track_data){
	float dist_milestone=0;

	ALLEGRO_TRANSFORM transform;
	al_identity_transform(&transform);
	al_translate_transform(&transform, -x,-y );
	al_rotate_transform(&transform , angle);
	al_scale_transform(&transform, scale,scale);
	al_translate_transform(&transform, width/2,height/2);
	al_use_transform(&transform);

	//The width of the track border is never allowed to become
	//less than 2 pixels after it has been transformed
	if(track_data.border_width*scale<1)
		track_data.border_width=1/scale;
	
	//finish line
	al_draw_line(-track_data.trackwidth/2,0,track_data.trackwidth/2,0, track_data.border_color, 
			track_data.border_width);

	int i=0;
	while(i<track_data.n_segments){
		if(track_data.segment_types[i]){//circle segment
			CIRCLE_SEGMENT*segment=(CIRCLE_SEGMENT*)track_data.segments[i];

			al_draw_arc(segment->midx,segment->midy,segment->r_inner, 
					segment->start_angle, segment->delta_angle,
					track_data.border_color, track_data.border_width);
			al_draw_arc(segment->midx,segment->midy,segment->r_outer,
					segment->start_angle, segment->delta_angle,
					track_data.border_color, track_data.border_width);
			
			//draw milestones
			if(track_data.milestones){
				float length=abs(segment->delta_angle*segment->r_mid);
				while(dist_milestone<length){
					//calculate the angle from the center
					float stone_angle = segment->start_angle+segment->delta_angle*
						dist_milestone/length;
					
					//draw inner milestone
					al_draw_line(cos(stone_angle)*segment->r_inner+segment->midx,
						sin(stone_angle)*segment->r_inner+segment->midy,
						cos(stone_angle)*(segment->r_inner-track_data.milestone_size)+
							segment->midx,
						sin(stone_angle)*(segment->r_inner-track_data.milestone_size)+
							segment->midy,
						track_data.border_color, track_data.border_width);

					//draw outer milestone
					al_draw_line(cos(stone_angle)*segment->r_outer+segment->midx,
						sin(stone_angle)*segment->r_outer+segment->midy,
						cos(stone_angle)*(segment->r_outer+
							track_data.milestone_size)+segment->midx,
						sin(stone_angle)*(segment->r_outer+
							track_data.milestone_size)+segment->midy,
						track_data.border_color, track_data.border_width);

					dist_milestone+=track_data.milestone_interval;
				}
				dist_milestone-=length;
			}



		}else{//line segment
			//it will draw the two trackborders begginning with the left
			LINE_SEGMENT*segment=(LINE_SEGMENT*)track_data.segments[i];

			//inner
			al_draw_line(segment->inner.x1,segment->inner.y1,segment->inner.x2,
				segment->inner.y2,track_data.border_color, track_data.border_width);

			//outer	
			al_draw_line(segment->outer.x1,segment->outer.y1,segment->outer.x2,
				segment->outer.y2,track_data.border_color, track_data.border_width);

			if(track_data.milestones){
				//draw the milestones
				float d_x=	cos(segment->angle+M_PI/2)*track_data.trackwidth/2;
				float d_y=	sin(segment->angle+M_PI/2)*track_data.trackwidth/2;
				float d_x_out=	cos(segment->angle+M_PI/2)*(track_data.trackwidth/2+
						track_data.milestone_size);
				float d_y_out=	sin(segment->angle+M_PI/2)*(track_data.trackwidth/2+
						track_data.milestone_size);
				
				while(dist_milestone<segment->length){
					float x_track=cos(segment->angle)*dist_milestone+segment->
						inner.x1-d_x;
					float y_track=sin(segment->angle)*dist_milestone+segment->
						inner.y1-d_y;

					//draw first milestone
					al_draw_line(x_track+d_x, y_track+d_y,x_track+d_x_out, 
						y_track+d_y_out,track_data.border_color, 
						track_data.border_width);

					//draw second milestone
					al_draw_line(x_track-d_x, y_track-d_y,x_track-d_x_out, 
						y_track-d_y_out, track_data.border_color, 
						track_data.border_width);
					dist_milestone+=track_data.milestone_interval;
				}
				dist_milestone-=segment->length;
			}

					
		}
		i++;
	}
	al_identity_transform(&transform);
	al_use_transform(&transform);
}
// vim: cc=100
