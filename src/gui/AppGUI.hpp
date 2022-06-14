#ifndef SRC_GUI_APPGUI
#define SRC_GUI_APPGUI

#include "windows/AddressWindow.hpp"
#include "windows/ChatWindow.hpp"
#include "windows/MessageBox.hpp"
#include "windows/KeyManager.hpp"

#include "../app/AppState.hpp"
#include "Chat.hpp"

#include <fmt/format.h>

class AppGUI: public gui::Desktop {
public:
	AppGUI(std::shared_ptr<AppState> appState);
	~AppGUI();

	void run();
	AppState* app();

	void msgBoxStr(const std::string& msg);

	template<typename... Args>
	void msgBox(const std::string_view msg, Args... args)
	{
		msgBoxStr(fmt::format(msg, args...));
	}
private:
	virtual void tick() override;
	bool chatWindowNeeded;

	void initiateChat();
	void tryConnect(const std::string& addr, int port);

	uint32_t chatWinId;

	std::mutex mtx;

	std::queue<std::string> msgBoxQueue;

	std::shared_ptr<AppState> appState;
	gui::Text serverAddrInfo;
	gui::Text credits;
	KeyManager keyMgr;
	std::unique_ptr<Chat> chat;
};

#endif /* SRC_GUI_APPGUI */
