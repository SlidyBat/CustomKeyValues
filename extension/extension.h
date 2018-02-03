/**
 * vim: set ts=4 :
 * =============================================================================
 * SourceMod Sample Extension
 * Copyright (C) 2004-2008 AlliedModders LLC.  All rights reserved.
 * =============================================================================
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License, version 3.0, as published by the
 * Free Software Foundation.
 * 
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * As a special exception, AlliedModders LLC gives you permission to link the
 * code of this program (as well as its derivative works) to "Half-Life 2," the
 * "Source Engine," the "SourcePawn JIT," and any Game MODs that run on software
 * by the Valve Corporation.  You must obey the GNU General Public License in
 * all respects for all other code used.  Additionally, AlliedModders LLC grants
 * this exception to all derivative works.  AlliedModders LLC defines further
 * exceptions, found in LICENSE.txt (as of this writing, version JULY-31-2007),
 * or <http://www.sourcemod.net/license.php>.
 *
 * Version: $Id$
 */

#ifndef _INCLUDE_SOURCEMOD_EXTENSION_PROPER_H_
#define _INCLUDE_SOURCEMOD_EXTENSION_PROPER_H_

/**
 * @file extension.h
 * @brief CustomKeyValues extension code header.
 */

#include "smsdk_ext.h"
#include "ISDKHooks.h"

#include <vector>
#include <string>
#include <unordered_map>

class CustomKeyValues
	:
	public SDKExtension,
	public ISMEntityListener,
	public IMetamodListener
{
public: // SDKExtension
	virtual bool SDK_OnLoad( char* error, size_t maxlength, bool late ) override;
	virtual void SDK_OnUnload() override;
	virtual void SDK_OnAllLoaded() override;
	virtual bool QueryRunning( char* error, size_t maxlength ) override;
public: // ISMEntityListener
	virtual void OnEntityCreated( CBaseEntity* pEntity, const char* classname ) override;
	virtual void OnEntityDestroyed( CBaseEntity* pEntity ) override;
public: // IMetadmodListener
	virtual void OnLevelInit( const char* pMapName, const char* pMapEntities, const char* pOldLevel, const char* pLandmarkName, bool loadGame, bool background ) override;
public: // Hooks
	bool Hook_KeyValue( const char* key, const char* value );
public:
#if defined SMEXT_CONF_METAMOD
	virtual bool SDK_OnMetamodLoad(ISmmAPI *ismm, char *error, size_t maxlength, bool late) override;
#endif
public:
	class Entry
	{
	public:
		Entry( const char* key, const char* value )
			:
			key( key ),
			value( value )
		{}
		std::string GetKey() const
		{
			return key;
		}
		std::string GetValue() const
		{
			return value;
		}
		void SetValue( const std::string& val )
		{
			value = val;
		}
	private:
		std::string key;
		std::string value;
	};

	std::unordered_map<int, std::vector<Entry>, std::hash<int>> m_CustomKVCache;
};

#endif // _INCLUDE_SOURCEMOD_EXTENSION_PROPER_H_
