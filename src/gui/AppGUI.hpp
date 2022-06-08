#ifndef SRC_GUI_APPGUI
#define SRC_GUI_APPGUI

#include "windows/AddressWindow.hpp"
#include "windows/ChatWindow.hpp"
#include "windows/MessageBox.hpp"

#include "../app/AppState.hpp"
#include "Chat.hpp"

class AppGUI {
public:
	AppGUI(std::shared_ptr<AppState> appState);

	void run();
private:
	void initiateChat();
	void tryConnect(const std::string& addr, int port);

	uint32_t chatWinId;

	std::shared_ptr<AppState> appState;
	gui::Desktop desk;
	gui::Text serverAddrInfo;
	gui::Text credits;
	std::unique_ptr<Chat> chat;
};

#endif /* SRC_GUI_APPGUI */
