#ifndef SRC_GUI_CONTROLS_ADDRESSWINDOW
#define SRC_GUI_CONTROLS_ADDRESSWINDOW

#include <lpg/gui/gui.hpp>

class AddressWindow: public gui::Window {

	gui::Text txtAddr, txtPort;
	gui::TextBox inAddr, inPort;

	std::function<void(std::string, int)> handler;

	gui::Button connBtn;
public:
	AddressWindow(al::Coord<> pos, std::function<void(std::string, int)> handler);
};

#endif /* SRC_GUI_CONTROLS_ADDRESSWINDOW */
