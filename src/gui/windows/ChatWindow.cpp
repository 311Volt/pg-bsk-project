#include "ChatWindow.hpp"
#include "MessageBox.hpp"

#include "../../app/FuturePromise.hpp"

#include <fmt/format.h>
#include <filesystem>

namespace stdfs = std::filesystem;

ChatWindow::ChatWindow(al::Coord<> pos, std::shared_ptr<AppState> app)
	: Window({640, 480}, pos),
		recvBox({600, 300}, {20, 30}),
		sendBox({570, 20}, {20, 340}),
		sendBtn({30, 20}, {590, 340}, "Send"),
		app(app),
		encMode({200, 20}, {20, 380}, {200, 100}),
		btnGenKP({120, 24}, {240, 380}, "Generate key pair"),
		btnLoadKP({120, 24}, {240, 410}, "Load key pair..."),
		btnSaveKP({120, 24}, {240, 440}, "Save key pair..."),
		lpCaption({30, 24}, {370, 410}, "pass: "),
		spCaption({30, 24}, {370, 440}, "pass: "),
		pfLoadPass({200, 24}, {405, 410}),
		pfSavePass({200, 24}, {405, 440}),
		keyText({250, 24}, {370, 380}, "no key data available")
{
	setTitle(fmt::format("Chat: {}:{}", app->ipAddress, app->port));
	give(std::make_unique<gui::TitleBar>());

	recvBox.setBgColor(al::White);
	recvBox.setEdgeType(gui::Window::EDGE_BEVELED_INWARD);
	recvBox.setPadding({8,8,8,8});

	btnGenKP.setCallback([this](){genKeyPair();});
	btnLoadKP.setCallback([this](){loadKeyPairDialog();});
	btnSaveKP.setCallback([this](){saveKeyPairDialog();});

	sendBox.setOnReturnCallback([this](){onSend();});
	sendBtn.setCallback([this](){onSend();});

	encMode.setElements({
		{CHACHA20_POLY1305, "CHACHA20_POLY1305"},
		{CHACHA20, "CHACHA20"}
	});
	encMode.setOnChangeCallback([this](uint32_t mode){
		static std::unordered_map<uint32_t, std::string> encModeNames = {
			{CHACHA20_POLY1305, "CHACHA20_POLY1305"},
			{CHACHA20, "CHACHA20"}
		};

		if(encModeNames.count(mode)) {
			this->app->currentEncryptionMode = (ENCRYPTION_MODE)mode;
			println("Encryption mode changed to {}", encModeNames[mode]);
		}
	});

	lpCaption.setTextAlignment(ALIGN_RIGHT_CENTER);
	spCaption.setTextAlignment(ALIGN_RIGHT_CENTER);
	keyText.setTextAlignment(ALIGN_LEFT_CENTER);

	addChildren({
		recvBox, sendBox, sendBtn,
		btnGenKP, btnLoadKP, btnSaveKP,
		pfLoadPass, pfSavePass, lpCaption, spCaption,
		encMode, keyText
	});

	updateKeyText();
	updateLog();
}



struct KeyPairPaths {
	std::string privKeyPath, pubKeyPath;
};

std::optional<KeyPairPaths> CheckKeyPathsLoad(const std::vector<std::string>& paths)
{
	KeyPairPaths ret;
	int priv{}, pub{};
	for(const auto& path: paths) {
		stdfs::path fsp(path);
		if(fsp.extension() == ".pub") {
			pub++;
			ret.pubKeyPath = path;
		}
		if(fsp.extension() == ".key") {
			priv++;
			ret.privKeyPath = path;
		}
	}
	if(priv==1 && pub==1) {
		return ret;
	}
	return {};
}

std::optional<KeyPairPaths> CheckKeyPathsSave(const std::vector<std::string>& paths)
{
	return {};
}

void ChatWindow::genKeyPair()
{
	app->GenerateKey();
	println("Generated new key");
	updateKeyText();
}

class KeyMgrError: public std::runtime_error{
public:
	using std::runtime_error::runtime_error;

	template<typename... Args>
	KeyMgrError(const std::string_view fmt, Args... args)
		: KeyMgrError(fmt::format(fmt, args...))
	{}
};

void ChatWindow::tryLoadKeyPair(al::FileDialogResult& r)
{
	auto kp = CheckKeyPathsLoad(r.paths);
	if(!kp) {
		throw KeyMgrError("One .pub and one .key file need to be provided.");
	}

	auto pp = pfLoadPass.getText();
	auto lr = app->LoadPrivateKey(kp->privKeyPath, pp);
	if(!lr) {
		throw KeyMgrError("Make sure {} exists and check the passphrase.", kp->privKeyPath);
	}
	auto lrp = app->LoadPublicKey(kp->pubKeyPath);
	if(!lrp) {
		throw KeyMgrError("Make sure {} exists.", kp->pubKeyPath);
	}

	println(
		"Key pair loaded from {}.(key,pub) successfully\n",
		stdfs::path(kp->pubKeyPath).replace_extension().string()
	);

	updateKeyText();
	pfLoadPass.setText("");
}

void ChatWindow::trySaveKeyPair(al::FileDialogResult& r)
{
	if(r.paths.size() > 1) {
		throw KeyMgrError("1 path expected, {} provided", r.paths.size());
	}
	stdfs::path path(r.paths.at(0));

	if(path.extension().string().size() && path.extension() != ".pub" && path.extension() != ".key") {
		throw KeyMgrError("{} exists and is not a key", path.string());
	}

	auto ppriv = stdfs::path(path).replace_extension(".key");
	auto ppub = stdfs::path(path).replace_extension(".pub");
	auto pp = pfSavePass.getText();

	app->SavePrivateKey(ppriv.string(), pp);
	app->SavePublicKey(ppub.string());

	println(
		"Current key pair saved to {}.(key,pub)\n",
		stdfs::path(path).replace_extension().string()
	);

	pfSavePass.setText("");
	updateKeyText();
}

void ChatWindow::loadKeyPairDialog()
{
	al::FileDialog dialog(
		"keys", "Pick a key...", "*.key;*.pub", 
		ALLEGRO_FILECHOOSER_MULTIPLE | ALLEGRO_FILECHOOSER_FILE_MUST_EXIST
	);

	Future(dialog.showAsync().share()).Then<int>([this](al::FileDialogResult res)->int{
		if(!res.wasCancelled()) {
			try {
				tryLoadKeyPair(res);
			} catch(KeyMgrError& err) {
				queueMsgBox("Cannot load key: " + std::string(err.what()));
			}
		}
		return 0;
	});
}

void ChatWindow::saveKeyPairDialog()
{
	al::FileDialog dialog("keys", "Save key...", "*.key;*.pub", ALLEGRO_FILECHOOSER_SAVE);

	Future(dialog.showAsync().share()).Then<int>([this](al::FileDialogResult res)->int{
		if(!res.wasCancelled()) {
			try {
				trySaveKeyPair(res);
			} catch(KeyMgrError& err) {
				queueMsgBox("Cannot save key: " + std::string(err.what()));
			}
		}
		return 0;
	});
}

void ChatWindow::queueMsgBox(const std::string& msg)
{
	std::lock_guard<std::mutex> lk(mtx);

	msgBoxQueue.push(msg);
}

void ChatWindow::tick()
{
	std::lock_guard<std::mutex> lk(mtx);

	while(!msgBoxQueue.empty()) {
		auto m = msgBoxQueue.back();
		msgBoxQueue.pop();
		al::Coord<> pos = al::Coord<>{200,200} + al::Coord<>(rand()%100, rand()%100);
		parent->give(std::make_unique<MessageBox>(pos,m));
	}

	Window::tick();
}

std::string StrMerge(const std::deque<std::string>& deq)
{
	std::string ret;
	for(const auto& ln: deq) {
		ret += ln + "\n";
	}
	return ret;
}

void ChatWindow::updateLog()
{
	recvBox.setText(StrMerge(log));
	recvBox.updateIfNeeded();
	while(recvBox.getSpan().height() >= recvBox.getHeight()-10) {
		log.pop_front();
		recvBox.setText(StrMerge(log));
		recvBox.updateIfNeeded();
	}
}

void ChatWindow::appendToLog(const std::string_view text)
{
	std::lock_guard<std::mutex> lk(mtx);

	log.push_back(std::string(text));

	updateLog();
}

void ChatWindow::acknowledgeReceivedMessage(const std::string_view msg)
{
	println("Received: {}", msg);
}

void ChatWindow::onSend()
{
	std::string msg = sendBox.getText();
	if(!msg.size()) {
		return;
	}
	println("Sending: {}", msg);
	sendMessage(msg);
	sendBox.setText("");
}

void ChatWindow::updateKeyText()
{
	Array32 privKeyHash;
	digest::sha256().absorb(app->privateKey).finalize(privKeyHash.data());

	std::string fingerprint;
	for(uint8_t b: privKeyHash) {
		fingerprint += fmt::format("{:02x}", b);
	}

	fingerprint = fingerprint.substr(0,12) + "...";

	keyText.setText(fmt::format("Current key fingerprint: {}", fingerprint));
}

void ChatWindow::sendMessage(std::string msg)
{
	app->SendMessage(msg).Then<int>([](uint32_t v)->int{return 0;});
}
