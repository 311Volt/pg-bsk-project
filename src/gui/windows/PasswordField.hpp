#ifndef SRC_GUI_WINDOWS_PASSWORDFIELD
#define SRC_GUI_WINDOWS_PASSWORDFIELD

#include <lpg/gui/TextBox.hpp>

class PasswordField: public gui::TextBox {
public:
	using gui::TextBox::TextBox;
	virtual void updateText() override;
};	

#endif /* SRC_GUI_WINDOWS_PASSWORDFIELD */
