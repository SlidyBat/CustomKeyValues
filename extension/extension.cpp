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

#include "extension.h"
#include <algorithm>
#include <amtl/am-string.h>

/**
 * @file extension.cpp
 * @brief Implement extension code here.
 */

CustomKeyValues g_CustomKeyValues;		/**< Global singleton for extension's main interface */
ISDKHooks *g_pSDKHooks = nullptr;

SMEXT_LINK( &g_CustomKeyValues );

SH_DECL_MANUALHOOK2( MHook_KeyValue, 0, 0, 0, bool, const char*, const char* );

static cell_t Native_GetCustomKeyValue( IPluginContext* pContext, const cell_t* params );
static cell_t Native_SetCustomKeyValue( IPluginContext* pContext, const cell_t* params );

const sp_nativeinfo_t g_MyNatives[] = 
{
	{ "GetCustomKeyValue", Native_GetCustomKeyValue },
	{ "SetCustomKeyValue", Native_SetCustomKeyValue },
	{ NULL, NULL }
};

bool CustomKeyValues::SDK_OnLoad( char* error, size_t maxlength, bool late )
{
	sharesys->AddDependency( myself, "sdkhooks.ext", true, true );

	IGameConfig* cfg;

	char conf_error[255];
	if( !gameconfs->LoadGameConfigFile( "customkeyvalues.games", &cfg, conf_error, sizeof( conf_error ) ) )
	{
		if( conf_error[0] )
			snprintf( error, maxlength, "Could not read config file custom-keyvalues.games.txt: %s", conf_error );

		return false;
	}

	int offset = -1;
	cfg->GetOffset( "CBaseEntity::KeyValue", &offset );
	if( offset > -1 )
	{
		SH_MANUALHOOK_RECONFIGURE( MHook_KeyValue, offset, 0, 0 );
	}
	else
	{
		snprintf( error, maxlength, "Could not get offset for CBaseEntity::KeyValue" );

		gameconfs->CloseGameConfigFile( cfg );
		return false;
	}

	gameconfs->CloseGameConfigFile( cfg );

	sharesys->RegisterLibrary( myself, "custom-keyvalues" );

	return true;
}

bool CustomKeyValues::SDK_OnMetamodLoad( ISmmAPI* ismm, char* error, size_t maxlength, bool late )
{
	ismm->AddListener( this, this );

	return true;
}

void CustomKeyValues::OnLevelInit( const char* pMapName, const char* pMapEntities, const char* pOldLevel, const char* pLandmarkName, bool loadGame, bool background )
{
	m_CustomKVCache.clear();
}

void CustomKeyValues::SDK_OnUnload()
{
	if( g_pSDKHooks )
	{
		g_pSDKHooks->RemoveEntityListener( this );
	}
}

void CustomKeyValues::SDK_OnAllLoaded()
{
	SM_GET_LATE_IFACE( SDKHOOKS, g_pSDKHooks );
	if( g_pSDKHooks )
	{
		g_pSDKHooks->AddEntityListener( this );
	}
	
	sharesys->AddNatives( myself, g_MyNatives );
}

bool CustomKeyValues::QueryRunning( char *error, size_t maxlength )
{
	SM_CHECK_IFACE( SDKHOOKS, g_pSDKHooks );
	return true;
}

void CustomKeyValues::OnEntityCreated( CBaseEntity* pEntity, const char* classname )
{
	SH_ADD_MANUALHOOK( MHook_KeyValue, pEntity, SH_MEMBER( this, &CustomKeyValues::Hook_KeyValue ), true );
}

void CustomKeyValues::OnEntityDestroyed( CBaseEntity* pEntity )
{
	SH_REMOVE_MANUALHOOK( MHook_KeyValue, pEntity, SH_MEMBER( this, &CustomKeyValues::Hook_KeyValue ), true );
}

bool CustomKeyValues::Hook_KeyValue( const char* key, const char* value )
{
	bool ret = META_RESULT_ORIG_RET( bool );

	if( ret == true ) // default key, dont bother checking
	{
		RETURN_META_VALUE( MRES_IGNORED, ret );
	}

	CBaseEntity* pEntity = META_IFACEPTR( CBaseEntity );

	int ref = gamehelpers->EntityToReference( pEntity );
	auto i = m_CustomKVCache.find( ref );
	if( i == m_CustomKVCache.end() ) // new entity
	{
		m_CustomKVCache.emplace( ref, std::vector<Entry>{ Entry( key, value ) } );
	}
	else
	{
		i->second.emplace_back( key, value );
	}

	//g_pSM->LogMessage( myself, "CBaseEntity::KeyValue ran for entity w/ classname %s ('%s'='%s')", gamehelpers->GetEntityClassname( pEntity ), key, value );

	return false; // shouldnt matter what i return here
}


// native bool GetCustomKeyValue( int entity, const char[] key, char[] value, int maxlen );
cell_t Native_GetCustomKeyValue( IPluginContext* pContext, const cell_t* params )
{
	int ref = gamehelpers->IndexToReference( params[1] );
	if( ref == INVALID_EHANDLE_INDEX )
	{
		pContext->ThrowNativeError( "Invalid entity %i", params[1] );
	}

	auto i = g_CustomKeyValues.m_CustomKVCache.find( ref );

	if( i == g_CustomKeyValues.m_CustomKVCache.end() )
	{
		return 0; // entity passed in does not exist in cache
	}

	char* key;
	pContext->LocalToString( params[2], &key );
	std::vector<CustomKeyValues::Entry>& entityKeyValues = i->second;

	auto entry = std::find_if( entityKeyValues.begin(), entityKeyValues.end(), [key]( const CustomKeyValues::Entry& e )
	{
		return e.GetKey() == key;
	} );

	if( entry == entityKeyValues.end() ) // no custom key value exists for that entity
	{
		return 0;
	}
	
	char* value;
	pContext->LocalToString( params[3], &value );
	ke::SafeStrcpy( value, params[4], entry->GetValue().c_str() );

	return 1;
}

// native void SetCustomKeyValue( int entity, const char[] key, const char[] value );
cell_t Native_SetCustomKeyValue( IPluginContext* pContext, const cell_t* params )
{
	int ref = gamehelpers->IndexToReference( params[1] );
	if( ref == INVALID_EHANDLE_INDEX )
	{
		pContext->ThrowNativeError( "Invalid entity %i", params[1] );
	}

	char* key;
	pContext->LocalToString( params[2], &key );
	char* value;
	pContext->LocalToString( params[3], &value );

	auto i = g_CustomKeyValues.m_CustomKVCache.find( ref );
	if( i == g_CustomKeyValues.m_CustomKVCache.end() )
	{
		g_CustomKeyValues.m_CustomKVCache.emplace( ref, std::vector<CustomKeyValues::Entry>{ CustomKeyValues::Entry( key, value ) } );
		return 1;
	}

	std::vector<CustomKeyValues::Entry>& entityKeyValues = i->second;

	auto entry = std::find_if( entityKeyValues.begin(), entityKeyValues.end(), [key]( const CustomKeyValues::Entry& e )
	{
		return e.GetKey() == key;
	} );

	if( entry == entityKeyValues.end() )
	{
		entityKeyValues.emplace_back( key, value );
	}
	else
	{
		entry->SetValue( value );
	}

	return 1;
}