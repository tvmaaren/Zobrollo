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



#include <stdio.h>
#include <stdlib.h>

void must_init(_Bool test, const char *description)
{
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

