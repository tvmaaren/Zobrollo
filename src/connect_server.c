#include <allegro5/allegro5.h>
#include <allegro5/allegro_font.h>

#include "display.h"
#include "config.h"
#include "gui.h"
#include "connect_server.h"

void wait_for_server(CONFIG* config, ALLEGRO_DISPLAY* disp, ALLEGRO_EVENT* event,
		ALLEGRO_EVENT_QUEUE *queue,ALLEGRO_FONT* font){
	bool first=true;
	int screen_width; int screen_height;
	while(true){
		bool EndProgram=false;
		bool redraw = false;
		handle_display(&screen_width,&screen_height,first, disp,event, queue, font);
		if(event->type == ALLEGRO_EVENT_KEY_DOWN 
				&& event->keyboard.keycode == ALLEGRO_KEY_ESCAPE)
			return;
		if(first|al_is_event_queue_empty(queue)){
			al_clear_to_color(al_map_rgb(0,0,0));
			draw_text(config->font_name, "Waiting untill host starts race", 
					config->button_text_color, screen_width/2, screen_height/2,
					screen_width/2, screen_height/2);

			al_flip_display();
		}
		al_wait_for_event(queue, event);
	}
	
}
