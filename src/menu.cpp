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
#include <Agui/Widgets/TextField/TextField.hpp>

#include <stdlib.h>
#include <iostream>

//networking
#ifdef _WIN32
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

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

typedef enum{MAIN=0,MULTIPLAYER=1,JOIN=2} GUI_I;

//Globals
ALLEGRO_DISPLAY *disp;
ALLEGRO_TIMER *timer;
ALLEGRO_EVENT event;
ALLEGRO_EVENT_QUEUE *queue;
int screen_width;
int screen_height;
PATHS paths;
CONFIG config;
GUI_I gui_i;

class connect_server_listener : public agui::ActionListener
{
public:
	virtual void actionPerformed(const agui::ActionEvent &evt);
};


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
				gui_i = MULTIPLAYER;
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
class join_server_listener : 
	public agui::ActionListener{
		public:
			virtual void actionPerformed(const agui::ActionEvent &evt){
				gui_i = JOIN;
			}
};
class start_server_listener : 
	public agui::ActionListener{
		public:
			virtual void actionPerformed(const agui::ActionEvent &evt){
				 track_menu(start_server);
			}
};
class multiplayer_back_listener : 
	public agui::ActionListener{
		public:
			virtual void actionPerformed(const agui::ActionEvent &evt){
				gui_i = MAIN;
			}
};
class join_server_back_listener : 
	public agui::ActionListener{
		public:
			virtual void actionPerformed(const agui::ActionEvent &evt){
				gui_i = MULTIPLAYER;
			}
};



class WidgetCreator {
private:
	//main menu
	agui::Button time_trial;
	agui::Button multiplayer;
	agui::Button records;
	agui::Button quit;

	time_trial_listener time_trialAL;
	multiplayer_listener multiplayerAL;
	records_listener recordsAL;
	quit_listener quitAL;


	//multiplayer menu
	agui::Button join_server;
	agui::Button start_server;
	agui::Button multiplayer_back;

	join_server_listener join_serverAL;
	start_server_listener start_serverAL;
	multiplayer_back_listener multiplayer_backAL;

	//join server menu
	agui::TextField join_ip;
	agui::TextField join_port;
	agui::Button connect_server;
	agui::Button join_server_back;

	connect_server_listener connect_serverAL;
	join_server_back_listener join_server_backAL;
public:
	void get_join_text(std::string* pjoin_ip,std::string* pjoin_port){
		join_ip.selectAll();
		*pjoin_ip = join_ip.getSelectedText();
		join_port.selectAll();
		*pjoin_port = join_port.getSelectedText();
	}

	void rel_location_pos(){
		//main menu
		time_trial.setLocation(	screen_width*0.3,	screen_height*0.42);
		multiplayer.setLocation(screen_width*0.3,	screen_height*0.57);
		records.setLocation(	screen_width*0.3,	screen_height*0.72);
		quit.setLocation(	screen_width*0.51,	screen_height*0.72);
		
		time_trial.setSize(	screen_width*0.4,	screen_height*0.08);
		multiplayer.setSize(	screen_width*0.4,	screen_height*0.08);
		records.setSize(	screen_width*0.19,	screen_height*0.08);
		quit.setSize(		screen_width*0.19,	screen_height*0.08);

		//multiplayer menu
		join_server.setLocation(screen_width*0.2,	screen_height*0.2);
		start_server.setLocation(screen_width*0.2,	screen_height*0.55);
		multiplayer_back.setLocation(screen_width*0.05,	screen_height*0.9);

		join_server.setSize(	screen_width*0.6,	screen_height*0.25);
		start_server.setSize(	screen_width*0.6,	screen_height*0.25);
		multiplayer_back.setSize(screen_width*0.1,	screen_height*0.05);

		//Join server menu
		join_ip.setLocation(	screen_width*0.25,	screen_height*0.5-30);
		join_port.setLocation(	screen_width*0.25,	screen_height*0.5+30);
		connect_server.setLocation(screen_width*0.75, 	screen_height*0.75);
		join_server_back.setLocation(screen_width*0.25-80,	screen_height*0.25);

		join_ip.setSize(	screen_width*0.5,	25);
		join_port.setSize(	screen_width*0.5,	25);
		connect_server.setSize(80,40);
		join_server_back.setSize(80,40);
	}

	WidgetCreator(agui::Gui *gui){
		//main menu
		gui[0].add(&time_trial);
		gui[0].add(&multiplayer);
		gui[0].add(&records);
		gui[0].add(&quit);


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


		//multiplayer menu
		gui[1].add(&join_server);
		gui[1].add(&start_server);
		gui[1].add(&multiplayer_back);
		
		join_server.setText("join server");
		start_server.setText("start server");
		multiplayer_back.setText("Back");

		join_server.addActionListener(&join_serverAL);
		start_server.addActionListener(&start_serverAL);
		multiplayer_back.addActionListener(&multiplayer_backAL);


		//join menu
		gui[2].add(&join_ip);
		gui[2].add(&join_port);
		gui[2].add(&connect_server);
		gui[2].add(&join_server_back);

		join_port.setText("8888");
		connect_server.setText("Connect");
		join_server_back.setText("Back");

		connect_server.addActionListener(&connect_serverAL);
		join_server_back.addActionListener(&join_server_backAL);

		rel_location_pos();
	}
};

WidgetCreator* creator;

void connect_server_listener::actionPerformed(const agui::ActionEvent &evt){
	std::string join_ip_text;
	std::string join_port_text;
	creator->get_join_text(&join_ip_text, &join_port_text);
	int port = std::stoi(join_port_text);
	uint16_t player_number;
	//connect to the server
	int i = 0;
	char server_ip[join_ip_text.size()+1];
	while(i<join_ip_text.size()){
		server_ip[i] = join_ip_text[i];
		i++;
	}
	server_ip[i]='\0';
#ifdef _WIN32
	WSADATA wsa;
	printf("Initialising winsock\n");
	if(WSAStartup(0x0202, &wsa)){
		error_message("Initialising winsock");
		return;
	}
	printf("Winsock has been initialised\n");
#endif

	printf("Trying to connect to %s\n", server_ip);
	int client_socket;
	if((client_socket = socket(AF_INET, SOCK_STREAM,0))==-1){
		error_message("to create socket");
		return;
	}
	printf("Created socket %d\n",client_socket);
	struct sockaddr_in server_address;
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(port);
	server_address.sin_addr.s_addr = inet_addr(server_ip);
	if(server_address.sin_addr.s_addr==INADDR_ANY)
	{
		fprintf(stderr, "Invalid IP address\n");
		exit(1);
	}
	if(connect(client_socket, (struct sockaddr *)&server_address, 
				sizeof(server_address))<0){
		error_message("Connecting to the server");
		return;
	}
	printf("connected\n");
	if(!SetSocketBlocking(client_socket, 0)){
		error_message("blocking socket");
		return;
	}
	ALLEGRO_FONT* font;
	connect_server_race(client_socket, player_number);
	close(client_socket);
	return;
}



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
	agui::Gui *gui = new agui::Gui[3]();
	for(int i = 0; i<3; i++){
		gui[i].setInput(inputHandler);
		gui[i].setGraphics(graphicsHandler);
	}
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
				gui[gui_i].resizeToDisplay();
				break;
			case ALLEGRO_EVENT_DISPLAY_SWITCH_IN:
				gui[gui_i].resizeToDisplay();
				break;
			case ALLEGRO_EVENT_DISPLAY_CLOSE:
				return 0;
				break;
		}
		if (redraw && al_event_queue_is_empty(queue)) {
			gui[gui_i].logic();
			al_clear_to_color(al_map_rgb(0,0,0));
			gui[gui_i].render();
			al_flip_display();
		}
		i++;
	}

	for(int i =0; i<3; i++){
		gui[i].getTop()->clear();
		delete &gui[i];
	}
	delete creator;
	creator = NULL;
	gui = NULL;
	delete inputHandler;
	delete graphicsHandler;
	inputHandler = NULL;
	graphicsHandler = NULL;

	delete defaultFont;
	defaultFont = NULL;

	return 0;
}
