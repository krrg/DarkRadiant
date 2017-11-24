#include "GameSetupPageIdTech.h"

#include "i18n.h"
#include "imodule.h"
#include "igame.h"

#include "wxutil/PathEntry.h"

#include <fmt/format.h>
#include <wx/sizer.h>
#include <wx/textctrl.h>
#include <wx/stattext.h>
#include <wx/panel.h>

#include "string/trim.h"
#include "os/file.h"
#include "os/path.h"
#include "registry/Widgets.h"
#include "settings/Win32Registry.h"

namespace ui
{

GameSetupPageIdTech::GameSetupPageIdTech(wxWindow* parent, const game::IGamePtr& game) :
	GameSetupPage(parent, game),
	_fsGameEntry(nullptr),
	_fsGameBaseEntry(nullptr),
	_enginePathEntry(nullptr)
{
	wxFlexGridSizer* table = new wxFlexGridSizer(3, 2, wxSize(6, 6));
	this->SetSizer(table);

	_enginePathEntry = new wxutil::PathEntry(this, true);
	_enginePathEntry->getEntryWidget()->SetMinClientSize(
		wxSize(_enginePathEntry->getEntryWidget()->GetCharWidth() * 30, -1));

	table->Add(new wxStaticText(this, wxID_ANY, _("Engine Path")), 0, wxALIGN_CENTRE_VERTICAL);
	table->Add(_enginePathEntry, 0);

	_fsGameEntry = new wxTextCtrl(this, wxID_ANY);
	_fsGameEntry->SetMinClientSize(wxSize(_fsGameEntry->GetCharWidth() * 30, -1));
	table->Add(new wxStaticText(this, wxID_ANY, _("Mod (fs_game)")), 0, wxALIGN_CENTRE_VERTICAL);
	table->Add(_fsGameEntry, 0);

	_fsGameBaseEntry = new wxTextCtrl(this, wxID_ANY);
	_fsGameBaseEntry->SetMinClientSize(wxSize(_fsGameEntry->GetCharWidth() * 30, -1));
	table->Add(new wxStaticText(this, wxID_ANY, _("Mod Base (fs_game_base, optional)")), 0, wxALIGN_CENTRE_VERTICAL);
	table->Add(_fsGameBaseEntry, 0);
}

const char* GameSetupPageIdTech::TYPE()
{
	return "idTechGeneric";
}

const char* GameSetupPageIdTech::getType()
{
	return TYPE();
}

void GameSetupPageIdTech::validateSettings()
{
	constructPaths();
	
	std::string errorMsg;

	if (!os::fileOrDirExists(_enginePath))
	{
		// Engine path doesn't exist
		errorMsg += fmt::format(_("Engine path \"{0}\" does not exist.\n"), _enginePath);
	}

	// Check the mod base path, if not empty
	if (!_modBasePath.empty() && !os::fileOrDirExists(_modBasePath))
	{
		// Mod base name is not empty, but folder doesnt' exist
		errorMsg += fmt::format(_("The mod base path \"{0}\" does not exist.\n"), _modBasePath);
	}

	// Check the mod path, if not empty
	if (!_modPath.empty() && !os::fileOrDirExists(_modPath))
	{
		// Mod name is not empty, but mod folder doesnt' exist
		errorMsg += fmt::format(_("The mod path \"{0}\" does not exist.\n"), _modPath);
	}

	if (!errorMsg.empty())
	{
		throw GameSettingsInvalidException(errorMsg);
	}
}

std::string GameSetupPageIdTech::getEnginePath()
{
	return _enginePath;
}

std::string GameSetupPageIdTech::getModBasePath()
{
	return _modBasePath;
}

std::string GameSetupPageIdTech::getModPath()
{
	return _modPath;
}

void GameSetupPageIdTech::onPageShown()
{
	// Load the values from the registry if the controls are still empty
	if (_enginePathEntry->getValue().empty())
	{
		_enginePath = registry::getValue<std::string>(RKEY_ENGINE_PATH);

		if (_enginePath.empty())
		{
			// No engine path known, but we have a valid game description
			// Try to deduce the engine path from the Registry settings (Win32 only)
			std::string regKey = _game->getKeyValue("registryKey");
			std::string regValue = _game->getKeyValue("registryValue");

			rMessage() << "GameSetupPageIdTech: Querying Windows Registry for game path: "
				<< "HKEY_LOCAL_MACHINE\\" << regKey << "\\" << regValue << std::endl;

			// Query the Windows Registry for a default installation path
			// This will return "" for non-Windows environments
			_enginePath = game::Win32Registry::getKeyValue(regKey, regValue);

			rMessage() << "GameManager: Windows Registry returned result: " << _enginePath << std::endl;
		}

		// If the engine path is still empty, consult the .game file for a fallback value
		if (_enginePath.empty())
		{
			// No engine path set so far, search the game file for default values
			const std::string ENGINEPATH_ATTRIBUTE =
#if defined(WIN32)
				"enginepath_win32"
#elif defined(__linux__) || defined (__FreeBSD__)
				"enginepath_linux"
#elif defined(__APPLE__)
				"enginepath_macos"
#else
#error "unknown platform"
#endif
				;

			_enginePath = os::standardPathWithSlash(_game->getKeyValue(ENGINEPATH_ATTRIBUTE));
		}

		_enginePathEntry->setValue(_enginePath);
	}

	if (_fsGameEntry->GetValue().empty())
	{
		// Check if we have a valid mod path
		_modPath = registry::getValue<std::string>(RKEY_MOD_PATH);

		if (!_modPath.empty())
		{
			// Extract the fs_game part from the absolute mod path, if possible
			std::string fsGame = os::getRelativePath(_modPath, _enginePath);
			string::trim_right(fsGame, "/");

			_fsGameEntry->SetValue(fsGame);
		}
	}

	if (_fsGameBaseEntry->GetValue().empty())
	{
		// Check if we have a valid mod base path
		_modBasePath = registry::getValue<std::string>(RKEY_MOD_BASE_PATH);

		if (!_modBasePath.empty())
		{
			std::string fsGameBase = os::getRelativePath(_modBasePath, _enginePath);
			string::trim_right(fsGameBase, "/");

			// Extract the fs_game part from the absolute mod base path, if possible
			_fsGameBaseEntry->SetValue(fsGameBase);
		}
	}
}

void GameSetupPageIdTech::constructPaths()
{
	_enginePath = _enginePathEntry->getEntryWidget()->GetValue().ToStdString();

	// Make sure it's a well formatted path
	_enginePath = os::standardPathWithSlash(_enginePath);

	// Load the fsGame and fsGameBase from the registry
	std::string fsGame = _fsGameEntry->GetValue().ToStdString();
	std::string fsGameBase = _fsGameBaseEntry->GetValue().ToStdString();

	if (!fsGameBase.empty())
	{
		// greebo: #3480 check if the mod base path is absolute. If not, append it to the engine path
		_modBasePath = fs::path(fsGameBase).is_absolute() ? fsGameBase : _enginePath + fsGameBase;

		// Normalise the path as last step
		_modBasePath = os::standardPathWithSlash(_modBasePath);
	}
	else
	{
		// No fs_game_base, no mod base path
		_modBasePath = "";
	}

	if (!fsGame.empty())
	{
		// greebo: #3480 check if the mod path is absolute. If not, append it to the engine path
		_modPath = fs::path(fsGame).is_absolute() ? fsGame : _enginePath + fsGame;

		// Normalise the path as last step
		_modPath = os::standardPathWithSlash(_modPath);
	}
	else
	{
		// No fs_game, no modpath
		_modPath = "";
	}
}

}
