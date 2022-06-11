
#include <app/AppState.hpp>
#include <gui/AppGUI.hpp>

#include <chrono>
#include <thread>
#include <iostream>

#include <lpg/gui/gui.hpp>
#include <axxegro/axxegro.hpp>

#include <functional>


int main(int argc, char** argv) 
{
	al::FullInit();
	srand(std::hash<double>()(al::GetTime()));
	std::set_terminate(al::Terminate);
	al::Display disp(1024, 768);
	
	gui::Window::RM.registerDefaultLoaders();
	gui::Window::RM.loadFromConfig(al::Config("gui/default.ini"));

	int myPort = (rand()%20000) + 10000;
	auto app = std::make_shared<AppState>("127.0.0.1", myPort);
	AppGUI appGUI(app);

	appGUI.run();
}

