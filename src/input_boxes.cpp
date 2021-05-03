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
#include <Agui/FlowLayout.hpp>

#ifdef _WIN32
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#include <iostream>

#include "misc.h"
#include "networking.h"
#include "file_paths.h"
#include "config.h"
#include "input_boxes.h"
#include "drawtrack.h"
#include "race.h"
//#include "connect_server.h"

#define FRAME_RATE 60
#define PORT 8888

//Globals
ALLEGRO_DISPLAY *disp = NULL;
ALLEGRO_TIMER *timer = NULL;
ALLEGRO_EVENT event;
ALLEGRO_EVENT_QUEUE *queue = NULL;
bool done = false;
agui::Gui *gui = NULL;
agui::Allegro5Input* inputHandler = NULL;
agui::Allegro5Graphics* graphicsHandler = NULL;

agui::Font *defaultFont = NULL;
std::string text_in_box;

void wait_for_server(CONFIG* config,ALLEGRO_DISPLAY* disp, ALLEGRO_EVENT* event,ALLEGRO_EVENT_QUEUE *queue, 
		ALLEGRO_FONT* font);

class SimpleActionListener : public agui::ActionListener
{
public:
	virtual void actionPerformed(const agui::ActionEvent &evt)
	{
		agui::TextField* textfield = dynamic_cast<agui::TextField*>(evt.getSource());
		if(textfield){
			textfield->selectAll();
			std::cout << textfield->getSelectedText() << std::endl;
			text_in_box = textfield->getSelectedText();
			
		}
	}
};




class WidgetCreator {
private:
	SimpleActionListener simpleAL;
	agui::FlowLayout flow;
	agui::Gui* mGui;
	agui::TextField textField;

public:
	WidgetCreator(agui::Gui *guiInstance)
{
	mGui = guiInstance;
	mGui->add(&flow);

	flow.add(&textField);
	textField.setSize(1000,30);
	textField.addActionListener(&simpleAL);


}
};

WidgetCreator* creator;


void initializeAgui()
{

	//Set the image loader
	agui::Image::setImageLoader(new agui::Allegro5ImageLoader);

	//Set the font loader
	agui::Font::setFontLoader(new agui::Allegro5FontLoader);

	//Instance the input handler
	inputHandler = new agui::Allegro5Input();

	//Instance the graphics handler
	graphicsHandler = new agui::Allegro5Graphics();

	//Allegro does not automatically premultiply alpha so let Agui do it
	agui::Color::setPremultiplyAlpha(true);

	//Instance the gui
	gui = new agui::Gui();

	//Set the input handler
	gui->setInput(inputHandler);

	//Set the graphics handler
	gui->setGraphics(graphicsHandler);




}

void addWidgets()
{
	creator = new WidgetCreator(gui);
}
void cleanUp()
{
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
}
	void render()
	{
	al_clear_to_color(al_map_rgb(0,0,0));

	//render the widgets
	gui->render();

	al_flip_display();

	}

int input_box(CONFIG* config, ALLEGRO_DISPLAY* disp, PATHS* paths,ALLEGRO_EVENT* event,
		ALLEGRO_EVENT_QUEUE *queue, ALLEGRO_FONT* font){
	/*if (!initializeAllegro())
	{
		return 1;
	}*/
	text_in_box = "";

	initializeAgui();
	printf("Dir=%s\n", al_get_current_directory());
	printf("font=%s\n", config->font_name);
	printf("After defaultFont\n");
	defaultFont = agui::Font::load(config->font_name,16);
	agui::Widget::setGlobalFont(defaultFont);
	addWidgets();
	printf("After addWidgets\n");
	bool needRedraw = true;
	// Start the event queue to handle keyboard input, mouse and our timer
	
	al_register_event_source(queue, (ALLEGRO_EVENT_SOURCE*)al_get_keyboard_event_source());
	//al_register_event_source(queue, (ALLEGRO_EVENT_SOURCE*)al_get_mouse_event_source());
	//al_register_event_source(queue, (ALLEGRO_EVENT_SOURCE*)timer);
	//al_register_event_source(queue, (ALLEGRO_EVENT_SOURCE*)disp);


	while(!done) {
	
		// Block until an event enters the queue
		al_wait_for_event(queue, event);

		//Let Agui process the event
		inputHandler->processEvent(*event);

		//Handle rendering and logic
		if (needRedraw && al_event_queue_is_empty(queue)) {

			gui->logic();
			render();

			needRedraw = false;
		}

		if(text_in_box.size()){
			std::cout << "Text from input_box:"<<text_in_box<<std::endl;
			uint16_t player_number;
			//connect to the server
			int i = 0;
			char server_ip[text_in_box.size()+1];
			while(i<text_in_box.size()){
				server_ip[i] = text_in_box[i];
				i++;
			}
			server_ip[i]='\0';
#ifdef _WIN32
			WSADATA wsa;
			printf("Initialising winsock\n");
			if(WSAStartup(0x0202, &wsa)){
				error_message("Initialising winsock");
				return(1);
			}
			printf("Winsock has been initialised\n");
#endif

			printf("Trying to connect to %s\n", server_ip);
			int client_socket;
			if((client_socket = socket(AF_INET, SOCK_STREAM,0))==-1){
				error_message("to create socket");
				return(1);
			}
			printf("Created socket %d\n and invalid would be %d\n",client_socket, -1);
			struct sockaddr_in server_address;
			server_address.sin_family = AF_INET;
			server_address.sin_port = htons(PORT);
#ifdef _WIN32
			server_address.sin_addr.s_addr = inet_addr(server_ip);
			if(server_address.sin_addr.s_addr==INADDR_ANY)
#else
			if(!inet_aton(server_ip, &server_address.sin_addr))
#endif
			{
				fprintf(stderr, "Invalid IP address\n");
				exit(1);
			}
			//server_address.sin_addr.s_addr = inet_addr(server_ip);	
			if(connect(client_socket, (struct sockaddr *)&server_address, 
						sizeof(server_address))<0){
				error_message("Connecting to the server");
				cleanUp();
				return(1);
			}
			printf("connected\n");
			if(!SetSocketBlockingEnabled(client_socket, 0)){
				error_message("blocking soket");
				return(1);
			}
			//cleanUp();
			//wait_for_server(config, disp, event,queue,font);
			connect_server_race(client_socket, player_number,
					config, disp, paths, 
					event, queue, font);
			//shutdown(client_socket,2);
			close(client_socket);//,2);
			return(0);
		}
		
		switch(event->type) {
	case (ALLEGRO_EVENT_KEY_DOWN):
		if(event->keyboard.keycode == ALLEGRO_KEY_ESCAPE){
			return 0;
		}
		break;

		
	case ALLEGRO_EVENT_TIMER:

		needRedraw = true;
		
		break;
	case ALLEGRO_EVENT_DISPLAY_RESIZE:

		al_acknowledge_resize(event->display.source);

		//Resize Agui
		gui->resizeToDisplay();
		
		break;
	case ALLEGRO_EVENT_DISPLAY_SWITCH_IN:
		//Resize Agui
		gui->resizeToDisplay();
		break;
	case ALLEGRO_EVENT_DISPLAY_CLOSE:
		exit(1);
		break;
		}
	}

	cleanUp();

return 0;
}
// vim: cc=100
