typedef enum {COUNTDOWN, PLAYING, END} MODE;

typedef struct{
	float angle;
	float x;
	float y;
	MODE mode;
	
	ALLEGRO_COLOR color;
}kart_t;

void drawkart(float x,float y, float angle, float scale, int width, int height, 
		kart_t kart, CONFIG* config);

// vim: cc=100
