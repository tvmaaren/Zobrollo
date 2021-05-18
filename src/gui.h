_Bool handle_click_box(float mouse_x,float mouse_y,float x1,float y1,float x2,float y2,
		char* text);
_Bool handle_click_box_relative(float mouse_x,float mouse_y,float x1frac,float y1frac,float x2frac,
		float y2frac, char* text);
void draw_text(const char* name, char* text, ALLEGRO_COLOR color, float x, 
		float y, int max_width, int max_height);

// vim: cc=100
