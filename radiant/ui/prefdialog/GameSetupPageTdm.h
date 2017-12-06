#pragma once

#include "GameSetupPage.h"

class wxComboBox;
namespace wxutil { class PathEntry; }

namespace ui
{

/**
* A Setup Page for the standalone version
* of The Dark Mod (version 2.0 and later).
*/
class GameSetupPageTdm :
	public GameSetupPage
{
private:
	wxComboBox* _missionEntry;
	wxutil::PathEntry* _enginePathEntry;

	std::string _fmFolder;

public:
	GameSetupPageTdm(wxWindow* parent, const game::IGamePtr& game);

	static const char* TYPE();

	const char* getType() override;
	void validateSettings() override;
	void onPageShown() override;

private:
	void constructPaths();
	void populateAvailableMissionPaths();
};

}
