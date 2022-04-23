
#include <app/AppState.hpp>

#include <chrono>
#include <thread>
#include <iostream>

int main(int argc, char** argv) {
	srand(time(NULL));
	int myport = (rand()%20000)+10000;
	printf(" my port: %i\n", myport);
	AppState* app = new AppState("127.0.0.1", myport);
	if(argc == 3) {
		std::string ip = argv[1];
		int port = atoi(argv[2]);
		int ret = app->ConnectAndHandshake(ip, port);
		if(ret == SUCCESS) {
			printf(" connected to %s:%i\n", ip.c_str(), port);
		} else {
			printf(" error connecting to %s:%i -> %i\n", ip.c_str(), port, ret);
		}
	}
	std::thread thread([](AppState* app) {
			while(true) {
				std::this_thread::sleep_for(std::chrono::seconds(1));
				std::string msg;
				if(app->PopMessage(msg)) {
					printf(" Received [%lu]: %s\n", msg.size(), msg.c_str());
				}
			}
		}, app);
	while(true) {
		std::string msg;
		std::getline(std::cin, msg);
		if(msg.size() == 0) {
			if(app->currentEncryptionMode == CHACHA20)
				app->currentEncryptionMode = CHACHA20_POLY1305;
			else
				app->currentEncryptionMode = CHACHA20;
		} else {
			app->SendMessage(msg);
		}
	}
	return 0;
}

