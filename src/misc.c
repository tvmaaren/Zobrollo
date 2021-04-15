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


#include <allegro5/allegro5.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "misc.h"

void must_init(_Bool test, const char *description){
	if(test) return;
	
	fprintf(stderr,"couldn't initialize %s\n", description);
	exit(1);
}

//Adds ten elements to the list if this is necessary.
void add_element(void** list, int *required, int *available, size_t element_size){
	(*required)++;
	if(*required>*available){
		(*available)+=10;
		*list = realloc(*list,*available*element_size);
	}
	return;
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

//Deletes the node that comes after the specified node from the linked list
void del_node(node_t* node){
	node_t* next=node->next->next;
	free(node->next);
	node->next = next;
}


//Adds a node after the specified node in the linked list and returns a pointer to that new node
node_t* add_node(node_t* node){
	node_t* next = node->next;
	node->next = malloc(sizeof(node_t));
	node->next->next = next;
	return (node->next);
}

// vim: cc=100
