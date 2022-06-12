#include "PasswordField.hpp"

void PasswordField::updateText()
{
	txt.setText(std::string(buffer.size(), '*'));
	txt.updateIfNeeded();
	cursor.setPos(al::Coord<>(txt.getSpan().b.x, 0));
}