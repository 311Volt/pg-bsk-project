#include "ChatWindow.hpp"
#include "MessageBox.hpp"

#include "../../app/FuturePromise.hpp"

#include <fmt/format.h>
#include <filesystem>

#include "../AppGUI.hpp"


ChatWindow::ChatWindow(al::Coord<> pos, AppGUI* gui)
	: Window({640, 480}, pos),
		recvBox({600, 300}, {20, 30}),
		sendBox({570, 20}, {20, 340}),
		sendBtn({30, 20}, {590, 340}, "Send"),
		gui(gui),
		encMode({200, 20}, {20, 380}, {200, 100})
{
	setTitle(fmt::format("Chat: {}:{}", gui->app()->ipAddress, gui->app()->port));
	give(std::make_unique<gui::TitleBar>());

	recvBox.setBgColor(al::White);
	recvBox.setEdgeType(gui::Window::EDGE_BEVELED_INWARD);
	recvBox.setPadding({8,8,8,8});

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
			this->gui->app()->currentEncryptionMode = (ENCRYPTION_MODE)mode;
			println("Encryption mode changed to {}", encModeNames[mode]);
		}
	});


	addChildren({
		recvBox, sendBox, sendBtn,
		encMode
	});

	updateLog();
}




void ChatWindow::tick()
{
	std::lock_guard<std::mutex> lk(mtx);
	Window::tick();
}

std::string StrMerge(const std::deque<std::string>& deq)
{
	std::string ret;
	for(const auto& ln: deq) {
		ret += ln + "\n";
	}
	return ret;
}

void ChatWindow::updateLog()
{
	recvBox.setText(StrMerge(log));
	recvBox.updateIfNeeded();
	while(recvBox.getSpan().height() >= recvBox.getHeight()-10) {
		log.pop_front();
		recvBox.setText(StrMerge(log));
		recvBox.updateIfNeeded();
	}
}

void ChatWindow::appendToLog(const std::string_view text)
{
	std::lock_guard<std::mutex> lk(mtx);

	log.push_back(std::string(text));

	updateLog();
}

void ChatWindow::acknowledgeReceivedMessage(const std::string_view msg)
{
	println("Received: {}", msg);
}

void ChatWindow::onSend()
{
	std::string msg = sendBox.getText();
	if(!msg.size()) {
		return;
	}
	println("Sending: {}", msg);
	sendMessage(msg);
	sendBox.setText("");
}


void ChatWindow::sendMessage(std::string msg)
{
	gui->app()->SendMessage(msg).Then<int>([](uint32_t v)->int{return 0;});
}
