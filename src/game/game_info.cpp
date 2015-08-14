/* $Id$ */

/*
 * This file is part of OpenTTD.
 * OpenTTD is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
 * OpenTTD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with OpenTTD. If not, see <http://www.gnu.org/licenses/>.
 */

/** @file game_info.cpp Implementation of GameInfo */

#include "../stdafx.h"

#include "../script/convert.hpp"
#include "../script/script_scanner.hpp"
#include "game_info.hpp"
#include "../debug.h"

static const char *const game_api_versions[] =
	{ "1.2", "1.3", "1.4", "1.5", "1.6" };

/* static */ void GameInfo::RegisterAPI(Squirrel *engine)
{
	/* Create the GSInfo class, and add the RegisterGS function */
	engine->AddClassBegin ("GSInfo");
	SQConvert::AddConstructor <void (GameInfo::*)(), 1> (engine, "x");
	SQConvert::DefSQAdvancedMethod (engine, "GSInfo", &GameInfo::AddSetting, "AddSetting");
	SQConvert::DefSQAdvancedMethod (engine, "GSInfo", &GameInfo::AddLabels,  "AddLabels");
	engine->AddConst ("CONFIG_NONE",      SCRIPTCONFIG_NONE);
	engine->AddConst ("CONFIG_RANDOM",    SCRIPTCONFIG_RANDOM);
	engine->AddConst ("CONFIG_BOOLEAN",   SCRIPTCONFIG_BOOLEAN);
	engine->AddConst ("CONFIG_INGAME",    SCRIPTCONFIG_INGAME);
	engine->AddConst ("CONFIG_DEVELOPER", SCRIPTCONFIG_DEVELOPER);
	engine->AddClassEnd();

	engine->AddMethod("RegisterGS", &GameInfo::Constructor, 2, "tx");
}

/* static */ SQInteger GameInfo::Constructor(HSQUIRRELVM vm)
{
	/* Get the GameInfo */
	SQUserPointer instance = NULL;
	if (SQ_FAILED(sq_getinstanceup(vm, 2, &instance, 0)) || instance == NULL) return sq_throwerror(vm, "Pass an instance of a child class of GameInfo to RegisterGame");
	GameInfo *info = (GameInfo *)instance;

	ScriptScanner *scanner = ScriptScanner::Get (vm);

	SQInteger res = scanner->construct (info);
	if (res != 0) return res;

	/* When there is an IsSelectable function, call it. */
	bool is_developer_only;
	if (scanner->method_exists ("IsDeveloperOnly")) {
		if (!scanner->call_bool_method ("IsDeveloperOnly", MAX_GET_OPS, &is_developer_only)) return SQ_ERROR;
	} else {
		is_developer_only = false;
	}

	/* Remove the link to the real instance, else it might get deleted by RegisterGame() */
	sq_setinstanceup(vm, 2, NULL);
	/* Register the Game to the base system */
	scanner->RegisterScript (info, info->GetName(), is_developer_only);
	return 0;
}

SQInteger GameInfo::construct (ScriptScanner *scanner)
{
	return this->ScriptVersionedInfo::construct (scanner,
						game_api_versions, NULL);
}


/* static */ void GameLibrary::RegisterAPI(Squirrel *engine)
{
	/* Create the GameLibrary class, and add the RegisterLibrary function */
	engine->AddClassBegin("GSLibrary");
	engine->AddClassEnd();
	engine->AddMethod("RegisterLibrary", &GameLibrary::Constructor, 2, "tx");
}

/* static */ SQInteger GameLibrary::Constructor(HSQUIRRELVM vm)
{
	/* Create a new library */
	GameLibrary *library = new GameLibrary();

	ScriptScanner *scanner = ScriptScanner::Get (vm);

	SQInteger res = scanner->construct (library);
	if (res != 0) {
		delete library;
		return res;
	}

	/* Register the Library to the base system */
	char name [1024];
	bstrfmt (name, "%s.%s", library->GetCategory(), library->GetInstanceName());
	scanner->RegisterScript (library, name);

	return 0;
}
