#include "FileTransferWin.hpp"

#include "../../app/FileTransfer.hpp"
#include "ChatWindow.hpp"

#include <filesystem>
namespace stdfs = std::filesystem;

FileTransferWin::FileTransferWin(const al::Coord<> pos, ChatWindow* cw, FileTransfer* tr)
	: gui::Window({250, 80}, pos),
	  cWin(cw),
	  transfer(tr),
	  progBar({150, 20}, {0, 55}),
	  btnAccept({80, 24}, {0, -10}, "Accept"),
	  status({250, 40}, {5, 5}, "Waiting...")
{
	needsUpdate = false;

	btnAccept.setAlignment(gui::Window::ALIGN_CENTER_BOTTOM);
	progBar.setAlignment(gui::Window::ALIGN_CENTER_TOP);

	btnAccept.visible = false;
	if(auto* recv = dynamic_cast<FileTransferRecv*>(tr)) {
		status.setText(fmt::format(
			"Peer wants to send {} ({} bytes). Accept?", 
			stdfs::path(recv->fileName()).filename().string(), 
			recv->bytesTotal()
		));
		btnAccept.setCallback([recv](){
			recv->acceptTransfer();
		});
		btnAccept.visible = true;
	}

	tr->setOnAcceptFn([this](){onAccept();});
	tr->setOnUpdateFn([this](){needsUpdate = true;});
	

	addChildren({progBar, btnAccept, status});

	progBar.visible = false;
}

void FileTransferWin::tick()
{
	Window::tick();

	if(needsUpdate) {
		onUpdate();
		needsUpdate = false;
	}
}

void FileTransferWin::onAccept()
{
	progBar.visible = true;
	btnAccept.visible = false;

	onUpdate();
}

void FileTransferWin::onUpdate()
{
	double progress = (double)transfer->bytesCompleted() / (double)transfer->bytesTotal();

	progBar.setValue(progress);
	status.setText(fmt::format(
		"Transferring {}\n({} / {} bytes)",
		stdfs::path(transfer->fileName()).filename().string(),
		transfer->bytesCompleted(),
		transfer->bytesTotal()
	));
}

void FileTransferWin::onFinish()
{
	cWin->println(
		"{} was {} successfully",
		transfer->fileName(),
		dynamic_cast<FileTransferSend*>(transfer) ? "sent" : "received"
	);
}