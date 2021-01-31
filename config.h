
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
	float	death_crash;

	float	max_wheel_angle;

	float	max_F;
	float	mass;

	int	kart_color_r;
	int	kart_color_g;
	int	kart_color_b;
	
	float	kart_wheel_radius;
	
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
//	_Bool show_record;
	_Bool show_map;
	_Bool show_lap;

	float	life_per_heart;

	//ghost
	int	ghost_color_r;
	int	ghost_color_g;
	int	ghost_color_b;

}CONFIG;
void get_config(CONFIG* config);
_Bool get_config_bool(const ALLEGRO_CONFIG *config, const char *section, const char *key);
