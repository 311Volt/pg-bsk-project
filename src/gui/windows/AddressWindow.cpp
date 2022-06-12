#include "AddressWindow.hpp"

#include "../../app/AppState.hpp"

AddressWindow::AddressWindow(al::Coord<> pos, std::function<void(std::string, int)> handler)
	: Window({200, 150}, pos),
	  txtAddr({50, 20}, {17, 30}, "Address: "),
	  txtPort({50, 20}, {17, 60}, "Port: "),
	  inAddr({100, 20}, {70, 30}, "127.0.0.1"),
	  inPort({100, 20}, {70, 60}, "10000"),
	  connBtn({70, 30}, {75, 100}, "Connect"),
	  connDbg({30, 30}, {150, 100}, "dbg"),
	  handler(handler)
{
	setTitle("Connect to a peer");
	give(std::make_unique<gui::TitleBar>());

	connBtn.setCallback([this](){
		int portNum = std::stoi(inPort.getText());
		this->handler(inAddr.getText(), portNum);
	});

	connDbg.setCallback([this](){
		this->handler(
			AppState::singleton->ipAddress,
			AppState::singleton->port
		);
	});

	txtAddr.setTextAlignment(gui::Window::ALIGN_RIGHT_CENTER);
	txtPort.setTextAlignment(gui::Window::ALIGN_RIGHT_CENTER);
	
	addChildren({txtAddr, txtPort, inAddr, inPort, connBtn});

#ifndef NDEBUG
	addChild(connDbg);
#endif
}
