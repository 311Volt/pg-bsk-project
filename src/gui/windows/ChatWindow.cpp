#include "ChatWindow.hpp"

ChatWindow::ChatWindow(al::Coord<> pos, std::shared_ptr<AppState> app)
	: Window({640, 480}, pos),
		recvBox({600, 300}, {20, 30}),
		sendBox({570, 20}, {20, 340}),
		sendBtn({30, 20}, {590, 340}, "Send"),
		app(app)
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

	sendBtn.setCallback([&](){
		std::string msg = sendBox.getText();
		appendToLog("Sending: "+msg+"\n");
		sendMessage(msg);
		sendBox.setText("");
	});
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
	appendToLog("Received: " + std::string(msg) + "\n");
}

void ChatWindow::sendMessage(std::string msg)
{
	if(msg.size() == 0) {
		if(app->currentEncryptionMode == CHACHA20)
			app->currentEncryptionMode = CHACHA20_POLY1305;
		else
			app->currentEncryptionMode = CHACHA20;
	} else {
		app->SendMessage(msg).Then<void>([](uint32_t v){
					printf(" received: %i\n", v);
			});
	}
}