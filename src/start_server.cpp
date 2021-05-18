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
#include <allegro5/allegro5.h>
#include <allegro5/allegro_font.h>

#include <Agui/Agui.hpp>
#include <Agui/Backends/Allegro5/Allegro5.hpp>

#include <Agui/Widgets/TextField/TextField.hpp>
#include <Agui/Widgets/Button/Button.hpp>
#include <Agui/FlowLayout.hpp>

#ifdef _WIN32
#include <winsock2.h>
#else
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#include <iostream>

#include "global.h"
#include "misc.h"
#include "networking.h"
#include "file_paths.h"
#include "config.h"
#include "drawtrack.h"
#include "race.h"
#include "start_server.h"


//globals
agui::Gui *start_gui = NULL;
agui::Allegro5Input* start_inputHandler = NULL;
agui::Allegro5Graphics* start_graphicsHandler = NULL;

agui::Font *start_defaultFont = NULL;
std::string start_port_text;
	
agui::TextField* pstart_port_textField;


class start_SimpleActionListener : public agui::ActionListener
{
public:
	TRACK_DATA* track;
	char* filename;
	virtual void actionPerformed(const agui::ActionEvent &evt){
		agui::Button* button = dynamic_cast<agui::Button*>(evt.getSource());
		if(button){
			pstart_port_textField->selectAll();
			start_port_text = pstart_port_textField->getSelectedText();
			int port = std::stoi(start_port_text);

			int server_socket , new_socket;
			struct sockaddr_in server , client;
			int max_sd;
			int c;
#ifdef __WIN32
			WSADATA wsa;
			if (WSAStartup(MAKEWORD(2,2),&wsa) != 0)
			{
				printf("Failed. Error Code : %d",WSAGetLastError());
				return;
			}
			
#else
			//Otherwise a SIGPIPE signal would crash the program
			struct sigaction sa;
			sa.sa_handler = SIG_IGN;
			sigaction(SIGPIPE, &sa, NULL);
#endif
			//Create a socket
			if((server_socket = socket(AF_INET , SOCK_STREAM , 0 )) == -1)
			{
				error_message("creating socket");
				return;
			}
			max_sd = server_socket;
			
			//Prepare the sockaddr_in structure
			server.sin_family = AF_INET;
			server.sin_addr.s_addr = INADDR_ANY;
			server.sin_port = htons( port );
			
			//Bind
			if( bind(server_socket ,(struct sockaddr *)&server , sizeof(server)) == -1)
			{
				error_message("To bind");
			}
			

			//Listen to incoming connections
			listen(server_socket , 3);
			
			node_t sockets_head;
			sockets_head.value = server_socket;
			sockets_head.next = NULL;
			
			server_race(server_socket, max_sd, &sockets_head,
					track,filename);
			close(server_socket);
#ifdef __WIN32
			WSACleanup();
#endif
		}
	}
};




class start_WidgetCreator {
private:
	start_SimpleActionListener simpleAL;
	agui::FlowLayout flow;
	agui::Gui* mGui;

public:
	agui::TextField port_textField;
	agui::Button connect_button;
	start_WidgetCreator(agui::Gui *guiInstance){
		mGui = guiInstance;


		mGui->add(&port_textField);
		port_textField.addActionListener(&simpleAL);
		port_textField.setText("8888");
		port_textField.setMaxLength(4);
		port_textField.setLocation(screen_width/2,screen_height/2);

		mGui->add(&connect_button);
		connect_button.setSize(80,40);
		connect_button.setText("Connect");
		connect_button.addActionListener(&simpleAL);

		pstart_port_textField = &port_textField;



	}
	void Give_track(TRACK_DATA* track, char* filename){
		simpleAL.track = track;
		simpleAL.filename = filename;
	}
};
start_WidgetCreator* start_creator;

void start_cleanUp();





void start_cleanUp(){
	start_gui->getTop()->clear();
	delete start_creator;
	start_creator = NULL;
	delete start_gui;
	start_gui = NULL;
	delete start_inputHandler;
	delete start_graphicsHandler;
	start_inputHandler = NULL;
	start_graphicsHandler = NULL;

	delete start_defaultFont;
	start_defaultFont = NULL;
}



void start_initializeAgui()
{

	//Set the image loader
	agui::Image::setImageLoader(new agui::Allegro5ImageLoader);

	//Set the font loader
	agui::Font::setFontLoader(new agui::Allegro5FontLoader);

	//Instance the input handler
	start_inputHandler = new agui::Allegro5Input();

	//Instance the graphics handler
	start_graphicsHandler = new agui::Allegro5Graphics();

	//Allegro does not automatically premultiply alpha so let Agui do it
	agui::Color::setPremultiplyAlpha(true);

	//Instance the gui
	start_gui = new agui::Gui();

	//Set the input handler
	start_gui->setInput(start_inputHandler);

	//Set the graphics handler
	start_gui->setGraphics(start_graphicsHandler);




}

void start_addWidgets()
{
	start_creator = new start_WidgetCreator(start_gui);
}

void start_server(TRACK_DATA *track,char* filename){
	bool done = false;

	start_initializeAgui();
	start_defaultFont = agui::Font::load(config.font_name,16);
	agui::Widget::setGlobalFont(start_defaultFont);
	start_addWidgets();
	start_creator->Give_track(track,filename);
	bool needRedraw = true;
	// Start the event queue to handle keyboard input, mouse and our timer
	
	al_register_event_source(queue, (ALLEGRO_EVENT_SOURCE*)al_get_keyboard_event_source());


	while(!done) {
	
		// Block until an event enters the queue
		al_wait_for_event(queue, &event);

		//Let Agui process the event
		start_inputHandler->processEvent(event);

		//Handle rendering and logic
		if (needRedraw && al_event_queue_is_empty(queue)) {

			start_gui->logic();

			al_clear_to_color(al_map_rgb(0,0,0));
			int screen_width = al_get_display_width(disp);
			int screen_height = al_get_display_height(disp);

#define input_height 25
#define gap 10
			ALLEGRO_FONT* font = al_load_font(config.font_name,16,0);

			const char *start_port_text = "port: ";
			al_draw_text(font, config.button_text_color, screen_width/4-
					al_get_text_width(font,start_port_text)/2,screen_height/2,
					ALLEGRO_ALIGN_CENTER,start_port_text);
			pstart_port_textField->setLocation(
					screen_width/2-screen_width/4,
					screen_height/2);
			pstart_port_textField->setSize(screen_width/2,input_height);
			start_gui->render();
			
			
			start_creator->connect_button.setLocation(screen_width/4*3, screen_height/4*3);
			
			al_flip_display();

			//render();

			needRedraw = false;
		}

		
		switch(event.type) {
	case (ALLEGRO_EVENT_KEY_DOWN):
		if(event.keyboard.keycode == ALLEGRO_KEY_ESCAPE){
			return;
		}
		break;

		
	case ALLEGRO_EVENT_TIMER:

		needRedraw = true;
		
		break;
	case ALLEGRO_EVENT_DISPLAY_RESIZE:

		al_acknowledge_resize(event.display.source);

		//Resize Agui
		start_gui->resizeToDisplay();
		
		break;
	case ALLEGRO_EVENT_DISPLAY_SWITCH_IN:
		//Resize Agui
		start_gui->resizeToDisplay();
		break;
	case ALLEGRO_EVENT_DISPLAY_CLOSE:
		exit(1);
		break;
		}
	}

	start_cleanUp();

return;
}
// vim: cc=100
