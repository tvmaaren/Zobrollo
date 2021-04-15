//If true it needs to redraw
#ifdef __cplusplus
#define _Bool bool
extern "C"{
#endif
_Bool handle_display(int* screen_width, int* screen_height, 
		_Bool first/*Is true when this is the first loop*/, ALLEGRO_DISPLAY* disp, 
		ALLEGRO_EVENT*event, ALLEGRO_EVENT_QUEUE* queue, ALLEGRO_FONT* font);
#ifdef __cplusplus
}
#endif
// vim: cc=100
