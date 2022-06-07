
#include <app/AppState.hpp>

#include <chrono>
#include <thread>
#include <iostream>

#include <lpg/gui/gui.hpp>
#include <axxegro/axxegro.hpp>

#include <functional>


class MessageBox: public gui::Window {
	gui::Text txt;
public:
	MessageBox(al::Coord<> pos, const std::string_view text, const std::string_view title = "Message")
		: gui::Window({320, 240}, pos),
		  txt({320, 240}, {}, text)
	{
		txt.setAlignment(ALIGN_CENTER);


		addChild(txt);
		setTitle(std::string(title));
		give(std::make_unique<gui::TitleBar>());
		setZIndex(311);
	}
};

class AddressWindow: public gui::Window {

	gui::Text txtAddr, txtPort;
	gui::TextBox inAddr, inPort;

	std::function<void(std::string, int)> handler;

	gui::Button connBtn;
public:
	AddressWindow(al::Coord<> pos, std::function<void(std::string, int)> handler)
		: Window({200, 150}, pos),
		  txtAddr({70, 20}, {20, 30}, "Address: "),
		  txtPort({70, 20}, {20, 60}, "Port: "),
		  inAddr({100, 20}, {70, 30}, "127.0.0.1"),
		  inPort({100, 20}, {70, 60}, "10000"),
		  connBtn({70, 30}, {75, 100}, "Connect"),
		  handler(handler)
	{
		setTitle("Connect to a peer");
		give(std::make_unique<gui::TitleBar>());

		connBtn.setCallback([&](){
			int portNum = std::stoi(inPort.getText());
			this->handler(inAddr.getText(), portNum);
		});

		addChild(txtAddr);
		addChild(txtPort);
		addChild(inAddr);
		addChild(inPort);
		addChild(connBtn);
	}
};

class ChatWindow: public gui::Window {

	gui::Text recvBox;
	gui::TextBox sendBox;

	gui::Button sendBtn;
	std::string buf;
	bool bufChanged;

	std::shared_ptr<AppState> app;
public:
	ChatWindow(al::Coord<> pos, std::shared_ptr<AppState> app)
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

	void tick()
	{
		if(bufChanged) {
			bufChanged = false;
			recvBox.setText(buf);
		}
	}

	void appendToLog(const std::string_view text)
	{
		buf += text;
		bufChanged = true;
	}

	void acknowledgeReceivedMessage(const std::string_view msg)
	{
		appendToLog("Received: " + std::string(msg) + "\n");
	}
	
	void sendMessage(std::string msg)
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
	
};


int main(int argc, char** argv) 
{
	srand(time(NULL));
	al::FullInit();
	al::Display disp(1024, 768);


	al::Config resCfg("gui/default.ini");
	
	gui::Window::RM.registerDefaultLoaders();
	gui::Window::RM.loadFromConfig(resCfg);


	int myport = (rand()%20000)+10000;

	disp.setTitle(fmt::format("BSK project @ 127.0.0.1:{}",myport));
	auto app = std::make_shared<AppState>("127.0.0.1", myport);

	gui::Desktop desk;
	desk.setBgColor(al::RGB(0,127,127));

	gui::Text addrp({300, 40}, {30, 30});
	addrp.setTextColor(al::White);
	addrp.setText(fmt::format("Server running on {}:{}", app->ipAddress, app->port));
	addrp.setZIndex(-311);
	desk.addChild(addrp);

	ChatWindow chatWin({250, 250}, app);
	
	desk.addChild(chatWin);

	desk.give(std::make_unique<AddressWindow>(al::Coord<>{40, 40}, [&](std::string addr, int port){
		
		try {
			app->ConnectAndHandshake(addr, port);
			chatWin.appendToLog(fmt::format("Conversation initiated with {}:{}\n", addr, port));
		} catch(KexError& e) {
			auto mb = std::make_unique<MessageBox>(al::Coord<>{400, 400}, fmt::format("Handshake failed: {}", e.what()));
			desk.give(std::move(mb));
		}

	}));

	std::atomic_bool exitFlag = false;

	std::thread thread([&chatWin, &exitFlag](std::shared_ptr<AppState> app) {
			while(!exitFlag) {
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
				std::string msg;
				while(app->PopMessage(msg)) {
					chatWin.acknowledgeReceivedMessage(msg);
				}
			}
		}, app
	);


	desk.mainLoop();
	exitFlag = true;
	thread.join();

	return 0;
}

