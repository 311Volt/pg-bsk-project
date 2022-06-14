#ifndef SRC_GUI_WINDOWS_FILETRANSFER
#define SRC_GUI_WINDOWS_FILETRANSFER

#include <lpg/gui/gui.hpp>
#include <lpg/gui/ProgressBar.hpp>

class ChatWindow;
class FileTransfer;

class FileTransferWin: public gui::Window {
public:
	FileTransferWin(const al::Coord<> pos, ChatWindow* cw, FileTransfer* tr);

	void onAccept();
	void onUpdate();
	void onFinish();
private:
	bool needsUpdate;
	gui::Button btnAccept;

	virtual void tick() override;

	gui::Text status;
	gui::ProgressBar progBar;
	ChatWindow* cWin;
	FileTransfer* transfer;
};

#endif /* SRC_GUI_WINDOWS_FILETRANSFER */
