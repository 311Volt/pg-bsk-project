#ifndef SRC_GUI_CONTROLS_CHATWINDOW
#define SRC_GUI_CONTROLS_CHATWINDOW

#include <lpg/gui/gui.hpp>
#include "../../app/AppState.hpp"

class ChatWindow: public gui::Window {

	gui::Text recvBox;
	gui::TextBox sendBox;

	gui::Button sendBtn;
	std::string buf;
	bool bufChanged;

	std::shared_ptr<AppState> app;
	std::mutex mtx;
public:
	ChatWindow(al::Coord<> pos, std::shared_ptr<AppState> app);
	virtual void tick() override;
	void appendToLog(const std::string_view text);
	void acknowledgeReceivedMessage(const std::string_view msg);
	void sendMessage(std::string msg);
	
};


#endif /* SRC_GUI_CONTROLS_CHATWINDOW */
