#ifndef SRC_GUI_WINDOWS_KEYMANAGER
#define SRC_GUI_WINDOWS_KEYMANAGER

#include <lpg/gui/gui.hpp>

#include "PasswordField.hpp"

class AppGUI;
class AppState;

class KeyManager: public gui::Window {
public:
	KeyManager(al::Coord<> pos, AppGUI* gui);

	void genKeyPair();

	void tryLoadKeyPair(al::FileDialogResult& r);
	void trySaveKeyPair(al::FileDialogResult& r);

	void loadKeyPairDialog();
	void saveKeyPairDialog();

	void updateKeyText();
private:
	gui::Button btnGenKP, btnLoadKP, btnSaveKP;
	gui::Text lpCaption, spCaption;
	PasswordField pfLoadPass, pfSavePass;
	gui::Text keyText;


	AppGUI* gui;
};

#endif /* SRC_GUI_WINDOWS_KEYMANAGER */
