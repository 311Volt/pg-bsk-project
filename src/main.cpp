
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
	MessageBox(int x, int y, const std::string_view text)
		: gui::Window(x, y, 320, 240),
		  txt(0, 0, al::CurrentDisplay.width(), al::CurrentDisplay.height())
	{
			txt.setAlignment(gui::Window::Alignment::CENTER);
			txt.setSizeMode(gui::Text::SizeMode::AUTO);
			txt.setText(text);


			resize(txt.getSize() + al::Vec2(40, 70));
			addChild(txt);
			setTitle(" ");
			give(std::make_unique<gui::TitleBar>());
			setZIndex(-311);
	}
};

class AddressWindow: public gui::Window {

	gui::Text txtAddr, txtPort;
	gui::TextBox inAddr, inPort;

	std::function<void(std::string, int)> handler;

	gui::Button connBtn;
public:
	AddressWindow(int x, int y, std::function<void(std::string, int)> handler)
		: gui::Window(200, 150, x, y),
		  txtAddr(20, 30, 70, 20),
		  txtPort(20, 60, 70, 20),
		  inAddr(70, 30, 100, 20),
		  inPort(70, 60, 100, 20),
		  handler(handler),
		  connBtn(70, 30, 75, 100)
	{
		setTitle("Connect to a peer");
		give(std::make_unique<gui::TitleBar>());

		txtAddr.setText("Address: ");
		txtPort.setText("Port: ");
		connBtn.setTitle("Connect");

		inAddr.setText("127.0.0.1");
		inPort.setText("10000");

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
	ChatWindow(int x, int y, std::shared_ptr<AppState> app)
		: gui::Window(640, 480, 100, 100),
		  recvBox(20, 30, 600, 300),
		  sendBox(20, 340, 570, 20),
		  sendBtn(30, 20, 590, 340),
		  app(app)
	{
		bufChanged = true;
		setTitle(fmt::format("Chat: {}:{}", app->ipAddress, app->port));
		give(std::make_unique<gui::TitleBar>());

		recvBox.setBgColor(al::White);
		recvBox.setEdgeType(gui::Window::EdgeType::BEVELED_INWARD);

		sendBox.setText("");

		addChild(recvBox);
		addChild(sendBox);
		addChild(sendBtn);

		sendBtn.setTitle("Send");
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

	gui::Text addrp(10, 10, al::CurrentDisplay.width(), al::CurrentDisplay.height());
	addrp.setSizeMode(gui::Text::SizeMode::AUTO);
	addrp.setTextColor(al::White);
	addrp.setText(fmt::format("Server running on {}:{}", app->ipAddress, app->port));
	addrp.setZIndex(311);
	desk.addChild(addrp);

	ChatWindow chatWin(250, 250, app);
	
	desk.addChild(chatWin);

	desk.give(std::make_unique<AddressWindow>(40, 40, [&](std::string addr, int port){
		
		try {
			app->ConnectAndHandshake(addr, port);
			chatWin.appendToLog(fmt::format("Conversation initiated with {}:{}\n", addr, port));
		} catch(KexError& e) {
			auto mb = std::make_unique<MessageBox>(400, 400, fmt::format("Handshake failed: {}", e.what()));
			desk.give(std::move(mb));
		}

	}));

	std::thread thread([&chatWin](std::shared_ptr<AppState> app) {
			while(true) {
				std::this_thread::sleep_for(std::chrono::milliseconds(500));
				std::string msg;
				while(app->PopMessage(msg)) {
					chatWin.acknowledgeReceivedMessage(msg);
				}
			}
		}, app
	);


	desk.mainLoop();

	return 0;
}

