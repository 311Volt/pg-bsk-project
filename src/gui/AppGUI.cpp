#include "AppGUI.hpp"
#include <fmt/format.h>

const std::string_view CreditsText = R"(
Projekt z przedmiotu Bezpieczeństwo Systemów Komputerowych
Politechnika Gdańska WETI 2022
Jan Trusiłło (s180141@student.pg.edu.pl)
Marek Zalewski (s180465@student.pg.edu.pl)
)";


AppGUI::AppGUI(std::shared_ptr<AppState> appState)
	: serverAddrInfo({300, 40}, {10, 10}, "RPC server not running"),
	  appState(appState), chatWinId(0),
	  chatWindowNeeded(false),
	  keyMgr({70,300}, this)
{
	setBgColor(al::RGB(0,127,127));

	credits.resize({400,200});
	credits.setText(CreditsText);
	credits.setPos({0,0});
	credits.setTextColor(al::White);
	credits.setAlignment(gui::Window::ALIGN_RIGHT_BOTTOM);
	credits.setTextAlignment(gui::Window::ALIGN_RIGHT_BOTTOM);
	credits.setPadding({0,0,6,12});

	serverAddrInfo.setTextColor(al::White);

	auto addrWin = std::make_unique<AddressWindow>(
		al::Coord<>{100, 100},
		[this](std::string addr, int port){
			tryConnect(addr, port);
		}
	);
	give(std::move(addrWin));

	appState->SetReceiveKexCallback([this](){chatWindowNeeded=true;});

	addChildren({serverAddrInfo, credits, keyMgr});

	serverAddrInfo.setZIndex(-1000);
	credits.setZIndex(-1000);
}

AppGUI::~AppGUI()
{

}

AppState* AppGUI::app()
{
	return appState.get();
}

void AppGUI::initiateChat()
{
	auto chatWin = std::make_unique<ChatWindow>(
		al::Coord<>{200,200},
		this
	);

	addChild(*chatWin);

	chatWin->println(
		"Connection initiated with {}:{}",
		appState->theirIPAddress,
		appState->theirPort
	);

	chat = std::make_unique<Chat>(std::move(chatWin), appState);
	chat->start();
	
}

void AppGUI::tryConnect(const std::string& addr, int port)
{
	try {
		appState->ConnectAndHandshake(addr, port);
		initiateChat();
	} catch(std::runtime_error& e) {
		give(std::make_unique<MessageBox>(
			al::Coord<>{300,300}, 
			fmt::format("Connection failed: {}", e.what()),
			"Error"
		));
	}
}

void AppGUI::run()
{

	serverAddrInfo.setText(fmt::format(
		"RPC server running at {}:{}",
		appState->ipAddress,
		appState->port	
	));

	mainLoop();
}

void AppGUI::tick()
{
	std::lock_guard<std::mutex> lk(mtx);
	
	while(!msgBoxQueue.empty()) {
		al::Coord<unsigned> offset;
		Random::Fill(&offset, sizeof(offset));
		offset.x %= 100;
		offset.y %= 100;

		give(std::make_unique<MessageBox>(
			al::Coord<>(300,300) + al::Coord<>(offset),
			msgBoxQueue.back(),
			"Message"
		));

		msgBoxQueue.pop();
	}

	if(chatWindowNeeded) {
		initiateChat();
		chatWindowNeeded = false;
	}

	gui::Desktop::tick();
}

void AppGUI::msgBoxStr(const std::string& msg)
{
	std::lock_guard<std::mutex> lk(mtx);
	msgBoxQueue.push(msg);
}
