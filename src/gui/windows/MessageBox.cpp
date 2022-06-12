#include "MessageBox.hpp"

MessageBox::MessageBox(al::Coord<> pos, const std::string_view text, const std::string_view title)
	: gui::Window({320, 240}, pos),
	  txt({320, 240}, {}, text),
	  ok({90, 30}, {0, -10}, "ok", [this](){setExitFlag();})
{
	
	txt.resizeToFit();
	resize(txt.getSize() + al::Vec2<>(30, 120));
	txt.resize(getSize() - al::Vec2<>(0, 30));

	txt.setTextAlignment(gui::Window::ALIGN_CENTER);

	setTitle(std::string(title));
	give(std::make_unique<gui::TitleBar>());
	setZIndex(-31100);

	ok.setAlignment(gui::Window::ALIGN_CENTER_BOTTOM);

	addChildren({txt, ok});
}
