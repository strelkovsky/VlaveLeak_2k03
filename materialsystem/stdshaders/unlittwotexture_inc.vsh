
#include "macros.vsh"

sub UnlitTwoTexture
{
	local( $vertexcolor ) = shift;

	local( $worldPos, $projPos );

	;------------------------------------------------------------------------------
	; Vertex blending
	;------------------------------------------------------------------------------
	&AllocateRegister( \$worldPos );
	&SkinPosition( $g_numBones, $worldPos );

	;------------------------------------------------------------------------------
	; Transform the position from world to proj space
	;------------------------------------------------------------------------------

	&AllocateRegister( \$projPos );
	dp4 $projPos.x, $worldPos, $cViewProj0
	dp4 $projPos.y, $worldPos, $cViewProj1
	dp4 $projPos.z, $worldPos, $cViewProj2
	dp4 $projPos.w, $worldPos, $cViewProj3
	mov oPos, $projPos

	;------------------------------------------------------------------------------
	; Fog
	;------------------------------------------------------------------------------
	&CalcFog( $worldPos, $projPos );
	&FreeRegister( \$projPos );
	&FreeRegister( \$worldPos );

	;------------------------------------------------------------------------------
	; Texture coordinates (use world-space normal for envmap, tex transform for mask)
	;------------------------------------------------------------------------------
	dp4 oT0.x, $vTexCoord0, c90
	dp4 oT0.y, $vTexCoord0, c91

	dp4 oT1.x, $vTexCoord0, c92
	dp4 oT1.y, $vTexCoord0, c93

	if( $vertexcolor )
	{
		; Modulation color
		mul oD0, $vColor, $cModulationColor
	}
	else
	{
		; Modulation color
		mov oD0, $cModulationColor
	}
}