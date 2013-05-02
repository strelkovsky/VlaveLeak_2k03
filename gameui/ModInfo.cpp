//========= Copyright � 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================
#include <string.h>
#include <malloc.h>

#include "ModInfo.h"
#include <KeyValues.h>
#include <vgui_controls/Controls.h>
#include <FileSystem.h>
#include "EngineInterface.h"

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

//-----------------------------------------------------------------------------
// Purpose: singleton accessor
//-----------------------------------------------------------------------------
CModInfo &ModInfo()
{
	static CModInfo s_ModInfo;
	return s_ModInfo;
}

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CModInfo::CModInfo()
{
	m_pModData = new KeyValues("ModData");
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CModInfo::~CModInfo()
{
	FreeModInfo();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CModInfo::FreeModInfo()
{
	if (m_pModData)
	{
		m_pModData->deleteThis();
		m_pModData = NULL;
	}
}

//-----------------------------------------------------------------------------
// Purpose: data accessor
//-----------------------------------------------------------------------------
const char *CModInfo::GetStartMap()
{
	return m_pModData->GetString("startmap", "c0a0");
}

//-----------------------------------------------------------------------------
// Purpose: data accessor
//-----------------------------------------------------------------------------
const char *CModInfo::GetTrainMap()
{
//	return m_pModData->GetString("trainmap", "t0a0");
	return m_pModData->GetString("trainmap", "bump_test");
}

//-----------------------------------------------------------------------------
// Purpose: data accessor
//-----------------------------------------------------------------------------
bool CModInfo::IsMultiplayerOnly()
{
	return (stricmp(m_pModData->GetString("type", ""), "multiplayer_only") == 0);
}

bool CModInfo::IsSinglePlayerOnly()
{
	return (stricmp(m_pModData->GetString("type", ""), "singleplayer_only") == 0);
}

//-----------------------------------------------------------------------------
// Purpose: data accessor
//-----------------------------------------------------------------------------
bool CModInfo::NoModels()
{
	return (stricmp(m_pModData->GetString("nomodels", "0"), "1") == 0);
}

//-----------------------------------------------------------------------------
// Purpose: data accessor
//-----------------------------------------------------------------------------
bool CModInfo::NoHiModel()
{
	return (stricmp(m_pModData->GetString("nohimodel", "0"), "1") == 0);
}

//-----------------------------------------------------------------------------
// Purpose: data accessor
//-----------------------------------------------------------------------------
const char *CModInfo::GetGameDescription()
{
//	Msg( "%s\n", m_pModData->GetString("game", "") );
	return m_pModData->GetString("game", "");
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CModInfo::LoadCurrentGameInfo()
{
	// Load up liblist.gam for the current mod
	char const *filename = "liblist.gam";

	// walk through and add the key/value pairs to the keyvalues object
	FileHandle_t fh = vgui::filesystem()->Open( filename, "rb" );
	if ( fh != FILESYSTEM_INVALID_HANDLE )
	{
		int len = vgui::filesystem()->Size( fh );
		if ( len > 0 )
		{
			char *buf = (char *)_alloca( len + 1 );
			vgui::filesystem()->Read( buf, len, fh );
			buf[ len ] = 0;
			LoadGameInfoFromBuffer(buf, len);
		}

		vgui::filesystem()->Close( fh );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CModInfo::LoadGameInfoFromBuffer(const char *buffer, int bufferSize)
{
	char token[ 1024 ];
	bool done = false;
	char *p = (char *)buffer;
	while ( !done && p )
	{
		char key[ 256 ];
		char value[ 256 ];

		p = engine->COM_ParseFile( p, token );
		if ( strlen( token ) <= 0 )
			break;
		strcpy( key, token );

		p = engine->COM_ParseFile( p, token );
		strcpy( value, token );

		m_pModData->SetString(key, value);
	}
}

