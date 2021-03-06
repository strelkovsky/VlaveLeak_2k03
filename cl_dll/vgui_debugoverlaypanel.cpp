//======== (C) Copyright 1999, 2000 Valve, L.L.C. All rights reserved. ========
//
// The copyright to the contents herein is the property of Valve, L.L.C.
// The contents may be used and/or copied only with the written permission of
// Valve, L.L.C., or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Purpose: 
//
// $Workfile:     $
// $NoKeywords: $
//=============================================================================
#include "cbase.h"
#include "idebugoverlaypanel.h"
#include "overlaytext.h"
#include <vgui/IVgui.h>
#include "engine/IVDebugOverlay.h"
#include "vguimatsurface/imatsystemsurface.h"
#include <vgui_controls/Panel.h>
#include <vgui_controls/Controls.h>
#include <vgui/IScheme.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class CDebugOverlay : public vgui::Panel
{
	typedef vgui::Panel BaseClass;

public:
	CDebugOverlay( vgui::VPANEL parent );
	virtual ~CDebugOverlay( void );

	virtual void ApplySchemeSettings(vgui::IScheme *pScheme);
	virtual void Paint();
	virtual void OnTick( void );

	virtual bool ShouldDraw( void );

private:
	vgui::HFont			m_hFont;
};

//-----------------------------------------------------------------------------
// Purpose: Instances the overlay object
// Input  : *parent - 
//-----------------------------------------------------------------------------
CDebugOverlay::CDebugOverlay( vgui::VPANEL parent ) :
	BaseClass( NULL, "CDebugOverlay" )
{
	SetParent( parent );
	SetSize( ScreenWidth(), ScreenHeight() );
	SetPos( 0, 0 );
	SetVisible( false );
	SetCursor( null );

	m_hFont = 0;
	SetFgColor( Color( 0, 0, 0, 0 ) );
	SetPaintBackgroundEnabled( false );

	// set the scheme before any child control is created
	vgui::HScheme scheme = vgui::scheme()->LoadSchemeFromFile("resource/ClientScheme.res", "Client");
	SetScheme( scheme );
	
	vgui::ivgui()->AddTickSignal( GetVPanel(), 250 );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CDebugOverlay::~CDebugOverlay( void )
{
}

void CDebugOverlay::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	// Use a large font
//	m_hFont = pScheme->GetFont( "Default" );
	m_hFont = pScheme->GetFont( "DebugOverlay" );
	Assert( m_hFont );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CDebugOverlay::OnTick( void )
{
	SetVisible( ShouldDraw() );
}

bool CDebugOverlay::ShouldDraw( void )
{
	if ( debugoverlay->GetFirst() )
		return true;
	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Paints the 2D overlay items to the screen
//-----------------------------------------------------------------------------
void CDebugOverlay::Paint()
{
	OverlayText_t* pCurrText = debugoverlay->GetFirst();
	while (pCurrText) 
	{

		if ( pCurrText->text != NULL ) 
		{
			// --------------
			// Draw the text
			// --------------
			int r = pCurrText->r;
			int g = pCurrText->g;
			int b = pCurrText->b;
			int a = pCurrText->a;
			Vector screenPos;

			if (pCurrText->bUseOrigin)
			{
				if (!debugoverlay->ScreenPosition( pCurrText->origin, screenPos )) 
				{
					float xPos		= screenPos[0];
					float yPos		= screenPos[1]+ (pCurrText->lineOffset*13); // Line spacing;
					g_pMatSystemSurface->DrawColoredText( m_hFont, xPos, yPos, r, g, b, a, "%s", pCurrText->text ); // VXP: https://github.com/ValveSoftware/source-sdk-2013/pull/372
				}
			}
			else
			{
				if (!debugoverlay->ScreenPosition( pCurrText->flXPos,pCurrText->flYPos, screenPos )) 
				{	
					float xPos		= screenPos[0];
					float yPos		= screenPos[1]+ (pCurrText->lineOffset*13); // Line spacing;
					g_pMatSystemSurface->DrawColoredText( m_hFont, xPos, yPos, r, g, b, a, "%s", pCurrText->text ); // VXP: https://github.com/ValveSoftware/source-sdk-2013/pull/372
				}
			}
		}
		pCurrText = debugoverlay->GetNext( pCurrText );
	}

	debugoverlay->ClearDeadOverlays();
}

class CDebugOverlayPanel : public IDebugOverlayPanel
{
private:
	CDebugOverlay *debugOverlayPanel;
public:
	CDebugOverlayPanel( void )
	{
		debugOverlayPanel = NULL;
	}
	void Create( vgui::VPANEL parent )
	{
		debugOverlayPanel = new CDebugOverlay( parent );
	}
	void Destroy( void )
	{
		if ( debugOverlayPanel )
		{
			debugOverlayPanel->SetParent( (vgui::Panel *)NULL );
			delete debugOverlayPanel;
		}
	}
};

static CDebugOverlayPanel g_DebugOverlay;
IDebugOverlayPanel *debugoverlaypanel =  ( IDebugOverlayPanel * )&g_DebugOverlay;
