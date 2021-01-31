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


#include <stdlib.h>
#include <stdio.h>
#include <allegro5/allegro5.h>
#include "config.h"

//reads the config.cfg file


void get_config(CONFIG* config){
	

	//get info from config file
	//TODO: Have better error detection
	const ALLEGRO_CONFIG* cfg= al_load_config_file("config.cfg");

	config->font_name= 	al_get_config_value(cfg, "", "font");
	
	config->laps= 		atoi(al_get_config_value(cfg,"", "laps"));
	config->fps=		atof(al_get_config_value(cfg, "", "fps"));
	config->sec_before_start=atoi(al_get_config_value(cfg, "", "sec_before_start"));

	
	config->window_width= 	atoi(al_get_config_value(cfg, "window", "width"));
	config->window_height= 	atoi(al_get_config_value(cfg, "window", "height"));

	
	config->kart_width= 	atof(al_get_config_value(cfg, "kart", "width"));
	config->kart_height= 	atof(al_get_config_value(cfg, "kart", "height"));

	config->accelleration= 	atof(al_get_config_value(cfg, "kart", "accelleration"));
	config->back_accelleration=atof(al_get_config_value(cfg,"kart", "back_accelleration"));
	config->break_speed= 	atof(al_get_config_value(cfg,"kart", "break_speed"));
	config->death_crash= 	atof(al_get_config_value(cfg,"kart", "death_crash"));

	config->max_wheel_angle= atof(al_get_config_value(cfg,"kart", "max_wheel_angle"));

	config->max_F=atof(al_get_config_value(cfg,"kart", "max_F"));
	config->mass=atof(al_get_config_value(cfg,"kart", "mass"));

	config->kart_color_r=atoi(al_get_config_value(cfg,"kart", "color_r"));
	config->kart_color_g=atoi(al_get_config_value(cfg,"kart", "color_g"));
	config->kart_color_b=atoi(al_get_config_value(cfg,"kart", "color_b"));
	
	config->kart_wheel_radius=atof(al_get_config_value(cfg,"kart", "wheel_radius"));


	//information
	config->show_speed=get_config_bool(cfg, "information", "speed");
	config->show_kart_angle=get_config_bool(cfg, "information", "kart_angle");
	config->show_scale=get_config_bool(cfg, "information", "scale");
	config->show_track_angle=get_config_bool(cfg, "information", "track_angle");
	config->show_damage=get_config_bool(cfg, "information", "damage");
	config->show_x_pos=get_config_bool(cfg, "information", "x_pos");
	config->show_y_pos=get_config_bool(cfg, "information", "y_pos");
	config->show_segment=get_config_bool(cfg, "information", "segment");
	config->show_stopwatch=get_config_bool(cfg, "information", "stopwatch");
	//config->show_record=get_config_bool(cfg, "information", "record");
	config->show_map=get_config_bool(cfg, "information", "map");
	config->show_lap=get_config_bool(cfg, "information", "lap");

	config->life_per_heart= atof(al_get_config_value(cfg, "information", "life_per_heart"));

	//ghost
	config->ghost_color_r=atoi(al_get_config_value(cfg,"ghost", "color_r"));
	config->ghost_color_g=atoi(al_get_config_value(cfg,"ghost", "color_g"));
	config->ghost_color_b=atoi(al_get_config_value(cfg,"ghost", "color_b"));
}

_Bool get_config_bool(const ALLEGRO_CONFIG *config, const char *section, const char *key){
	const char * string = al_get_config_value(config, section, key);
	if(!strcmp(string, "true"))return(true);
	else if(!strcmp(string, "false"))return(false);
	fprintf(stderr, "\"%s\" is an invalid option for \"%s\" in section \"%s\".\n" ,string, 
			key, section);
	exit(1);
}
