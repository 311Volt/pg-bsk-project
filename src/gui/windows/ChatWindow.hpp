#ifndef SRC_GUI_CONTROLS_CHATWINDOW
#define SRC_GUI_CONTROLS_CHATWINDOW

#include <lpg/gui/gui.hpp>
#include "../../app/AppState.hpp"

#include "PasswordField.hpp"

#include <fmt/format.h>

class ChatWindow: public gui::Window {

	gui::Text recvBox;
	gui::TextBox sendBox;

	gui::DropdownList encMode;

	gui::Button btnGenKP, btnLoadKP, btnSaveKP;
	gui::Text lpCaption, spCaption;
	PasswordField pfLoadPass, pfSavePass;
	gui::Text keyText;

	gui::Button sendBtn;
	bool bufChanged;

	std::shared_ptr<AppState> app;
	std::mutex mtx;

	std::queue<std::string> msgBoxQueue;
	std::deque<std::string> log;
public:

	void genKeyPair();

	void tryLoadKeyPair(al::FileDialogResult& r);
	void trySaveKeyPair(al::FileDialogResult& r);

	void loadKeyPairDialog();
	void saveKeyPairDialog();

	ChatWindow(al::Coord<> pos, std::shared_ptr<AppState> app);
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

	void updateKeyText();

	void queueMsgBox(const std::string& msg);
};


#endif /* SRC_GUI_CONTROLS_CHATWINDOW */
