#ifndef SRC_GUI_WINDOWS_CHATWINDOW
#define SRC_GUI_WINDOWS_CHATWINDOW

#include <lpg/gui/gui.hpp>
#include "../../app/AppState.hpp"

#include "PasswordField.hpp"
#include "FileTransferWin.hpp"

#include <fmt/format.h>

class AppGUI;
class FileTransferWin;

class ChatWindow: public gui::Window {

	AppGUI* gui;

	gui::Text recvBox;
	gui::TextBox sendBox;

	gui::DropdownList encMode;
	std::unique_ptr<FileTransferWin> ftWin;

	gui::Button sendBtn;
	gui::Button sendFileBtn;
	bool bufChanged;

	std::mutex mtx;

	std::deque<std::string> log;
public:

	ChatWindow(al::Coord<> pos, AppGUI* gui);
	~ChatWindow();
	virtual void tick() override;

	void sendFileDialog();
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
