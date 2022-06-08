#ifndef SRC_GUI_CHAT
#define SRC_GUI_CHAT

#include "windows/ChatWindow.hpp"
#include <thread>

class Chat {
	std::unique_ptr<ChatWindow> chatWindow;
	std::shared_ptr<AppState> appState;
	std::thread chatThread;
	bool exitFlag;
public:
	Chat(std::unique_ptr<ChatWindow> chatWindow, std::shared_ptr<AppState> appState);
	~Chat();

	void start();
};

#endif /* SRC_GUI_CHAT */
