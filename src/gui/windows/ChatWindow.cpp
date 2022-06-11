#include "ChatWindow.hpp"

#include <fmt/format.h>

ChatWindow::ChatWindow(al::Coord<> pos, std::shared_ptr<AppState> app)
	: Window({640, 480}, pos),
		recvBox({600, 300}, {20, 30}),
		sendBox({570, 20}, {20, 340}),
		sendBtn({30, 20}, {590, 340}, "Send"),
		app(app),
		encMode({200, 20}, {20, 380}, {200, 100})
{
	bufChanged = true;
	setTitle(fmt::format("Chat: {}:{}", app->ipAddress, app->port));
	give(std::make_unique<gui::TitleBar>());

	recvBox.setBgColor(al::White);
	recvBox.setEdgeType(gui::Window::EDGE_BEVELED_INWARD);
	recvBox.setPadding({8,8,8,8});

	addChild(recvBox);
	addChild(sendBox);
	addChild(sendBtn);

	sendBtn.setCallback([this](){
		std::string msg = sendBox.getText();
		appendToLog("Sending: "+msg+"\n");
		sendMessage(msg);
		sendBox.setText("");
	});

	encMode.setElements({
		{CHACHA20_POLY1305, "CHACHA20_POLY1305"},
		{CHACHA20, "CHACHA20"}
	});
	encMode.setOnChangeCallback([this](uint32_t mode){
		static std::unordered_map<uint32_t, std::string> encModeNames = {
			{CHACHA20_POLY1305, "CHACHA20_POLY1305"},
			{CHACHA20, "CHACHA20"}
		};

		if(encModeNames.count(mode)) {
			this->app->currentEncryptionMode = (ENCRYPTION_MODE)mode;
			appendToLog(fmt::format("Encryption mode changed to {}\n", encModeNames[mode]));
		}
	});

	addChild(encMode);
}

void ChatWindow::tick()
{
	std::lock_guard<std::mutex> lk(mtx);

	if(bufChanged) {
		bufChanged = false;
		recvBox.setText(buf);
	}
	Window::tick();
}

void ChatWindow::appendToLog(const std::string_view text)
{
	std::lock_guard<std::mutex> lk(mtx);

	buf += text;
	bufChanged = true;
}

void ChatWindow::acknowledgeReceivedMessage(const std::string_view msg)
{
	appendToLog(fmt::format("Received: {}\n", msg));
}

void ChatWindow::sendMessage(std::string msg)
{
	app->SendMessage(msg).Then<void>([](uint32_t v){});
}