typedef struct{
	float angle;
	float x;
	float y;
	
	float kart_width;
 	float kart_height;
	ALLEGRO_COLOR color;
}kart_t;

void drawkart(float x,float y, float angle, float scale, int width, int height, 
		kart_t kart);

// vim: cc=100
