#ifndef SRC_GUI_CONTROLS_CHATWINDOW
#define SRC_GUI_CONTROLS_CHATWINDOW

#include <lpg/gui/gui.hpp>
#include "../../app/AppState.hpp"

class ChatWindow: public gui::Window {

	gui::Text recvBox;
	gui::TextBox sendBox;

	gui::DropdownList encMode;

	gui::Button btnGenKP, btnLoadKP, btnSaveKP;

	gui::Button sendBtn;
	std::string buf;
	bool bufChanged;

	std::shared_ptr<AppState> app;
	std::mutex mtx;

	std::queue<std::string> msgBoxQueue;
public:

	void genKeyPair();
	void loadKeyPair();
	void saveKeyPair();

	ChatWindow(al::Coord<> pos, std::shared_ptr<AppState> app);
	virtual void tick() override;
	void appendToLog(const std::string_view text);
	void acknowledgeReceivedMessage(const std::string_view msg);
	void onSend();
	void sendMessage(std::string msg);

	void queueMsgBox(const std::string& msg);
};


#endif /* SRC_GUI_CONTROLS_CHATWINDOW */
