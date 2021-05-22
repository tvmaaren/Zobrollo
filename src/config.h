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

#ifdef __cplusplus
#define _Bool bool
extern "C"{
#endif

typedef struct{
	
	const char*font_name;
	
	int 	laps;
	float 	fps;
	int 	sec_before_start;

	//window
	int 	window_width;
	int	window_height;

	//kart
	float	kart_width;
	float	kart_height;

	float	accelleration;
	float	back_accelleration;
	float	break_speed;
	float	speed_decrease;
	float	death_crash;

	float	max_wheel_angle;

	float	max_F;
	float	mass;

	int	kart_color_r;
	int	kart_color_g;
	int	kart_color_b;
	ALLEGRO_COLOR kart_color;
	
	float	kart_wheel_radius;

	//camera
	enum {AS_START=0, RELATIVE_TO_KART, RELATIVE_TO_TRACK} camera_angle;
	
	//information
	_Bool show_speed;
	_Bool show_kart_angle;
	_Bool show_scale;
	_Bool show_track_angle;
	_Bool show_damage;
	_Bool show_x_pos;
	_Bool show_y_pos;
	_Bool show_segment;
	_Bool show_stopwatch;
	_Bool show_record;
	_Bool show_map;
	_Bool show_lap;
	_Bool show_hearts;

	float	life_per_heart;

	//ghost
	_Bool 	save_ghost;
	_Bool	play_against_ghost;
	
	//button

	float button_border_thickness;

	int button_border_color_r;
	int button_border_color_g;
	int button_border_color_b;
	ALLEGRO_COLOR button_border_color;

	int button_select_color_r;
	int button_select_color_g;
	int button_select_color_b;
	ALLEGRO_COLOR button_select_color;

	int button_text_color_r;
	int button_text_color_g;
	int button_text_color_b;
	ALLEGRO_COLOR button_text_color;

}CONFIG;
extern CONFIG config;
void get_config(const ALLEGRO_CONFIG* cfg);
_Bool get_config_bool(const ALLEGRO_CONFIG *config, const char *section, const char *key);

#ifdef __cplusplus
}
#endif

// vim: cc=100
