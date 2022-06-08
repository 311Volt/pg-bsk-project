#include "Chat.hpp"


Chat::Chat(std::unique_ptr<ChatWindow> chatWindow, std::shared_ptr<AppState> appState)
	: chatWindow(std::move(chatWindow)),
	  appState(appState),
	  exitFlag(false)
{

}

Chat::~Chat()
{
	exitFlag = true;
	if(chatThread.joinable()) {
		chatThread.join();
	}
}

void Chat::start()
{
	chatThread = std::thread([this](){
		while(!exitFlag) {
			using namespace std::chrono_literals;

			std::string msg;
			while(appState->PopMessage(msg)) {
				chatWindow->acknowledgeReceivedMessage(msg);
			}
			
			std::this_thread::sleep_for(30ms);
		}
	});
}