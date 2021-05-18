//If true it needs to redraw
#ifdef __cplusplus
#define _Bool bool
extern "C"{
#endif
_Bool handle_display(_Bool first/*Is true when this is the first loop*/,ALLEGRO_FONT* font);
#ifdef __cplusplus
}
#endif
// vim: cc=100
