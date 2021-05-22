/*
Zobrollo is a 2d minimalistic top-view racing game
Copyright (C) 2021  Thomas van Maaren

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.

e-mail:thomas.v.maaren@outlook.com
*/

#include <Agui/Agui.hpp>
#include <Agui/Backends/Allegro5/Allegro5.hpp>

#include <Agui/Widgets/Button/Button.hpp>
#include <Agui/FlowLayout.hpp>

#include <stdlib.h>
#include <iostream>

#include "networking.h"
#include "config.h"
#include "misc.h"
#include "file_paths.h"
#include "join_server.h"
#include "ghost.h"
#include "gui.h"
#include "drawtrack.h"
#include "start_server.h"
#include "record.h"
#include "race.h"
#include "display.h"
#include "no_agui_menu.h"

#define FRAME_RATE 60

//Globals
ALLEGRO_DISPLAY *disp;
ALLEGRO_TIMER *timer;
ALLEGRO_EVENT event;
ALLEGRO_EVENT_QUEUE *queue;
int screen_width;
int screen_height;
PATHS paths;
CONFIG config;


class time_trial_listener : 
	public agui::ActionListener{
		public:
			virtual void actionPerformed(const agui::ActionEvent &evt){
				track_menu(singleplayer_race);
				al_flush_event_queue(queue);
			}
};
class multiplayer_listener : 
	public agui::ActionListener{
		public:
			virtual void actionPerformed(const agui::ActionEvent &evt){
				multiplayer_menu();
				al_flush_event_queue(queue);
			}
};
class records_listener : 
	public agui::ActionListener{
		public:
			virtual void actionPerformed(const agui::ActionEvent &evt){
				track_menu(show_record);
				al_flush_event_queue(queue);
			}
};
class quit_listener : 
	public agui::ActionListener{
		public:
			virtual void actionPerformed(const agui::ActionEvent &evt){
				exit(0);
			}
};



class WidgetCreator {
private:
	agui::Button time_trial;
	agui::Button multiplayer;
	agui::Button records;
	agui::Button quit;

	time_trial_listener time_trialAL;
	multiplayer_listener multiplayerAL;
	records_listener recordsAL;
	quit_listener quitAL;

	agui::Gui* mGui;
public:
	void rel_location_pos(){
		time_trial.setLocation(	screen_width*0.3,	screen_height*0.42);
		multiplayer.setLocation(screen_width*0.3,	screen_height*0.57);
		records.setLocation(	screen_width*0.3,	screen_height*0.72);
		quit.setLocation(	screen_width*0.51,	screen_height*0.72);
		
		time_trial.setSize(	screen_width*0.4,	screen_height*0.08);
		multiplayer.setSize(	screen_width*0.4,	screen_height*0.08);
		records.setSize(	screen_width*0.19,	screen_height*0.08);
		quit.setSize(		screen_width*0.19,	screen_height*0.08);
	}

	WidgetCreator(agui::Gui *gui){
		gui->add(&time_trial);
		gui->add(&multiplayer);
		gui->add(&records);
		gui->add(&quit);


		time_trial.setText("time trial");
		multiplayer.setText("multiplayer");
		records.setText("records");
		quit.setText("quit");


		time_trial.resizeToContents();
		multiplayer.resizeToContents();
		records.resizeToContents();
		quit.resizeToContents();
		
		time_trial.addActionListener(&time_trialAL);
		multiplayer.addActionListener(&multiplayerAL);
		records.addActionListener(&recordsAL);
		quit.addActionListener(&quitAL);


		rel_location_pos();
	}
};

WidgetCreator* creator;



int main(int argc, char *argv[]){
	must_init(al_init(),"Allegro");
	must_init(al_init_image_addon(),"image addon");
	must_init(al_init_font_addon(),"font addon");
	must_init(al_init_ttf_addon(),"ttf addon");
	must_init(al_init_primitives_addon(),"primitives addon");
	must_init(al_install_mouse(),"mouse");
	must_init(al_install_keyboard(),"keyboard");

	//get directories
	
	paths.home= getenv(home_var);
	
	std::string record_path = (std::string)paths.home+local_dir sep_str "records";
	paths.record = record_path.c_str();
	
	std::string ghost_path = (std::string)paths.home+local_dir sep_str "ghosts";
	paths.ghost = ghost_path.c_str();
	
	al_change_directory(data_dir);
	paths.data = al_get_current_directory();

	//Load configuration	
	std::string config_path = (std::string)paths.home+local_dir sep_str "config.cfg";

	const ALLEGRO_CONFIG* cfg = al_load_config_file(config_path.c_str());
	if(!cfg){
		cfg= al_load_config_file(data_dir sep_str "config.cfg");
		if(!cfg){
			fprintf(stderr, "Error: Could not find \"config.cfg\"\n");
			exit(1);
		}
	}
	get_config(cfg);
	
	// Start a timer to regulate speed
	timer = al_create_timer(1.0/config.fps);
	al_start_timer(timer);

	//show screen
	al_set_new_display_flags(ALLEGRO_RESIZABLE);
	disp = al_create_display(640,480);
	must_init(disp,"display");
	
	//show the mouse
	al_show_mouse_cursor(disp); 
	al_set_new_bitmap_flags(ALLEGRO_VIDEO_BITMAP);

	//Window Title
	queue = al_create_event_queue();

	agui::Font::setFontLoader(new agui::Allegro5FontLoader);
	agui::Allegro5Input* inputHandler = new agui::Allegro5Input();
	agui::Allegro5Graphics* graphicsHandler = new agui::Allegro5Graphics();
	agui::Color::setPremultiplyAlpha(true);
	agui::Gui *gui = new agui::Gui();
	gui->setInput(inputHandler);
	gui->setGraphics(graphicsHandler);
	agui::Font *defaultFont= agui::Font::load(config.font_name,16);
	agui::Widget::setGlobalFont(defaultFont);
	// Start the event queue to handle keyboard input, mouse and our timer
	al_register_event_source(queue, (ALLEGRO_EVENT_SOURCE*)al_get_keyboard_event_source());
	al_register_event_source(queue, (ALLEGRO_EVENT_SOURCE*)al_get_mouse_event_source());
	al_register_event_source(queue, (ALLEGRO_EVENT_SOURCE*)timer);
	al_register_event_source(queue, (ALLEGRO_EVENT_SOURCE*)disp);

	screen_width=al_get_display_width(disp);
	screen_height=al_get_display_height(disp);
	
	creator = new WidgetCreator(gui);

	int i =0;

	while(true) {
		printf("%d\n",i);
		_Bool redraw = false;
		al_wait_for_event(queue, &event);
		inputHandler->processEvent(event);
		switch(event.type) {
		
			case ALLEGRO_EVENT_TIMER:
				if(event.timer.source == timer)
				{
					redraw = true;
				}
				
				break;
			case ALLEGRO_EVENT_DISPLAY_RESIZE:
				al_acknowledge_resize(event.display.source);

				screen_width=al_get_display_width(disp);
				screen_height=al_get_display_height(disp);

				creator->rel_location_pos();
				gui->resizeToDisplay();
				break;
			case ALLEGRO_EVENT_DISPLAY_SWITCH_IN:
				gui->resizeToDisplay();
				break;
			case ALLEGRO_EVENT_DISPLAY_CLOSE:
				return 0;
				break;
		}
		if (redraw && al_event_queue_is_empty(queue)) {
			gui->logic();
			al_clear_to_color(al_map_rgb(240,240,240));
			gui->render();
			al_flip_display();
		}
		i++;
	}

	gui->getTop()->clear();
	delete creator;
	creator = NULL;
	delete gui;
	gui = NULL;
	delete inputHandler;
	delete graphicsHandler;
	inputHandler = NULL;
	graphicsHandler = NULL;

	delete defaultFont;
	defaultFont = NULL;

	return 0;
}
