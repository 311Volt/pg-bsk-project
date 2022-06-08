#ifndef SRC_GUI_WINDOWS_MESSAGEBOX
#define SRC_GUI_WINDOWS_MESSAGEBOX

#include <lpg/gui/gui.hpp>

class MessageBox: public gui::Window {
	gui::Text txt;
	gui::Button ok;
public:
	MessageBox(al::Coord<> pos, const std::string_view text, const std::string_view title = "Message");
};

#endif /* SRC_GUI_WINDOWS_MESSAGEBOX */
