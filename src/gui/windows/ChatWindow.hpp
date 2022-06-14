#ifndef SRC_GUI_WINDOWS_CHATWINDOW
#define SRC_GUI_WINDOWS_CHATWINDOW

#include <lpg/gui/gui.hpp>
#include "../../app/AppState.hpp"

#include "PasswordField.hpp"

#include <fmt/format.h>

class AppGUI;

class ChatWindow: public gui::Window {

	AppGUI* gui;

	gui::Text recvBox;
	gui::TextBox sendBox;

	gui::DropdownList encMode;

	gui::Button sendBtn;
	bool bufChanged;

	std::mutex mtx;

	std::deque<std::string> log;
public:

	ChatWindow(al::Coord<> pos, AppGUI* gui);
	virtual void tick() override;

	void updateLog();
	void appendToLog(const std::string_view text);

	template<typename... Args>
	void println(const std::string_view text, Args... args)
	{
		appendToLog(fmt::format(text, args...));
	}


	void acknowledgeReceivedMessage(const std::string_view msg);
	void onSend();
	void sendMessage(std::string msg);
};


#endif /* SRC_GUI_WINDOWS_CHATWINDOW */
