/*  This file is part of PCE.emu.

	PCE.emu is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	PCE.emu is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with PCE.emu.  If not, see <http://www.gnu.org/licenses/> */

#include <emuframework/EmuApp.hh>
#include <emuframework/OptionView.hh>
#include <emuframework/EmuSystemActionsView.hh>
#include <emuframework/EmuInput.hh>
#include "internal.hh"

class ConsoleOptionView : public TableView
{
	BoolMenuItem sixButtonPad
	{
		"6-button Gamepad",
		(bool)option6BtnPad,
		[this](BoolMenuItem &item, View &, Input::Event e)
		{
			EmuSystem::sessionOptionSet();
			bool on = item.flipBoolValue(*this);
			option6BtnPad = on;
			EmuControls::setActiveFaceButtons(on ? 6 : 2);
		}
	};

	BoolMenuItem arcadeCard
	{
		"Arcade Card",
		(bool)optionArcadeCard,
		[this](BoolMenuItem &item, View &, Input::Event e)
		{
			EmuSystem::sessionOptionSet();
			optionArcadeCard = item.flipBoolValue(*this);
			EmuApp::promptSystemReloadDueToSetOption(attachParams(), e);
		}
	};

	std::array<MenuItem*, 2> menuItem
	{
		&sixButtonPad,
		&arcadeCard
	};

public:
	ConsoleOptionView(ViewAttachParams attach):
		TableView
		{
			"Console Options",
			attach,
			menuItem
		}
	{}
};

class CustomSystemActionsView : public EmuSystemActionsView
{
private:
	TextMenuItem options
	{
		"Console Options",
		[this](TextMenuItem &, View &, Input::Event e)
		{
			pushAndShow(makeView<ConsoleOptionView>(), e);
		}
	};

public:
	CustomSystemActionsView(ViewAttachParams attach): EmuSystemActionsView{attach, true}
	{
		item.emplace_back(&options);
		loadStandardItems();
	}
};

class CustomSystemOptionView : public SystemOptionView
{
	char sysCardPathStr[256]{};

	TextMenuItem sysCardPath
	{
		sysCardPathStr,
		[this](TextMenuItem &, View &, Input::Event e)
		{
			auto biosSelectMenu = makeViewWithName<BiosSelectMenu>("System Card", &::sysCardPath,
				[this]()
				{
					logMsg("set bios %s", ::sysCardPath.data());
					printBiosMenuEntryStr(sysCardPathStr);
					sysCardPath.compile(renderer(), projP);
				},
				hasHuCardExtension);
			pushAndShow(std::move(biosSelectMenu), e);
		}
	};

	template <size_t S>
	static void printBiosMenuEntryStr(char (&str)[S])
	{
		string_printf(str, "System Card: %s", strlen(::sysCardPath.data()) ? FS::basename(::sysCardPath).data() : "None set");
	}

public:
	CustomSystemOptionView(ViewAttachParams attach): SystemOptionView{attach, true}
	{
		loadStockItems();
		printBiosMenuEntryStr(sysCardPathStr);
		item.emplace_back(&sysCardPath);
	}
};

std::unique_ptr<View> EmuApp::makeCustomView(ViewAttachParams attach, ViewID id)
{
	switch(id)
	{
		case ViewID::SYSTEM_ACTIONS: return std::make_unique<CustomSystemActionsView>(attach);
		case ViewID::SYSTEM_OPTIONS: return std::make_unique<CustomSystemOptionView>(attach);
		default: return nullptr;
	}
}
