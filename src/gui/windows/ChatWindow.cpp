#include "ChatWindow.hpp"
#include "MessageBox.hpp"

#include "../../app/FuturePromise.hpp"

#include <fmt/format.h>

ChatWindow::ChatWindow(al::Coord<> pos, std::shared_ptr<AppState> app)
	: Window({640, 480}, pos),
		recvBox({600, 300}, {20, 30}),
		sendBox({570, 20}, {20, 340}),
		sendBtn({30, 20}, {590, 340}, "Send"),
		app(app),
		encMode({200, 20}, {20, 380}, {200, 100}),
		btnGenKP({120, 24}, {240, 380}, "Generate key pair"),
		btnLoadKP({120, 24}, {240, 410}, "Load key pair..."),
		btnSaveKP({120, 24}, {240, 440}, "Save key pair...")
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

	addChild(btnGenKP);
	addChild(btnLoadKP);
	addChild(btnSaveKP);

	btnGenKP.setCallback([this](){genKeyPair();});
	btnLoadKP.setCallback([this](){loadKeyPair();});
	btnSaveKP.setCallback([this](){saveKeyPair();});

	sendBox.setOnReturnCallback([this](){onSend();});
	sendBtn.setCallback([this](){onSend();});

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

void ChatWindow::genKeyPair()
{
	parent->give(std::make_unique<MessageBox>(
		al::Coord<>(200,200),
		"Key pair not generated (placeholder message box)"
	));
}

void ChatWindow::loadKeyPair()
{
	al::FileDialog dialog("keys", "Pick a key...", "*.key");

	Future(dialog.showAsync().share()).Then<void>([this](al::FileDialogResult res){
		if(!res.wasCancelled()) {
			queueMsgBox(fmt::format("Loading from {}", res.paths.at(1)));
		}
	});
}

void ChatWindow::saveKeyPair()
{
	al::FileDialog dialog("keys", "Save key...", "*.key", ALLEGRO_FILECHOOSER_SAVE);

	Future(dialog.showAsync().share()).Then<void>([this](al::FileDialogResult res){
		if(!res.wasCancelled()) {
			queueMsgBox(fmt::format("Saving as {}", res.paths.at(1)));
		}
	});
}

void ChatWindow::queueMsgBox(const std::string& msg)
{
	std::lock_guard<std::mutex> lk(mtx);

	msgBoxQueue.push(msg);
}

void ChatWindow::tick()
{
	std::lock_guard<std::mutex> lk(mtx);

	if(bufChanged) {
		bufChanged = false;
		recvBox.setText(buf);
	}

	while(!msgBoxQueue.empty()) {
		auto m = msgBoxQueue.back();
		msgBoxQueue.pop();
		al::Coord<> pos = al::Coord<>{200,200} + al::Coord<>(rand()%100, rand()%100);
		parent->give(std::make_unique<MessageBox>(pos,m));
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

void ChatWindow::onSend()
{
	std::string msg = sendBox.getText();
	appendToLog("Sending: "+msg+"\n");
	sendMessage(msg);
	sendBox.setText("");
}

void ChatWindow::sendMessage(std::string msg)
{
	app->SendMessage(msg).Then<void>([](uint32_t v){});
}