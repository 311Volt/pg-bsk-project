#include "KeyManager.hpp"
#include "../AppGUI.hpp"
#include <filesystem>
namespace stdfs = std::filesystem;

KeyManager::KeyManager(const al::Coord<> pos, AppGUI* gui)
	: Window({380, 120}, pos),
		gui(gui),
		btnGenKP({120, 24}, {10, 25}, "Generate key pair"),
		btnLoadKP({120, 24}, {10, 55}, "Load key pair..."),
		btnSaveKP({120, 24}, {10, 85}, "Save key pair..."),
		lpCaption({30, 24}, {140, 55}, "pass: "),
		spCaption({30, 24}, {140, 85}, "pass: "),
		pfLoadPass({200, 24}, {175, 55}),
		pfSavePass({200, 24}, {175, 85}),
		keyText({250, 24}, {140, 25}, "no key data available")

{

	setTitle("Key Manager");
	give(std::make_unique<gui::TitleBar>());

	btnGenKP.setCallback([this](){genKeyPair();});
	btnLoadKP.setCallback([this](){loadKeyPairDialog();});
	btnSaveKP.setCallback([this](){saveKeyPairDialog();});


	lpCaption.setTextAlignment(ALIGN_RIGHT_CENTER);
	spCaption.setTextAlignment(ALIGN_RIGHT_CENTER);
	keyText.setTextAlignment(ALIGN_LEFT_CENTER);

	addChildren({
		btnGenKP, btnLoadKP, btnSaveKP,
		lpCaption, spCaption,
		pfLoadPass, pfSavePass,
		keyText
	});

	updateKeyText();
}


struct KeyPairPaths {
	std::string privKeyPath, pubKeyPath;
};

class KeyMgrError: public std::runtime_error{
public:
	using std::runtime_error::runtime_error;

	template<typename... Args>
	KeyMgrError(const std::string_view fmt, Args... args)
		: KeyMgrError(fmt::format(fmt, args...))
	{}
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

void KeyManager::genKeyPair()
{
	gui->app()->GenerateKey();
	gui->msgBox("Generated new key");
	updateKeyText();
}



void KeyManager::tryLoadKeyPair(al::FileDialogResult& r)
{
	auto kp = CheckKeyPathsLoad(r.paths);
	if(!kp) {
		throw KeyMgrError("One .pub and one .key file need to be provided.");
	}

	auto pp = pfLoadPass.getText();
	auto lr = gui->app()->LoadPrivateKey(kp->privKeyPath, pp);
	if(!lr) {
		throw KeyMgrError("Make sure {} exists and check the passphrase.", kp->privKeyPath);
	}
	auto lrp = gui->app()->LoadPublicKey(kp->pubKeyPath);
	if(!lrp) {
		throw KeyMgrError("Make sure {} exists.", kp->pubKeyPath);
	}

	gui->msgBox(
		"Key pair loaded from {}.(key,pub) successfully\n",
		stdfs::path(kp->pubKeyPath).replace_extension().string()
	);

	updateKeyText();
	pfLoadPass.setText("");
}

void KeyManager::trySaveKeyPair(al::FileDialogResult& r)
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

	gui->app()->SavePrivateKey(ppriv.string(), pp);
	gui->app()->SavePublicKey(ppub.string());

	gui->msgBox(
		"Current key pair saved to {}.(key,pub)\n",
		stdfs::path(path).replace_extension().string()
	);

	pfSavePass.setText("");
	updateKeyText();
}

void KeyManager::loadKeyPairDialog()
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
				gui->msgBox("Cannot load key: " + std::string(err.what()));
			}
		}
		return 0;
	});
}

void KeyManager::saveKeyPairDialog()
{
	al::FileDialog dialog("keys", "Save key...", "*.key;*.pub", ALLEGRO_FILECHOOSER_SAVE);

	Future(dialog.showAsync().share()).Then<int>([this](al::FileDialogResult res)->int{
		if(!res.wasCancelled()) {
			try {
				trySaveKeyPair(res);
			} catch(KeyMgrError& err) {
				gui->msgBox("Cannot save key: " + std::string(err.what()));
			}
		}
		return 0;
	});
}

void KeyManager::updateKeyText()
{
	keyText.setText(fmt::format(
		"Current key fingerprint: {}...", 
		gui->app()->GetKeyFingerprint().substr(0,12))
	);
}