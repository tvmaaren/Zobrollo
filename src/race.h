typedef enum {NONE=0, CONNECT_TO_SERVER, SERVER} CONNECTION;

void singleplayer_race(TRACK_DATA *track, char* filename, CONFIG* config, ALLEGRO_DISPLAY* disp, PATHS *paths, 
		ALLEGRO_EVENT* event, ALLEGRO_EVENT_QUEUE *queue, ALLEGRO_FONT* font);
void server_race(int server_socket, int max_sd, node_t*sockets_head,
		TRACK_DATA *track, char* filename, CONFIG* config, 
		ALLEGRO_DISPLAY* disp,PATHS *paths,ALLEGRO_EVENT* event,ALLEGRO_EVENT_QUEUE *queue,
		ALLEGRO_FONT* font);
#ifdef __cplusplus
extern "C"{
#endif
void connect_server_race(int client_socket, uint16_t player_number, 
		CONFIG* config, ALLEGRO_DISPLAY* disp, 
		PATHS *paths,ALLEGRO_EVENT* event, ALLEGRO_EVENT_QUEUE *queue, ALLEGRO_FONT* font);
#ifdef __cplusplus
}
#endif
// vim: cc=100
