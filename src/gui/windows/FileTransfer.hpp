#ifndef SRC_GUI_WINDOWS_FILETRANSFER
#define SRC_GUI_WINDOWS_FILETRANSFER

#include <lpg/gui/gui.hpp>
#include <lpg/gui/ProgressBar.hpp>

#include "ChatWindow.hpp"

class FileTransferWin: public gui::Window {
public:
	FileTransferWin(const al::Coord<> pos, ChatWindow* cw);
private:
	gui::ProgressBar progBar;
	ChatWindow* cw;
};

#endif /* SRC_GUI_WINDOWS_FILETRANSFER */
