
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
/*
	int myport = (rand()%20000)+10000;

	disp.setTitle(fmt::format("BSK project @ 127.0.0.1:{}",myport));
	auto app = std::make_shared<AppState>("127.0.0.1", myport);

	gui::Desktop desk;
	desk.setBgColor(al::RGB(0,127,127));

	gui::Text addrp({300, 40}, {30, 30});
	addrp.setTextColor(al::White);
	addrp.setText(fmt::format("Server running on {}:{}", app->ipAddress, app->port));
	addrp.setZIndex(-311);
	desk.addChild(addrp);

	ChatWindow chatWin({250, 250}, app);
	
	desk.addChild(chatWin);

	desk.give(std::make_unique<AddressWindow>(al::Coord<>{40, 40}, [&](std::string addr, int port){
		
		try {
			app->ConnectAndHandshake(addr, port);
			chatWin.appendToLog(fmt::format("Conversation initiated with {}:{}\n", addr, port));
		} catch(KexError& e) {
			auto mb = std::make_unique<MessageBox>(al::Coord<>{400, 400}, fmt::format("Handshake failed: {}", e.what()));
			desk.give(std::move(mb));
		}

	}));

	std::atomic_bool exitFlag = false;

	std::thread thread([&chatWin, &exitFlag](std::shared_ptr<AppState> app) {
			while(!exitFlag) {
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
				std::string msg;
				while(app->PopMessage(msg)) {
					chatWin.acknowledgeReceivedMessage(msg);
				}
			}
		}, app
	);


	desk.mainLoop();
	exitFlag = true;
	thread.join();

	return 0;
	*/
}

