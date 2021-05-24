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
#include "join_server.h"
#include "drawtrack.h"
#include "race.h"


//globals
agui::Gui *join_gui = NULL;
agui::Allegro5Input* join_inputHandler = NULL;
agui::Allegro5Graphics* join_graphicsHandler = NULL;

agui::Font *join_defaultFont = NULL;
std::string join_ip_text;
std::string join_port_text;

class join_SimpleActionListener : public agui::ActionListener
{
public:
	virtual void actionPerformed(const agui::ActionEvent &evt){
		agui::Button* button = dynamic_cast<agui::Button*>(evt.getSource());
		if(button){
			pjoin_ip_textField->selectAll();
			join_ip_text = pjoin_ip_textField->getSelectedText();
			pjoin_port_textField->selectAll();
			join_port_text = pjoin_port_textField->getSelectedText();
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
	}
};




class join_WidgetCreator {
private:
	join_SimpleActionListener simpleAL;
	agui::FlowLayout flow;
	agui::Gui* mGui;

public:
	agui::TextField join_ip_textField;
	agui::TextField join_port_textField;
	agui::Button connect_button;
	join_WidgetCreator(agui::Gui *guiInstance){
		mGui = guiInstance;

		mGui->add(&join_ip_textField);
		join_ip_textField.addActionListener(&simpleAL);

		mGui->add(&join_port_textField);
		join_port_textField.addActionListener(&simpleAL);
		join_port_textField.setText("8888");
		join_port_textField.setMaxLength(4);

		mGui->add(&connect_button);
		connect_button.setSize(80,40);
		connect_button.setText("Connect");
		connect_button.addActionListener(&simpleAL);

		pjoin_ip_textField = &join_ip_textField;
		pjoin_port_textField = &join_port_textField;



	}
};
join_WidgetCreator* join_creator;

void join_cleanUp();





void join_cleanUp(){
	join_gui->getTop()->clear();
	delete join_creator;
	join_creator = NULL;
	delete join_gui;
	join_gui = NULL;
	delete join_inputHandler;
	delete join_graphicsHandler;
	join_inputHandler = NULL;
	join_graphicsHandler = NULL;

	delete join_defaultFont;
	join_defaultFont = NULL;
}



void join_initializeAgui()
{

	//Set the image loader
	agui::Image::setImageLoader(new agui::Allegro5ImageLoader);

	//Set the font loader
	agui::Font::setFontLoader(new agui::Allegro5FontLoader);

	//Instance the input handler
	join_inputHandler = new agui::Allegro5Input();

	//Instance the graphics handler
	join_graphicsHandler = new agui::Allegro5Graphics();

	//Allegro does not automatically premultiply alpha so let Agui do it
	agui::Color::setPremultiplyAlpha(true);

	//Instance the gui
	join_gui = new agui::Gui();

	//Set the input handler
	join_gui->setInput(join_inputHandler);

	//Set the graphics handler
	join_gui->setGraphics(join_graphicsHandler);




}

void join_addWidgets()
{
	join_creator = new join_WidgetCreator(join_gui);
}

int join_server(){
	bool done = false;
	join_ip_text = "";

	join_initializeAgui();
	join_defaultFont = agui::Font::load(config.font_name,16);
	agui::Widget::setGlobalFont(join_defaultFont);
	join_addWidgets();
	bool needRedraw = true;
	// Start the event queue to handle keyboard input, mouse and our timer
	
	al_register_event_source(queue, (ALLEGRO_EVENT_SOURCE*)al_get_keyboard_event_source());


	while(!done) {
	
		// Block until an event enters the queue
		al_wait_for_event(queue, &event);

		//Let Agui process the event
		join_inputHandler->processEvent(event);

		//Handle rendering and logic
		if (needRedraw && al_event_queue_is_empty(queue)) {

			join_gui->logic();

			al_clear_to_color(al_map_rgb(0,0,0));
			int screen_width = al_get_display_width(disp);
			int screen_height = al_get_display_height(disp);

#define input_height 25
#define gap 10
			ALLEGRO_FONT* font = al_load_font(config.font_name,16,0);
			const char *join_ip_text = "ip: ";
			int height = screen_height/2-gap/2-input_height;
			al_draw_text(font, config.button_text_color, screen_width/4-
					al_get_text_width(font,join_ip_text)/2,height,
					ALLEGRO_ALIGN_CENTER,join_ip_text);
			pjoin_ip_textField->setLocation(screen_width/4, height);
			pjoin_ip_textField->setSize(screen_width/2,input_height);

			const char *join_port_text = "port: ";
			height = screen_height/2+gap/2+input_height;
			al_draw_text(font, config.button_text_color, screen_width/4-
					al_get_text_width(font,join_port_text)/2,height,
					ALLEGRO_ALIGN_CENTER,join_port_text);
			pjoin_port_textField->setLocation(screen_width/4, height);
			pjoin_port_textField->setSize(screen_width/2,input_height);
			join_gui->render();
			
			
			join_creator->connect_button.setLocation(screen_width/4*3, screen_height/4*3);
			
			al_flip_display();

			needRedraw = false;
		}

		switch(event.type) {
	case (ALLEGRO_EVENT_KEY_DOWN):
		if(event.keyboard.keycode == ALLEGRO_KEY_ESCAPE){
			return 0;
		}
		break;

		
	case ALLEGRO_EVENT_TIMER:

		needRedraw = true;
		
		break;
	case ALLEGRO_EVENT_DISPLAY_RESIZE:

		al_acknowledge_resize(event.display.source);

		//Resize Agui
		join_gui->resizeToDisplay();
		
		break;
	case ALLEGRO_EVENT_DISPLAY_SWITCH_IN:
		//Resize Agui
		join_gui->resizeToDisplay();
		break;
	case ALLEGRO_EVENT_DISPLAY_CLOSE:
		exit(1);
		break;
		}
	}

	join_cleanUp();

return 0;
}
// vim: cc=100
