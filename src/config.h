
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
void get_config(CONFIG* config, const ALLEGRO_CONFIG* cfg);
_Bool get_config_bool(const ALLEGRO_CONFIG *config, const char *section, const char *key);
// vim: cc=100
