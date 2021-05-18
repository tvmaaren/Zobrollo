#ifdef __cplusplus
extern "C"{
#endif
typedef enum {NONE=0, CONNECT_TO_SERVER, SERVER} CONNECTION;

void singleplayer_race(TRACK_DATA *track, char* filename);
void server_race(int server_socket, int max_sd, node_t*sockets_head,
		TRACK_DATA *track, char* filename);
void connect_server_race(int client_socket, uint16_t player_number);
#ifdef __cplusplus
}
#endif
// vim: cc=100
