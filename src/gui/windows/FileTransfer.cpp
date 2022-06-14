#include "FileTransfer.hpp"

FileTransferWin::FileTransferWin(const al::Coord<> pos, ChatWindow* cw)
	: gui::Window({300, 100}, pos),
	  progBar({0, -10}, {0, 0})
{
	progBar.setAlignment(gui::Window::ALIGN_CENTER_BOTTOM);
}