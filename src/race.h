typedef enum {NONE=0, CONNECT_TO_SERVER, SERVER} CONNECTION;

void singleplayer_race(TRACK_DATA *track, char* filename, CONFIG* config, ALLEGRO_DISPLAY* disp, PATHS *paths, 
		ALLEGRO_EVENT* event, ALLEGRO_EVENT_QUEUE *queue, ALLEGRO_FONT* font);
// vim: cc=100
