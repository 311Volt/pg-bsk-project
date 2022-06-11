#ifndef SRC_GUI_CONTROLS_PROGRESSBAR
#define SRC_GUI_CONTROLS_PROGRESSBAR

#include <lpg/gui/gui.hpp>

class ProgressBar: public gui::Window {
	Window fill;
public:
	ProgressBar(al::Vec2<> size, al::Coord<> pos);

	void setValue(float value);
};

#endif /* SRC_GUI_CONTROLS_PROGRESSBAR */
