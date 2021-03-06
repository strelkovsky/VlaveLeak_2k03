//========= Copyright � 1996-2001, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#pragma warning(disable:4996) //I love old code style :)


#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <time.h>
#include "studio.h"
#include "optimize.h"
#include "vtfconvert.h"
#include "mathlib.h"

imglib_t image;

#if defined( _WIN32 ) || defined( WIN32 )
#define PATHSEPARATOR(c) ((c) == '\\' || (c) == '/')
#else	//_WIN32
#define PATHSEPARATOR(c) ((c) == '/')
#endif	//_WIN32

char g_modelsrcpath[MAX_PATH];

int RusPrintf( char *pRusBuff )
{
	char pValidBuff[ 2048 ];
	memset( pValidBuff, 0, sizeof(pValidBuff) );

	CharToOem( pRusBuff, pValidBuff );
	printf( "%s", pValidBuff );

	return 1;
}

void LogPrintf( char *fmt, ... )
{
	char string[10240];
	memset( string, 0, sizeof(string) );

	va_list marker;
	va_start( marker, fmt );
	vsnprintf( string, sizeof(string), fmt, marker );
	va_end( marker );

	RusPrintf( string );

	FILE *fp = fopen( "proxy.log", "ab" );
	fprintf( fp, "%s", string );
	fclose( fp );
}


char const* Sys_FindArg( char const *pArg, char const *pDefault )
{
	for( int i=0; i < __argc; i++ )
	{
		if( stricmp( __argv[i], pArg ) == 0 )
			return (i+1) < __argc ? __argv[i+1] : "";
	}

	return pDefault;
}


int Sys_FindArgInt( char const *pArg, int defaultVal )
{
	char const *pVal = Sys_FindArg( pArg, NULL );
	if( pVal )
		return atoi( pVal );
	else
		return defaultVal;
}

void COM_FixSlashes( char *pname )
{
	while ( *pname ) {
		if ( *pname == '\\' )
			*pname = '/';
		pname++;
	}
}

void COM_WinFixSlashes( char *pname )
{
	while ( *pname ) {
		if ( *pname == '/' )
			*pname = '\\';
		pname++;
	}
}

void    StripFilename (char *path)
{
	int             length;

	length = strlen(path)-1;
	while (length > 0 && !PATHSEPARATOR(path[length]))
		length--;
	path[length] = 0;
}


void    StripExtension (char *path)
{
	int             length;

	COM_FixSlashes( path );

	length = strlen(path)-1;
	while (length > 0 && path[length] != '.')
	{
		length--;
		if (path[length] == '/')
			return;		// no extension
	}
	if (length)
		path[length] = 0;
}

void AppendSlash( char *pStr, int strSize )
{
	int len;

	len = strlen( pStr );

	if ( !PATHSEPARATOR(pStr[len-1]) )
	{
		if ( len+1 >= strSize )
		{
			LogPrintf( "AppendSlash( %s ): ran out of buffer space", pStr );
		}
		
		pStr[len] = '/';
		len++;
		pStr[len] = 0;
	}
}

/*
====================
Extract file parts
====================
*/
// FIXME: should include the slash, otherwise
// backing to an empty path will be wrong when appending a slash
void ExtractFilePath (char *path, char *dest)
{
	char    *src;

	src = path + strlen(path) - 1;

//
// back up until a \ or the start
//
	while (src != path && *(src-1) != '\\' && *(src-1) != '/')
		src--;

	memcpy (dest, path, src-path);
	dest[src-path] = 0;
}

void ExtractFileBase (const char *path, char *dest, int destSize)
{
	char *pDestStart = dest;

	if ( destSize <= 1 )
		return;

	const char *src = path + strlen(path) - 1;

//
// back up until a \ or the start
//
	while (src != path && !PATHSEPARATOR(*(src-1)))
		src--;

	while (*src && *src != '.')
	{
		*dest++ = *src++;
		if ( dest - pDestStart >= destSize-1 )
			break;
	}
	*dest = 0;
}

void ExtractFileExtension (const char *path, char *dest, int destSize)
{
	const char    *src;

	src = path + strlen(path) - 1;

//
// back up until a . or the start
//
	while (src != path && *(src-1) != '.' )
		src--;

	// check to see if the '.' is part of a pathname
	if (src == path || *src == '/' || *src == '\\' )
	{
		*dest = 0;	// no extension
		return;
	}

	strncpy( dest, src, destSize );
}


OptimizedModel::FileHeader_t *LoadVtxHdr(  char *pFilename )
{
	int len;

	char pVtxFileName[MAX_PATH];
	memset( pVtxFileName, 0, sizeof(pVtxFileName) );
	sprintf( pVtxFileName, "%s.dx7_2bone.vtx", pFilename );

	FILE *fp = fopen(  pVtxFileName, "rb" );

	if( !fp )
	{
		LogPrintf( "Can't open file: %s\r\n", pVtxFileName );
		return NULL;
	}

	fseek( fp, 0, SEEK_END );

	len = ftell( fp );
	rewind( fp );

	OptimizedModel::FileHeader_t *pVtxHdr = ( OptimizedModel::FileHeader_t * )malloc( len );
	fread( pVtxHdr, 1, len, fp );

	fclose(fp);

	return pVtxHdr;
}

//studiohdr_t *LoadSudioHdr( char *pFilename )
studiohdr_v36_t *LoadSudioHdr(char *pFilename)
{
	int len;

	char pMdlFileName[MAX_PATH];
	memset( pMdlFileName, 0, sizeof(pMdlFileName) );
	sprintf( pMdlFileName, "%s.mdl", pFilename );

	FILE *fp = fopen(  pMdlFileName, "rb" );

	if( !fp )
	{
		LogPrintf( "Can't open file: %s\r\n", pMdlFileName );
		return NULL;
	}

	fseek( fp, 0, SEEK_END );

	len = ftell( fp );
	rewind( fp );

//	studiohdr_t *pStudioHdr = ( studiohdr_t * )malloc( len );
	studiohdr_v36_t *pStudioHdr = (studiohdr_v36_t *)malloc(len);
	fread( pStudioHdr, 1, len, fp );

	fclose(fp);

	return pStudioHdr;
}

#define VectorExpand(v) (v).x, (v).y, (v).z

/*void MatrixAngles( const matrix3x4_t& matrix, float *angles )
{ 
//	Assert( s_bMathlibInitialized );
	float forward[3];
	float left[3];
	float up[3];

	//
	// Extract the basis vectors from the matrix. Since we only need the Z
	// component of the up vector, we don't get X and Y.
	//
	forward[0] = matrix[0][0];
	forward[1] = matrix[1][0];
	forward[2] = matrix[2][0];
	left[0] = matrix[0][1];
	left[1] = matrix[1][1];
	left[2] = matrix[2][1];
	up[2] = matrix[2][2];

	float xyDist = sqrtf( forward[0] * forward[0] + forward[1] * forward[1] );
	
	// enough here to get angles?
	if ( xyDist > 0.001f )
	{
		// (yaw)	y = ATAN( forward.y, forward.x );		-- in our space, forward is the X axis
		angles[1] = RAD2DEG( atan2f( forward[1], forward[0] ) );

		// The engine does pitch inverted from this, but we always end up negating it in the DLL
		// UNDONE: Fix the engine to make it consistent
		// (pitch)	x = ATAN( -forward.z, sqrt(forward.x*forward.x+forward.y*forward.y) );
		angles[0] = RAD2DEG( atan2f( -forward[2], xyDist ) );

		// (roll)	z = ATAN( left.z, up.z );
		angles[2] = RAD2DEG( atan2f( left[2], up[2] ) );
	}
	else	// forward is mostly Z, gimbal lock-
	{
		// (yaw)	y = ATAN( -left.x, left.y );			-- forward is mostly z, so use right for yaw
		angles[1] = RAD2DEG( atan2f( -left[0], left[1] ) );

		// The engine does pitch inverted from this, but we always end up negating it in the DLL
		// UNDONE: Fix the engine to make it consistent
		// (pitch)	x = ATAN( -forward.z, sqrt(forward.x*forward.x+forward.y*forward.y) );
		angles[0] = RAD2DEG( atan2f( -forward[2], xyDist ) );

		// Assume no roll in this case as one degree of freedom has been lost (i.e. yaw == roll)
		angles[2] = 0;
	}
}*/

const char *unlookupControl( int val )
{
	if( val == STUDIO_X )
	{
		return "X";
	}
	else if( val == STUDIO_Y )
	{
		return "Y";
	}
	else if( val == STUDIO_Z )
	{
		return "Z";
	}
	if( val == STUDIO_XR )
	{
		return "XR";
	}
	else if( val == STUDIO_YR )
	{
		return "YR";
	}
	else if( val == STUDIO_ZR )
	{
		return "ZR";
	}
	if( val == STUDIO_LX )
	{
		return "LX";
	}
	else if( val == STUDIO_LY )
	{
		return "LY";
	}
	else if( val == STUDIO_LZ )
	{
		return "LZ";
	}
	if( val == STUDIO_LXR )
	{
		return "LXR";
	}
	else if( val == STUDIO_LYR )
	{
		return "LYR";
	}
	else if( val == STUDIO_LZR )
	{
		return "LZR";
	}

	return NULL;
}

//bool GenerateQCFile2836( studiohdr_t *pStudioHdr )
bool GenerateQCFile2836(studiohdr_v36_t *pStudioHdr)
{
	char pFullNameExt[MAX_PATH];
	strncpy( pFullNameExt, pStudioHdr->name, sizeof( pFullNameExt ) );

	char pClearName[MAX_PATH];
	ExtractFileBase( pFullNameExt, pClearName, sizeof(pClearName) );

	memset( g_modelsrcpath, 0, sizeof(g_modelsrcpath) );
	strncpy( g_modelsrcpath, pClearName, sizeof( g_modelsrcpath ) );
	CreateDirectory( g_modelsrcpath, NULL );

	char pQCFileName[MAX_PATH];
	sprintf( pQCFileName, "%s\\%s.qc", g_modelsrcpath, pClearName );

	LogPrintf( "Generating: %s\r\n", pQCFileName );

	FILE *fp = fopen( pQCFileName, "wb" );

	if(!fp)
	{
		LogPrintf( "Can't creating QC file: %s\r\n", pQCFileName );
		return false;
	}

	fprintf( fp, "/*\r\n" );
	fprintf( fp, "==============================================================================\r\n" );
	fprintf( fp, "\r\n" );
	fprintf( fp, "QC script generated by Half-Life 2 Beta 28-37 decompiler\r\n" );
//	fprintf( fp, "2013 by Fire64\r\n" );
	fprintf(fp, "2013-2016 by Fire64 and VXP\r\n");
	fprintf( fp, "\r\n" );
	fprintf( fp, "Original internal name: %s\r\n", pStudioHdr->name );
	fprintf( fp, "\r\n" );
	fprintf( fp, "==============================================================================\r\n" );
	fprintf( fp, "*/\r\n" );
	fprintf( fp, "\r\n" );

//	fprintf( fp, "$modelname \"%s.mdl\"\r\n", pClearName );
	fprintf(fp, "$modelname \"%s\"\r\n", pStudioHdr->name);
//	fprintf( fp, "$cd \".\\\"\r\n" );
//	fprintf( fp, "$cdtexture \".\\\"\r\n" );
	fprintf( fp, "$cdmaterials \"%s\"\r\n", pStudioHdr->pCdtexture(0) );
	// VXP: Is this only one all the time?
//	for (int i = 0; i < pStudioHdr->numcdtextures; i++)
//	{
//		fprintf(fp, "$cdmaterials%i \"%s\"\r\n", i, pStudioHdr->pCdtexture(i));
//	}
	fprintf( fp, "$scale 1.0\r\n" );

	fprintf( fp, "\r\n" );

	fprintf( fp, "$bbox %f %f %f %f %f %f\r\n", pStudioHdr->hull_min.x, pStudioHdr->hull_min.y, pStudioHdr->hull_min.z, pStudioHdr->hull_max.x, pStudioHdr->hull_max.y, pStudioHdr->hull_max.z );
	fprintf( fp, "$cbox %f %f %f %f %f %f\r\n", pStudioHdr->view_bbmin.x, pStudioHdr->view_bbmin.y, pStudioHdr->view_bbmin.z, pStudioHdr->view_bbmax.x, pStudioHdr->view_bbmax.y, pStudioHdr->view_bbmax.z );
	fprintf( fp, "$eyeposition %f %f %f\r\n", pStudioHdr->eyeposition.x, pStudioHdr->eyeposition.y, pStudioHdr->eyeposition.z );

	fprintf( fp, "\r\n" );
	fprintf( fp, "\r\n" );

	fprintf( fp, "//reference mesh(es)\r\n" );

	for( int i = 0; i < pStudioHdr->numbodyparts; i++ )
	{
		mstudiobodyparts_t *pBodypart = pStudioHdr->pBodypart( i );

		if( pBodypart->nummodels == 1 )
		{
			mstudiomodel_t *pModel = pBodypart->pModel(0);

			if( !pModel->nummeshes )
			{
				fprintf( fp, "$bodygroup \"%s\"\r\n", pBodypart->pszName() );
				fprintf( fp, "{\r\n" );
				fprintf( fp, "blank\r\n" );
				fprintf( fp, "}\r\n" );
				fprintf( fp, "\r\n" );
			}
			else
			{
				char pModelName[MAX_PATH];
				strncpy( pModelName, pModel->name, sizeof( pModelName ) );
				StripExtension( pModelName );

				fprintf( fp, "$body \"%s\" \"%s_lod0\"\r\n", pBodypart->pszName(), pModelName );
				fprintf( fp, "\r\n" );
			}
		}
		else
		{
			fprintf( fp, "$bodygroup \"%s\"\r\n", pBodypart->pszName() );
			fprintf( fp, "{\r\n" );

			for( int j = 0; j < pBodypart->nummodels; j++ )
			{
				mstudiomodel_t *pModel = pBodypart->pModel(j);

				if( !pModel->nummeshes )
				{
					fprintf( fp, "blank\r\n" );
				}
				else
				{
					char pModelName[MAX_PATH];
					strncpy( pModelName, pModel->name, sizeof( pModelName ) );
					StripExtension( pModelName );

					fprintf( fp, "studio \"%s_lod0\"\r\n", pModelName );
				}
			}

			fprintf( fp, "}\r\n" );
			fprintf( fp, "\r\n" );
		}

	}

	fprintf( fp, "\r\n" );

	if( pStudioHdr->numbonecontrollers )
	{
		fprintf( fp, "// %d bone controller(s)\r\n", pStudioHdr->numbonecontrollers );

		for( int i = 0; i < pStudioHdr->numbonecontrollers; i++ )
		{
			mstudiobonecontroller_t *pBonecontroller = pStudioHdr->pBonecontroller(i);

			if( pBonecontroller->inputfield == 4 )
			{
				fprintf( fp, "$controller mouth \"%s\" %s %f %f\r\n", pStudioHdr->pBone( pBonecontroller->bone )->pszName(), unlookupControl(pBonecontroller->type), pBonecontroller->start, pBonecontroller->end );
			}
			else
			{
				fprintf( fp, "$controller %d \"%s\" %s %f %f\r\n", pBonecontroller->inputfield, pStudioHdr->pBone( pBonecontroller->bone )->pszName(), unlookupControl( pBonecontroller->type), pBonecontroller->start, pBonecontroller->end );
			}
		}

		fprintf( fp, "\r\n" );
	}

	if( pStudioHdr->numattachments )
	{
		fprintf( fp, "// %d attachment(s)\r\n", pStudioHdr->numattachments );

		for( int i = 0; i < pStudioHdr->numattachments; i++ )
		{
			mstudioattachment_t *pAttachment = pStudioHdr->pAttachment(i);

			float angles[3];
			MatrixAngles( pAttachment->local, angles );

			Vector vRotation = Vector( angles[0], angles[1], angles[2] );
			Vector vTranslation = Vector( pAttachment->local[0][3], pAttachment->local[1][3], pAttachment->local[2][3] );

			fprintf( fp, "$attachment \"%s\" \"%s\" %.2f %.2f %.2f rotate %.0f %.0f %.0f\r\n", pAttachment->pszName(), pStudioHdr->pBone( pAttachment->bone )->pszName(), VectorExpand( vTranslation ), VectorExpand( vRotation ) );
		}

		fprintf( fp, "\r\n" );
	}


	if( pStudioHdr->version >= 35 )
	{
		int counthitboxes = 0;

		for( int i = 0; i < pStudioHdr->numhitboxsets; i++ )
		{
			mstudiohitboxset_t	*pHitboxSet = pStudioHdr->pHitboxSet(i);

			counthitboxes+= pHitboxSet->numhitboxes;
		}

		if(counthitboxes)
		{
			fprintf( fp, "// %d hit box(es)\r\n", counthitboxes );
		}


		for( int i = 0; i < pStudioHdr->numhitboxsets; i++ )
		{
			mstudiohitboxset_t	*pHitboxSet = pStudioHdr->pHitboxSet(i);

			for( int j = 0; j < pHitboxSet->numhitboxes; j++ )
			{
				mstudiobbox_t *pHitbox = pHitboxSet->pHitbox(j);

				fprintf( fp, "$hbox %d \"%s\" %f %f %f %f %f %f\r\n", pHitbox->group, pStudioHdr->pBone( pHitbox->bone )->pszName(), pHitbox->bbmin.x, pHitbox->bbmin.y, pHitbox->bbmin.z, pHitbox->bbmax.x, pHitbox->bbmax.y, pHitbox->bbmax.z );
			}
		}
	}

	fprintf( fp, "\r\n" );
	fprintf( fp, "// %d animation sequence(s)\r\n", pStudioHdr->numseq );

	for( int i = 0; i < pStudioHdr->numseq; i++ )
	{
	//	mstudioseqdesc_t *pSeqdesc = pStudioHdr->pSeqdesc(i);
		mstudioseqdesc_v36_t *pSeqdesc = pStudioHdr->pSeqdesc(i);
		mstudioanimdesc_t *pAnimdesc = pStudioHdr->pAnimdesc( pSeqdesc->anim[0][0] );

		fprintf( fp, "$sequence \"%s\" \"%s\" fps %.0f ", pSeqdesc->pszLabel(), pSeqdesc->pszLabel(), pAnimdesc->fps );

		bool looping = pSeqdesc->flags & STUDIO_LOOPING ? true : false;

		if( looping )
		{
			fprintf( fp, "loop " );
		}

		fprintf( fp, "\r\n" );
	}

	fprintf( fp, "\r\n" );
	fprintf( fp, "// End of QC script.\r\n" );
	fprintf( fp, "\r\n" );

	fclose(fp);

	return true;
}

//bool GenerateReferenceSMD3136( studiohdr_t *pStudioHdr, OptimizedModel::FileHeader_t *pVtxHdr )
bool GenerateReferenceSMD3136(studiohdr_v36_t *pStudioHdr, OptimizedModel::FileHeader_t *pVtxHdr)
{
	for( int bodyPartID = 0; bodyPartID < pVtxHdr->numBodyParts; bodyPartID++ )
	{
		OptimizedModel::BodyPartHeader_t *bodyPart = pVtxHdr->pBodyPart( bodyPartID );
		mstudiobodyparts_t *pStudioBodyPart = pStudioHdr->pBodypart( bodyPartID );

		for( int lodID = 0; lodID < pVtxHdr->numLODs; lodID++ )
		{
			for( int modelID = 0; modelID < bodyPart->numModels; modelID++ )
			{
				OptimizedModel::ModelHeader_t *model = bodyPart->pModel( modelID );
				mstudiomodel_t *pStudioModel = pStudioBodyPart->pModel( modelID );
				OptimizedModel::ModelLODHeader_t *pLOD = model->pLOD( lodID );

				FILE *fp = NULL;

				if( pStudioModel->nummeshes )
				{
					char pModelName[MAX_PATH];
					strncpy( pModelName, pStudioModel->name, sizeof( pModelName ) );
					StripExtension( pModelName );

					char pEndRefName[MAX_PATH];
					sprintf( pEndRefName, "%s\\%s_lod%d.smd", g_modelsrcpath, pModelName, lodID );

					fp = fopen( pEndRefName, "wb" );

					fprintf( fp, "version 1\r\n" );
					fprintf( fp, "nodes\r\n" );

					for( int i = 0; i < pStudioHdr->numbones; i++ )
					{
						mstudiobone_t *pBone = pStudioHdr->pBone(i);

						fprintf( fp, "  %d \"%s\" %d\r\n", i, pBone->pszName(), pBone->parent );
					}

					fprintf( fp, "end\r\n" );
					fprintf( fp, "skeleton\r\n" );
					fprintf( fp, "time 0\r\n" );

					for( int i = 0; i < pStudioHdr->numbones; i++ )
					{
						mstudiobone_t *pBone = pStudioHdr->pBone(i);
						fprintf( fp, "  %d %f %f %f %f %f %f\r\n", i, pBone->value[0], pBone->value[1], pBone->value[2], pBone->value[3], pBone->value[4], pBone->value[5] );
					}

					fprintf( fp, "end\r\n" );
					fprintf( fp, "triangles\r\n" );

					for( int meshID = 0; meshID < pLOD->numMeshes; meshID++ )
					{
						OptimizedModel::MeshHeader_t *mesh = pLOD->pMesh( meshID );
						mstudiomesh_t *pStudioMesh = pStudioModel->pMesh( meshID );

						for( int stripGroupID = 0; stripGroupID < mesh->numStripGroups; stripGroupID++ )
						{
							OptimizedModel::StripGroupHeader_t *pStripGroup = mesh->pStripGroup( stripGroupID );

							for (int i = 0; i < pStripGroup->numIndices; i += 3)
							{
								unsigned short nIndex1 = *pStripGroup->pIndex( i + 2 );
								unsigned short nIndex2 = *pStripGroup->pIndex( i + 1 );
								unsigned short nIndex3 = *pStripGroup->pIndex( i );

								OptimizedModel::Vertex_t* pVert1 = pStripGroup->pVertex( nIndex1 );
								OptimizedModel::Vertex_t* pVert2 = pStripGroup->pVertex( nIndex2 );
								OptimizedModel::Vertex_t* pVert3 = pStripGroup->pVertex( nIndex3 );

								fprintf( fp, "%s.bmp\r\n", pStudioHdr->pTexture( pStudioMesh->material )->pszName() );
								fprintf( fp, "%d %f %f %f %f %f %f %f %f\r\n", pStudioMesh->BoneWeights( pVert1->origMeshVertID )->bone[0], pStudioMesh->Position( pVert1->origMeshVertID )->x, pStudioMesh->Position( pVert1->origMeshVertID )->y, pStudioMesh->Position( pVert1->origMeshVertID )->z, pStudioMesh->Normal( pVert1->origMeshVertID )->x, pStudioMesh->Normal( pVert1->origMeshVertID )->y, pStudioMesh->Normal( pVert1->origMeshVertID )->z, pStudioMesh->Texcoord( pVert1->origMeshVertID )->x, pStudioMesh->Texcoord( pVert1->origMeshVertID )->y * -1 );
								fprintf( fp, "%d %f %f %f %f %f %f %f %f\r\n", pStudioMesh->BoneWeights( pVert2->origMeshVertID )->bone[0], pStudioMesh->Position( pVert2->origMeshVertID )->x, pStudioMesh->Position( pVert2->origMeshVertID )->y, pStudioMesh->Position( pVert2->origMeshVertID )->z, pStudioMesh->Normal( pVert2->origMeshVertID )->x, pStudioMesh->Normal( pVert2->origMeshVertID )->y, pStudioMesh->Normal( pVert2->origMeshVertID )->z, pStudioMesh->Texcoord( pVert2->origMeshVertID )->x, pStudioMesh->Texcoord( pVert2->origMeshVertID )->y * -1 );
								fprintf( fp, "%d %f %f %f %f %f %f %f %f\r\n", pStudioMesh->BoneWeights( pVert3->origMeshVertID )->bone[0], pStudioMesh->Position( pVert3->origMeshVertID )->x, pStudioMesh->Position( pVert3->origMeshVertID )->y, pStudioMesh->Position( pVert3->origMeshVertID )->z, pStudioMesh->Normal( pVert3->origMeshVertID )->x, pStudioMesh->Normal( pVert3->origMeshVertID )->y, pStudioMesh->Normal( pVert3->origMeshVertID )->z, pStudioMesh->Texcoord( pVert3->origMeshVertID )->x, pStudioMesh->Texcoord( pVert3->origMeshVertID )->y * -1 );
							}
						}
					}

					fprintf( fp, "end\r\n" );
					fclose(fp);
				}
			}
		}
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: return a sub frame rotation for a single bone
//-----------------------------------------------------------------------------
//void CalcBoneQuaternion3136( const studiohdr_t *pStudioHdr, int frame, float s, 
void CalcBoneQuaternion3136(const studiohdr_v36_t *pStudioHdr, int frame, float s,
						const mstudiobone_t *pbone, const mstudioanim_t *panim, Vector &pos )
{

	int					j, k;
	Quaternion			q1, q2;
	RadianEuler			angle1(0,0,0), angle2(0,0,0);
	mstudioanimvalue_t	*panimvalue;

	if (!(panim->flags & STUDIO_ROT_ANIMATED))
	{
		pos.Init( panim->u.pose.q[0], panim->u.pose.q[1], panim->u.pose.q[2] );
		return;
	}

	for (j = 0; j < 3; j++)
	{
		if (panim->u.offset[j+3] == 0)
		{
			angle2[j] = angle1[j] = pbone->value[j+3]; // default;
		}
		else
		{
			panimvalue = panim->pAnimvalue( j+3 );
			k = frame;
			// DEBUG
			if (panimvalue->num.total < panimvalue->num.valid)
				k = 0;
			while (panimvalue->num.total <= k)
			{
				k -= panimvalue->num.total;
				panimvalue += panimvalue->num.valid + 1;
				// DEBUG
				if (panimvalue->num.total < panimvalue->num.valid)
					k = 0;
			}
			// Bah, missing blend!
			if (panimvalue->num.valid > k)
			{
				angle1[j] = panimvalue[k+1].value;

				if (panimvalue->num.valid > k + 1)
				{
					angle2[j] = panimvalue[k+2].value;
				}
				else
				{
					if (panimvalue->num.total > k + 1)
						angle2[j] = angle1[j];
					else
						angle2[j] = panimvalue[panimvalue->num.valid+2].value;
				}
			}
			else
			{
				angle1[j] = panimvalue[panimvalue->num.valid].value;
				if (panimvalue->num.total > k + 1)
				{
					angle2[j] = angle1[j];
				}
				else
				{
					angle2[j] = panimvalue[panimvalue->num.valid + 2].value;
				}
			}
			angle1[j] = pbone->value[j+3] + angle1[j] * pbone->scale[j+3];
			angle2[j] = pbone->value[j+3] + angle2[j] * pbone->scale[j+3];
		}
	}

	pos[0] = angle1[0];
	pos[1] = angle1[1];
	pos[2] = angle1[2];
}

//-----------------------------------------------------------------------------
// Purpose: return a sub frame position for a single bone
//-----------------------------------------------------------------------------
//void CalcBonePosition3136( const studiohdr_t *pStudioHdr, int frame, float s, 
void CalcBonePosition3136(const studiohdr_v36_t *pStudioHdr, int frame, float s,
	const mstudiobone_t *pbone, const mstudioanim_t *panim, Vector &pos	)
{
	int					j, k;
	mstudioanimvalue_t	*panimvalue;

	if (!(panim->flags & STUDIO_POS_ANIMATED))
	{
		pos.Init( panim->u.pose.pos[0], panim->u.pose.pos[1], panim->u.pose.pos[2] );
		return;
	}

	for (j = 0; j < 3; j++)
	{
		pos[j] = pbone->value[j]; // default;
		if (panim->u.offset[j] != 0)
		{
			panimvalue = panim->pAnimvalue( j );
			/*
			if (i == 0 && j == 0)
				Con_DPrintf("%d  %d:%d  %f\n", frame, panimvalue->num.valid, panimvalue->num.total, s );
			*/
			
			k = frame;
			// DEBUG
			if (panimvalue->num.total < panimvalue->num.valid)
				k = 0;
			// find span of values that includes the frame we want
			while (panimvalue->num.total <= k)
			{
				k -= panimvalue->num.total;
				panimvalue += panimvalue->num.valid + 1;
  				// DEBUG
				if (panimvalue->num.total < panimvalue->num.valid)
					k = 0;
			}
			// if we're inside the span
			if (panimvalue->num.valid > k)
			{
				// and there's more data in the span
				if (panimvalue->num.valid > k + 1)
				{
					pos[j] += (panimvalue[k+1].value * (1.0 - s) + s * panimvalue[k+2].value) * pbone->scale[j];
				}
				else
				{
					pos[j] += panimvalue[k+1].value * pbone->scale[j];
				}
			}
			else
			{
				// are we at the end of the repeating values section and there's another section with data?
				if (panimvalue->num.total <= k + 1)
				{
					pos[j] += (panimvalue[panimvalue->num.valid].value * (1.0 - s) + s * panimvalue[panimvalue->num.valid + 2].value) * pbone->scale[j];
				}
				else
				{
					pos[j] += panimvalue[panimvalue->num.valid].value * pbone->scale[j];
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
//static void CalcRotations3136( const studiohdr_t *pStudioHdr,	Vector *pos, Vector *pos2, 
static void CalcRotations3136(const studiohdr_v36_t *pStudioHdr, Vector *pos, Vector *pos2,
	//const mstudioseqdesc_t *pseqdesc,
	const mstudioseqdesc_v36_t *pseqdesc,
	const mstudioanimdesc_t *panimdesc, int setframe )
{
	int					i;
	mstudiobone_t *pbone = pStudioHdr->pBone( 0 );

	int					iFrame;
	float				s;

	float fFrame = setframe; // cycle * (panimdesc->numframes - 1);

	iFrame = (int)fFrame;
	s = (fFrame - iFrame);

	mstudioanim_t *panim = panimdesc->pAnim( 0 );

	for (i = 0; i < pStudioHdr->numbones; i++, pbone++, panim++) 
	{
		if (pseqdesc->weight(i) > 0 /*&& (pbone->flags & boneMask)*/)
		{
			CalcBoneQuaternion3136( pStudioHdr, iFrame, s, pbone, panim, pos2[i] );
			CalcBonePosition3136  ( pStudioHdr, iFrame, s, pbone, panim, pos[i] );
		}
	}
}

//bool GenerateAnimationSMD2836( studiohdr_t *pStudioHdr )
bool GenerateAnimationSMD2836(studiohdr_v36_t *pStudioHdr)
{
	for( int i = 0; i < pStudioHdr->numseq; i++ )
	{
	//	mstudioseqdesc_t *pSeqdesc = pStudioHdr->pSeqdesc(i);
		mstudioseqdesc_v36_t *pSeqdesc = pStudioHdr->pSeqdesc(i);
		mstudioanimdesc_t *pAnimdesc = pStudioHdr->pAnimdesc( pSeqdesc->anim[0][0] );

		char pAnimName[MAX_PATH];
		sprintf( pAnimName, "%s//%s.smd", g_modelsrcpath, pSeqdesc->pszLabel() );

		FILE *fp = fopen( pAnimName, "wb" );

		if(!fp)
		{
			LogPrintf( "Can't write animation in %s\r\n", pAnimName );
			return false;
		}

		fprintf( fp, "version 1\r\n" );
		fprintf( fp, "nodes\r\n" );

		for( int i = 0; i < pStudioHdr->numbones; i++ )
		{
			mstudiobone_t *pBone = pStudioHdr->pBone(i);

			fprintf( fp, "  %d \"%s\" %d\r\n", i, pBone->pszName(), pBone->parent );
		}

		fprintf( fp, "end\r\n" );
		fprintf( fp, "skeleton\r\n" );

		static Vector		pos[MAXSTUDIOBONES];
		static Vector		rot[MAXSTUDIOBONES];

		for( int j = 0; j < pAnimdesc->numframes; j++ )
		{
			fprintf( fp, "time %d\r\n", j );

			CalcRotations3136( pStudioHdr,	pos, rot, pSeqdesc, pAnimdesc, j );

			for( int b = 0; b < pStudioHdr->numbones; b++ )
			{
				fprintf(fp, "  %d   %f %f %f %f %f %f\r\n", b, pos[b].x, pos[b].y, pos[b].z, rot[b].x, rot[b].y, rot[b].z);
			//	RadianEuler q(rot[b].x, rot[b].y, rot[b].z);
			//	fprintf( fp, "  %d   %f %f %f %f %f %f\r\n", b, pos[b].x, pos[b].y, pos[b].z, q.x, q.y, q.z );
			}
		}

		fprintf( fp, "end\r\n" );

		fclose(fp);
	}

	return true;
}


//static mstudioanimdesc_t *GetAnimDesc( const studiohdr37_t *pStudioHdr, const int anim )
static mstudioanimdesc_t *GetAnimDesc(const studiohdr_t *pStudioHdr, const int anim)
{
#ifdef aliengrey_v37
	int group = (pStudioHdr->pAnimGroup( anim ))->group;
	int value = (pStudioHdr->pAnimGroup( anim ))->value;

	if (group == 0 || pStudioHdr->numseqgroups == 1)
	{
		return pStudioHdr->pAnimdesc( value );
	}
	else
	{
		mstudioseqgroup_t *pgroup = pStudioHdr->pSeqgroup( 1 );
		studioseqhdr37_t *phdr;
		int nAnimCount = 0;
		for( int i = 1; i < pStudioHdr->numseqgroups; i++ )
		{
			phdr = (studioseqhdr37_t *) ((BYTE *)(pStudioHdr) + (BYTE)(pgroup->cache));
			if( value >= nAnimCount && value < (nAnimCount += phdr->numanim) )
				return phdr->pAnimdesc( value );
			pgroup++;
		}
		return pStudioHdr->pAnimdesc( value - nAnimCount );
	}
#else
	int group = (pStudioHdr->pAnimGroup(anim))->group;
//	int value = (pStudioHdr->pAnimGroup(anim))->value; 
	int value = (pStudioHdr->pAnimGroup(anim))->index;

	if(group == 0)
		return pStudioHdr->pAnimdesc(value);

//	studioseqhdr37_t *phdr = (studioseqhdr37_t *)((pStudioHdr->pSeqgroup(group))->cache);
	studiosharehdr_t *phdr = (studiosharehdr_t *)((pStudioHdr->pSeqgroup(group))->cache);

	return phdr->pAnimdesc(value);
#endif
}

//static void v37Convert( const mstudioseqdesc37_t *pseqdesc, unsigned short anim[32][32] )
static void v37Convert(const mstudioseqdesc_t *pseqdesc, unsigned short anim[32][32])
{
	int itmp = pseqdesc->groupsize[0]; 
	int jtmp = pseqdesc->groupsize[1];

	unsigned short *tmp = (unsigned short *)((byte *)(pseqdesc) + pseqdesc->blendindex);
	for (int i=0;i<itmp;i++)
	{
		for (int j=0;j<jtmp;j++)
		{
			anim[i][j]=*tmp;
			tmp++;
		}
	}
}

//bool GenerateQCFile37( studiohdr37_t *pStudioHdr )
bool GenerateQCFile37(studiohdr_t *pStudioHdr)
{
	char pFullNameExt[MAX_PATH];
	strncpy( pFullNameExt, pStudioHdr->name, sizeof( pFullNameExt ) );

	char pClearName[MAX_PATH];
	ExtractFileBase( pFullNameExt, pClearName, sizeof(pClearName) );

	memset( g_modelsrcpath, 0, sizeof(g_modelsrcpath) );
	strncpy( g_modelsrcpath, pClearName, sizeof( g_modelsrcpath ) );
	CreateDirectory( g_modelsrcpath, NULL );

	char pQCFileName[MAX_PATH];
	sprintf( pQCFileName, "%s\\%s.qc", g_modelsrcpath, pClearName );

	LogPrintf( "Generating: %s\r\n", pQCFileName );

	FILE *fp = fopen( pQCFileName, "wb" );

	if(!fp)
	{
		LogPrintf( "Can't creating QC file: %s\r\n", pQCFileName );
		return false;
	}

	fprintf( fp, "/*\r\n" );
	fprintf( fp, "==============================================================================\r\n" );
	fprintf( fp, "\r\n" );
	fprintf( fp, "QC script generated by Half-Life 2 Beta 28-37 decompiler\r\n" );
	fprintf( fp, "2013 by Fire64\r\n" );
	fprintf( fp, "\r\n" );
	fprintf( fp, "Original internal name: %s\r\n", pStudioHdr->name );
	fprintf( fp, "\r\n" );
	fprintf( fp, "==============================================================================\r\n" );
	fprintf( fp, "*/\r\n" );
	fprintf( fp, "\r\n" );

	fprintf( fp, "$modelname \"%s.mdl\"\r\n", pClearName );
	fprintf( fp, "$cd \".\\\"\r\n" );
	fprintf( fp, "$cdtexture \".\\\"\r\n" );
	fprintf( fp, "$scale 1.0\r\n" );

	fprintf( fp, "\r\n" );

	fprintf( fp, "$bbox %f %f %f %f %f %f\r\n", pStudioHdr->hull_min.x, pStudioHdr->hull_min.y, pStudioHdr->hull_min.z, pStudioHdr->hull_max.x, pStudioHdr->hull_max.y, pStudioHdr->hull_max.z );
	fprintf( fp, "$cbox %f %f %f %f %f %f\r\n", pStudioHdr->view_bbmin.x, pStudioHdr->view_bbmin.y, pStudioHdr->view_bbmin.z, pStudioHdr->view_bbmax.x, pStudioHdr->view_bbmax.y, pStudioHdr->view_bbmax.z );
	fprintf( fp, "$eyeposition %f %f %f\r\n", pStudioHdr->eyeposition.x, pStudioHdr->eyeposition.y, pStudioHdr->eyeposition.z );

	fprintf( fp, "\r\n" );
	fprintf( fp, "\r\n" );

	fprintf( fp, "//reference mesh(es)\r\n" );

	for( int i = 0; i < pStudioHdr->numbodyparts; i++ )
	{
		mstudiobodyparts_t *pBodypart = pStudioHdr->pBodypart( i );

		if( pBodypart->nummodels == 1 )
		{
			mstudiomodel_t *pModel = pBodypart->pModel(0);

			if( !pModel->nummeshes )
			{
				fprintf( fp, "$bodygroup \"%s\"\r\n", pBodypart->pszName() );
				fprintf( fp, "{\r\n" );
				fprintf( fp, "blank\r\n" );
				fprintf( fp, "}\r\n" );
				fprintf( fp, "\r\n" );
			}
			else
			{
				char pModelName[MAX_PATH];
				strncpy( pModelName, pModel->name, sizeof( pModelName ) );
				StripExtension( pModelName );

				fprintf( fp, "$body \"%s\" \"%s_lod0\"\r\n", pBodypart->pszName(), pModelName );
				fprintf( fp, "\r\n" );
			}
		}
		else
		{
			fprintf( fp, "$bodygroup \"%s\"\r\n", pBodypart->pszName() );
			fprintf( fp, "{\r\n" );

			for( int j = 0; j < pBodypart->nummodels; j++ )
			{
				mstudiomodel_t *pModel = pBodypart->pModel(j);

				if( !pModel->nummeshes )
				{
					fprintf( fp, "blank\r\n" );
				}
				else
				{
					char pModelName[MAX_PATH];
					strncpy( pModelName, pModel->name, sizeof( pModelName ) );
					StripExtension( pModelName );

					fprintf( fp, "studio \"%s_lod0\"\r\n", pModelName );
				}
			}

			fprintf( fp, "}\r\n" );
			fprintf( fp, "\r\n" );
		}

	}

	fprintf( fp, "\r\n" );

	if( pStudioHdr->numbonecontrollers )
	{
		fprintf( fp, "// %d bone controller(s)\r\n", pStudioHdr->numbonecontrollers );

		for( int i = 0; i < pStudioHdr->numbonecontrollers; i++ )
		{
			mstudiobonecontroller_t *pBonecontroller = pStudioHdr->pBonecontroller(i);

			if( pBonecontroller->inputfield == 4 )
			{
				fprintf( fp, "$controller mouth \"%s\" %s %f %f\r\n", pStudioHdr->pBone( pBonecontroller->bone )->pszName(), unlookupControl(pBonecontroller->type), pBonecontroller->start, pBonecontroller->end );
			}
			else
			{
				fprintf( fp, "$controller %d \"%s\" %s %f %f\r\n", pBonecontroller->inputfield, pStudioHdr->pBone( pBonecontroller->bone )->pszName(), unlookupControl( pBonecontroller->type), pBonecontroller->start, pBonecontroller->end );
			}
		}

		fprintf( fp, "\r\n" );
	}

	if( pStudioHdr->numattachments )
	{
		fprintf( fp, "// %d attachment(s)\r\n", pStudioHdr->numattachments );

		for( int i = 0; i < pStudioHdr->numattachments; i++ )
		{
			mstudioattachment_t *pAttachment = pStudioHdr->pAttachment(i);

			float angles[3];
			MatrixAngles( pAttachment->local, angles );

			Vector vRotation = Vector( angles[0], angles[1], angles[2] );
			Vector vTranslation = Vector( pAttachment->local[0][3], pAttachment->local[1][3], pAttachment->local[2][3] );

			fprintf( fp, "$attachment \"%s\" \"%s\" %.2f %.2f %.2f rotate %.0f %.0f %.0f\r\n", pAttachment->pszName(), pStudioHdr->pBone( pAttachment->bone )->pszName(), VectorExpand( vTranslation ), VectorExpand( vRotation ) );
		}

		fprintf( fp, "\r\n" );
	}


	if( pStudioHdr->version >= 35 )
	{
		int counthitboxes = 0;

		for( int i = 0; i < pStudioHdr->numhitboxsets; i++ )
		{
			mstudiohitboxset_t	*pHitboxSet = pStudioHdr->pHitboxSet(i);

			counthitboxes+= pHitboxSet->numhitboxes;
		}

		if(counthitboxes)
		{
			fprintf( fp, "// %d hit box(es)\r\n", counthitboxes );
		}


		for( int i = 0; i < pStudioHdr->numhitboxsets; i++ )
		{
			mstudiohitboxset_t	*pHitboxSet = pStudioHdr->pHitboxSet(i);

			for( int j = 0; j < pHitboxSet->numhitboxes; j++ )
			{
				mstudiobbox_t *pHitbox = pHitboxSet->pHitbox(j);

				fprintf( fp, "$hbox %d \"%s\" %f %f %f %f %f %f\r\n", pHitbox->group, pStudioHdr->pBone( pHitbox->bone )->pszName(), pHitbox->bbmin.x, pHitbox->bbmin.y, pHitbox->bbmin.z, pHitbox->bbmax.x, pHitbox->bbmax.y, pHitbox->bbmax.z );
			}
		}
	}

	fprintf( fp, "\r\n" );
	fprintf( fp, "// %d animation sequence(s)\r\n", pStudioHdr->numseq );

	for( int i = 0; i < pStudioHdr->numseq; i++ )
	{
	//	mstudioseqdesc37_t *pSeqdesc = pStudioHdr->pSeqdesc(i);
		mstudioseqdesc_t *pSeqdesc = pStudioHdr->pSeqdesc(i);

		unsigned short anim[32][32];
		memset(anim, 0, sizeof(unsigned short)*32*32);
		v37Convert( pSeqdesc, anim );

		mstudioanimdesc_t *pAnimdesc = GetAnimDesc( pStudioHdr, anim[0][0] );

		fprintf( fp, "$sequence \"%s\" \"%s\" fps %.0f ", pSeqdesc->pszLabel(), pSeqdesc->pszLabel(), pAnimdesc->fps );

		bool looping = pSeqdesc->flags & STUDIO_LOOPING ? true : false;

		if( looping )
		{
			fprintf( fp, "loop " );
		}

		fprintf( fp, "\r\n" );
	}

	fprintf( fp, "\r\n" );
	fprintf( fp, "// End of QC script.\r\n" );
	fprintf( fp, "\r\n" );

	fclose(fp);

	return true;
}

//bool GenerateReferenceSMD37( studiohdr37_t *pStudioHdr, OptimizedModel::FileHeader_t *pVtxHdr )
bool GenerateReferenceSMD37(studiohdr_t *pStudioHdr, OptimizedModel::FileHeader_t *pVtxHdr)
{
	for( int bodyPartID = 0; bodyPartID < pVtxHdr->numBodyParts; bodyPartID++ )
	{
		OptimizedModel::BodyPartHeader_t *bodyPart = pVtxHdr->pBodyPart( bodyPartID );
		mstudiobodyparts_t *pStudioBodyPart = pStudioHdr->pBodypart( bodyPartID );

		for( int lodID = 0; lodID < pVtxHdr->numLODs; lodID++ )
		{
			for( int modelID = 0; modelID < bodyPart->numModels; modelID++ )
			{
				OptimizedModel::ModelHeader_t *model = bodyPart->pModel( modelID );
				mstudiomodel_t *pStudioModel = pStudioBodyPart->pModel( modelID );
				OptimizedModel::ModelLODHeader_t *pLOD = model->pLOD( lodID );

				FILE *fp = NULL;

				if( pStudioModel->nummeshes )
				{
					char pModelName[MAX_PATH];
					strncpy( pModelName, pStudioModel->name, sizeof( pModelName ) );
					StripExtension( pModelName );

					char pEndRefName[MAX_PATH];
					sprintf( pEndRefName, "%s\\%s_lod%d.smd", g_modelsrcpath, pModelName, lodID );

					fp = fopen( pEndRefName, "wb" );

					fprintf( fp, "version 1\r\n" );
					fprintf( fp, "nodes\r\n" );

					for( int i = 0; i < pStudioHdr->numbones; i++ )
					{
						mstudiobone_t *pBone = pStudioHdr->pBone(i);

						fprintf( fp, "  %d \"%s\" %d\r\n", i, pBone->pszName(), pBone->parent );
					}

					fprintf( fp, "end\r\n" );
					fprintf( fp, "skeleton\r\n" );
					fprintf( fp, "time 0\r\n" );

					for( int i = 0; i < pStudioHdr->numbones; i++ )
					{
						mstudiobone_t *pBone = pStudioHdr->pBone(i);
						fprintf( fp, "  %d %f %f %f %f %f %f\r\n", i, pBone->value[0], pBone->value[1], pBone->value[2], pBone->value[3], pBone->value[4], pBone->value[5] );
					}

					fprintf( fp, "end\r\n" );
					fprintf( fp, "triangles\r\n" );

					for( int meshID = 0; meshID < pLOD->numMeshes; meshID++ )
					{
						OptimizedModel::MeshHeader_t *mesh = pLOD->pMesh( meshID );
						mstudiomesh_t *pStudioMesh = pStudioModel->pMesh( meshID );

						for( int stripGroupID = 0; stripGroupID < mesh->numStripGroups; stripGroupID++ )
						{
							OptimizedModel::StripGroupHeader_t *pStripGroup = mesh->pStripGroup( stripGroupID );

							for (int i = 0; i < pStripGroup->numIndices; i += 3)
							{
								unsigned short nIndex1 = *pStripGroup->pIndex( i + 2 );
								unsigned short nIndex2 = *pStripGroup->pIndex( i + 1 );
								unsigned short nIndex3 = *pStripGroup->pIndex( i );

								OptimizedModel::Vertex_t* pVert1 = pStripGroup->pVertex( nIndex1 );
								OptimizedModel::Vertex_t* pVert2 = pStripGroup->pVertex( nIndex2 );
								OptimizedModel::Vertex_t* pVert3 = pStripGroup->pVertex( nIndex3 );

								fprintf( fp, "%s.bmp\r\n", pStudioHdr->pTexture( pStudioMesh->material )->pszName() );
								fprintf( fp, "%d %f %f %f %f %f %f %f %f\r\n", pStudioMesh->BoneWeights( pVert1->origMeshVertID )->bone[0], pStudioMesh->Position( pVert1->origMeshVertID )->x, pStudioMesh->Position( pVert1->origMeshVertID )->y, pStudioMesh->Position( pVert1->origMeshVertID )->z, pStudioMesh->Normal( pVert1->origMeshVertID )->x, pStudioMesh->Normal( pVert1->origMeshVertID )->y, pStudioMesh->Normal( pVert1->origMeshVertID )->z, pStudioMesh->Texcoord( pVert1->origMeshVertID )->x, pStudioMesh->Texcoord( pVert1->origMeshVertID )->y * -1 );
								fprintf( fp, "%d %f %f %f %f %f %f %f %f\r\n", pStudioMesh->BoneWeights( pVert2->origMeshVertID )->bone[0], pStudioMesh->Position( pVert2->origMeshVertID )->x, pStudioMesh->Position( pVert2->origMeshVertID )->y, pStudioMesh->Position( pVert2->origMeshVertID )->z, pStudioMesh->Normal( pVert2->origMeshVertID )->x, pStudioMesh->Normal( pVert2->origMeshVertID )->y, pStudioMesh->Normal( pVert2->origMeshVertID )->z, pStudioMesh->Texcoord( pVert2->origMeshVertID )->x, pStudioMesh->Texcoord( pVert2->origMeshVertID )->y * -1 );
								fprintf( fp, "%d %f %f %f %f %f %f %f %f\r\n", pStudioMesh->BoneWeights( pVert3->origMeshVertID )->bone[0], pStudioMesh->Position( pVert3->origMeshVertID )->x, pStudioMesh->Position( pVert3->origMeshVertID )->y, pStudioMesh->Position( pVert3->origMeshVertID )->z, pStudioMesh->Normal( pVert3->origMeshVertID )->x, pStudioMesh->Normal( pVert3->origMeshVertID )->y, pStudioMesh->Normal( pVert3->origMeshVertID )->z, pStudioMesh->Texcoord( pVert3->origMeshVertID )->x, pStudioMesh->Texcoord( pVert3->origMeshVertID )->y * -1 );
							}
						}
					}

					fprintf( fp, "end\r\n" );
					fclose(fp);
				}
			}
		}
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: return a sub frame rotation for a single bone
//-----------------------------------------------------------------------------
//void CalcBoneQuaternion37( const studiohdr37_t *pStudioHdr, int frame, float s, 
void CalcBoneQuaternion37(const studiohdr_t *pStudioHdr, int frame, float s,
						const mstudiobone_t *pbone, const mstudioanim_t *panim, Vector &pos )
{

	int					j, k;
	Quaternion			q1, q2;
	RadianEuler			angle1(0,0,0), angle2(0,0,0);
	mstudioanimvalue_t	*panimvalue;

	if (!(panim->flags & STUDIO_ROT_ANIMATED))
	{
		pos.Init( panim->u.pose.q[0], panim->u.pose.q[1], panim->u.pose.q[2] );
		return;
	}

	for (j = 0; j < 3; j++)
	{
		if (panim->u.offset[j+3] == 0)
		{
			angle2[j] = angle1[j] = pbone->value[j+3]; // default;
		}
		else
		{
			panimvalue = panim->pAnimvalue( j+3 );
			k = frame;
			// DEBUG
			if (panimvalue->num.total < panimvalue->num.valid)
				k = 0;
			while (panimvalue->num.total <= k)
			{
				k -= panimvalue->num.total;
				panimvalue += panimvalue->num.valid + 1;
				// DEBUG
				if (panimvalue->num.total < panimvalue->num.valid)
					k = 0;
			}
			// Bah, missing blend!
			if (panimvalue->num.valid > k)
			{
				angle1[j] = panimvalue[k+1].value;

				if (panimvalue->num.valid > k + 1)
				{
					angle2[j] = panimvalue[k+2].value;
				}
				else
				{
					if (panimvalue->num.total > k + 1)
						angle2[j] = angle1[j];
					else
						angle2[j] = panimvalue[panimvalue->num.valid+2].value;
				}
			}
			else
			{
				angle1[j] = panimvalue[panimvalue->num.valid].value;
				if (panimvalue->num.total > k + 1)
				{
					angle2[j] = angle1[j];
				}
				else
				{
					angle2[j] = panimvalue[panimvalue->num.valid + 2].value;
				}
			}
			angle1[j] = pbone->value[j+3] + angle1[j] * pbone->scale[j+3];
			angle2[j] = pbone->value[j+3] + angle2[j] * pbone->scale[j+3];
		}
	}

	pos[0] = angle1[0];
	pos[1] = angle1[1];
	pos[2] = angle1[2];
}

//-----------------------------------------------------------------------------
// Purpose: return a sub frame position for a single bone
//-----------------------------------------------------------------------------
//void CalcBonePosition37( const studiohdr37_t *pStudioHdr, int frame, float s, 
void CalcBonePosition37(const studiohdr_t *pStudioHdr, int frame, float s,
	const mstudiobone_t *pbone, const mstudioanim_t *panim, Vector &pos	)
{
	int					j, k;
	mstudioanimvalue_t	*panimvalue;

	if (!(panim->flags & STUDIO_POS_ANIMATED))
	{
		pos.Init( panim->u.pose.pos[0], panim->u.pose.pos[1], panim->u.pose.pos[2] );
		return;
	}

	for (j = 0; j < 3; j++)
	{
		pos[j] = pbone->value[j]; // default;
		if (panim->u.offset[j] != 0)
		{
			panimvalue = panim->pAnimvalue( j );
			/*
			if (i == 0 && j == 0)
				Con_DPrintf("%d  %d:%d  %f\n", frame, panimvalue->num.valid, panimvalue->num.total, s );
			*/
			
			k = frame;
			// DEBUG
			if (panimvalue->num.total < panimvalue->num.valid)
				k = 0;
			// find span of values that includes the frame we want
			while (panimvalue->num.total <= k)
			{
				k -= panimvalue->num.total;
				panimvalue += panimvalue->num.valid + 1;
  				// DEBUG
				if (panimvalue->num.total < panimvalue->num.valid)
					k = 0;
			}
			// if we're inside the span
			if (panimvalue->num.valid > k)
			{
				// and there's more data in the span
				if (panimvalue->num.valid > k + 1)
				{
					pos[j] += (panimvalue[k+1].value * (1.0 - s) + s * panimvalue[k+2].value) * pbone->scale[j];
				}
				else
				{
					pos[j] += panimvalue[k+1].value * pbone->scale[j];
				}
			}
			else
			{
				// are we at the end of the repeating values section and there's another section with data?
				if (panimvalue->num.total <= k + 1)
				{
					pos[j] += (panimvalue[panimvalue->num.valid].value * (1.0 - s) + s * panimvalue[panimvalue->num.valid + 2].value) * pbone->scale[j];
				}
				else
				{
					pos[j] += panimvalue[panimvalue->num.valid].value * pbone->scale[j];
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
//static void CalcRotations37( const studiohdr37_t *pStudioHdr,	Vector *pos, Vector *pos2, 
static void CalcRotations37(const studiohdr_t *pStudioHdr, Vector *pos, Vector *pos2,
	//const mstudioseqdesc37_t *pseqdesc,
	const mstudioseqdesc_t *pseqdesc,
	const mstudioanimdesc_t *panimdesc, int setframe )
{
	int					i;
	mstudiobone_t *pbone = pStudioHdr->pBone( 0 );

	int					iFrame;
	float				s;

	float fFrame = setframe; // cycle * (panimdesc->numframes - 1);

	iFrame = (int)fFrame;
	s = (fFrame - iFrame);

	mstudioanim_t *panim = panimdesc->pAnim( 0 );

	for (i = 0; i < pStudioHdr->numbones; i++, pbone++, panim++) 
	{
		if (pseqdesc->weight(i) > 0 /*&& (pbone->flags & boneMask)*/)
		{
			CalcBoneQuaternion37( pStudioHdr, iFrame, s, pbone, panim, pos2[i] );
			CalcBonePosition37  ( pStudioHdr, iFrame, s, pbone, panim, pos[i] );
		}
	}
}

//bool GenerateAnimationSMD37( studiohdr37_t *pStudioHdr )
bool GenerateAnimationSMD37(studiohdr_t *pStudioHdr)
{
	for( int i = 0; i < pStudioHdr->numseq; i++ )
	{
	//	mstudioseqdesc37_t *pSeqdesc = pStudioHdr->pSeqdesc(i);
		mstudioseqdesc_t *pSeqdesc = pStudioHdr->pSeqdesc(i);

		unsigned short anim[32][32];
		memset(anim, 0, sizeof(unsigned short)*32*32);
		v37Convert( pSeqdesc, anim );

		mstudioanimdesc_t *pAnimdesc = GetAnimDesc( pStudioHdr, anim[0][0] );

		char pAnimName[MAX_PATH];
		sprintf( pAnimName, "%s//%s.smd", g_modelsrcpath, pSeqdesc->pszLabel() );

		FILE *fp = fopen( pAnimName, "wb" );

		if(!fp)
		{
			LogPrintf( "Can't write animation in %s\r\n", pAnimName );
			return false;
		}

		fprintf( fp, "version 1\r\n" );
		fprintf( fp, "nodes\r\n" );

		for( int i = 0; i < pStudioHdr->numbones; i++ )
		{
			mstudiobone_t *pBone = pStudioHdr->pBone(i);

			fprintf( fp, "  %d \"%s\" %d\r\n", i, pBone->pszName(), pBone->parent );
		}

		fprintf( fp, "end\r\n" );
		fprintf( fp, "skeleton\r\n" );

		static Vector		pos[MAXSTUDIOBONES];
		static Vector		rot[MAXSTUDIOBONES];

		for( int j = 0; j < pAnimdesc->numframes; j++ )
		{
			fprintf( fp, "time %d\r\n", j );

			CalcRotations37( pStudioHdr,	pos, rot, pSeqdesc, pAnimdesc, j );

			for( int b = 0; b < pStudioHdr->numbones; b++ )
			{
				fprintf( fp, "  %d   %f %f %f %f %f %f\r\n", b, pos[b].x, pos[b].y, pos[b].z, rot[b].x, rot[b].y, rot[b].z );
			}
		}

		fprintf( fp, "end\r\n" );

		fclose(fp);
	}

	return true;
}


// studio models
struct mstudiomodel2830_t
{
	char				name[64];

	int					type;

	float				boundingradius;

	int					nummeshes;	
	int					meshindex;
	inline mstudiomesh_t *pMesh( int i ) const { return (mstudiomesh_t *)(((byte *)this) + meshindex) + i; };

	// cache purposes
	int					numvertices;		// number of unique vertices/normals/texcoords
	int					vertexindex;		// vertex Vector
	int					normalsindex;		// normals Vector
	int					unkindex;
	int					texcoordindex;

	//Why do it?
	inline  Vector *Position( int i ) { return (Vector *)(((byte *)this) + vertexindex) + i; };
	inline  Vector *Normal( int i ) { return (Vector *)(((byte *)this) + normalsindex) + i; };
	inline  Vector2D *Texcoord( int i ) { return (Vector2D *)(((byte *)this) + texcoordindex) + i; };

	int					numeyeballs;
	int					eyeballindex;
	inline  mstudioeyeball_t *pEyeball( int i ) { return (mstudioeyeball_t *)(((byte *)this) + eyeballindex) + i; };

	int					unused[3];		// remove as appropriate
};

//bool GenerateReferenceSMD2830( studiohdr_t *pStudioHdr, OptimizedModel::FileHeader_t *pVtxHdr )
bool GenerateReferenceSMD2830(studiohdr_v36_t *pStudioHdr, OptimizedModel::FileHeader_t *pVtxHdr)
{
	for( int bodyPartID = 0; bodyPartID < pVtxHdr->numBodyParts; bodyPartID++ )
	{
		OptimizedModel::BodyPartHeader_t *bodyPart = pVtxHdr->pBodyPart( bodyPartID );
		mstudiobodyparts_t *pStudioBodyPart = pStudioHdr->pBodypart( bodyPartID );

		for( int lodID = 0; lodID < pVtxHdr->numLODs; lodID++ )
		{
			for( int modelID = 0; modelID < bodyPart->numModels; modelID++ )
			{
				OptimizedModel::ModelHeader_t *model = bodyPart->pModel( modelID );
				mstudiomodel2830_t *pStudioModel = (mstudiomodel2830_t *)pStudioBodyPart->pModel( modelID );
				OptimizedModel::ModelLODHeader_t *pLOD = model->pLOD( lodID );

				FILE *fp = NULL;

				if( pStudioModel->nummeshes )
				{
					char pModelName[MAX_PATH];
					strncpy( pModelName, pStudioModel->name, sizeof( pModelName ) );
					StripExtension( pModelName );

					char pEndRefName[MAX_PATH];
					sprintf( pEndRefName, "%s\\%s_lod%d.smd", g_modelsrcpath, pModelName, lodID );

					fp = fopen( pEndRefName, "wb" );

					fprintf( fp, "version 1\r\n" );
					fprintf( fp, "nodes\r\n" );

					for( int i = 0; i < pStudioHdr->numbones; i++ )
					{
						mstudiobone_t *pBone = pStudioHdr->pBone(i);

						fprintf( fp, "  %d \"%s\" %d\r\n", i, pBone->pszName(), pBone->parent );
					}

					fprintf( fp, "end\r\n" );
					fprintf( fp, "skeleton\r\n" );
					fprintf( fp, "time 0\r\n" );

					for( int i = 0; i < pStudioHdr->numbones; i++ )
					{
						mstudiobone_t *pBone = pStudioHdr->pBone(i);
						fprintf( fp, "  %d %f %f %f %f %f %f\r\n", i, pBone->value[0], pBone->value[1], pBone->value[2], pBone->value[3], pBone->value[4], pBone->value[5] );
					}

					fprintf( fp, "end\r\n" );
					fprintf( fp, "triangles\r\n" );

					for( int meshID = 0; meshID < pLOD->numMeshes; meshID++ )
					{
						OptimizedModel::MeshHeader_t *mesh = pLOD->pMesh( meshID );
						mstudiomesh_t *pStudioMesh = pStudioModel->pMesh( meshID );

						for( int stripGroupID = 0; stripGroupID < mesh->numStripGroups; stripGroupID++ )
						{
							OptimizedModel::StripGroupHeader_t *pStripGroup = mesh->pStripGroup( stripGroupID );

							for (int i = 0; i < pStripGroup->numIndices; i += 3)
							{
								unsigned short nIndex1 = *pStripGroup->pIndex( i + 2 );
								unsigned short nIndex2 = *pStripGroup->pIndex( i + 1 );
								unsigned short nIndex3 = *pStripGroup->pIndex( i );

								OptimizedModel::Vertex_t* pVert1 = pStripGroup->pVertex( nIndex1 );
								OptimizedModel::Vertex_t* pVert2 = pStripGroup->pVertex( nIndex2 );
								OptimizedModel::Vertex_t* pVert3 = pStripGroup->pVertex( nIndex3 );

								fprintf( fp, "%s.bmp\r\n", pStudioHdr->pTexture( pStudioMesh->material )->pszName() );
								fprintf( fp, "0 %f %f %f %f %f %f %f %f\r\n", pStudioModel->Position( pVert1->origMeshVertID )->x, pStudioModel->Position( pVert1->origMeshVertID )->y, pStudioModel->Position( pVert1->origMeshVertID )->z, pStudioModel->Normal( pVert1->origMeshVertID )->x, pStudioModel->Normal( pVert1->origMeshVertID )->y, pStudioModel->Normal( pVert1->origMeshVertID )->z, pStudioModel->Texcoord( pVert1->origMeshVertID )->x, pStudioModel->Texcoord( pVert1->origMeshVertID )->y * -1 );
								fprintf( fp, "0 %f %f %f %f %f %f %f %f\r\n", pStudioModel->Position( pVert2->origMeshVertID )->x, pStudioModel->Position( pVert2->origMeshVertID )->y, pStudioModel->Position( pVert2->origMeshVertID )->z, pStudioModel->Normal( pVert2->origMeshVertID )->x, pStudioModel->Normal( pVert2->origMeshVertID )->y, pStudioModel->Normal( pVert2->origMeshVertID )->z, pStudioModel->Texcoord( pVert2->origMeshVertID )->x, pStudioModel->Texcoord( pVert2->origMeshVertID )->y * -1 );
								fprintf( fp, "0 %f %f %f %f %f %f %f %f\r\n", pStudioModel->Position( pVert3->origMeshVertID )->x, pStudioModel->Position( pVert3->origMeshVertID )->y, pStudioModel->Position( pVert3->origMeshVertID )->z, pStudioModel->Normal( pVert3->origMeshVertID )->x, pStudioModel->Normal( pVert3->origMeshVertID )->y, pStudioModel->Normal( pVert3->origMeshVertID )->z, pStudioModel->Texcoord( pVert3->origMeshVertID )->x, pStudioModel->Texcoord( pVert3->origMeshVertID )->y * -1 );
							}
						}
					}

					fprintf( fp, "end\r\n" );
					fclose(fp);
				}
			}
		}
	}

	return true;
}

struct rgbdata8b_t
{
	byte	b;
	byte	g;
	byte	r;
};

int GetOrAddPixeltoPallet( rgbdata8b_t pRgbData, RGBQUAD *Palette )
{
	int colorid = ( pRgbData.b + pRgbData.g + pRgbData.r ) / 3; //get value 0...255 :)

	Palette[colorid].rgbBlue = pRgbData.b;
	Palette[colorid].rgbGreen = pRgbData.g;
	Palette[colorid].rgbRed = pRgbData.r;

	return colorid;
}

bool SaveBMP8( char *pImageName )
{
	BITMAPFILEHEADER bfh;
	memset( &bfh, 0, sizeof( BITMAPFILEHEADER ) );

	// define the bitmap file header
	bfh.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * 256 + image.width * image.height;
	bfh.bfType = 0x4D42;
	bfh.bfReserved1 = 0;
	bfh.bfReserved2 = 0;
	bfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * 256;

	BITMAPINFOHEADER bih;
	memset( &bih, 0, sizeof( BITMAPINFOHEADER ) );

	// define the bitmap information header
	bih.biSize = sizeof(BITMAPINFOHEADER);
	bih.biPlanes = 1;
	bih.biBitCount = 8;                        // 8-bit
	bih.biCompression = BI_RGB;                // no compression
	bih.biSizeImage = image.width * image.height;    // width * height * indez
	bih.biXPelsPerMeter = 0;
	bih.biYPelsPerMeter = 0;
	bih.biClrUsed = 256;
	bih.biClrImportant = 0;
	bih.biWidth = image.width;                        // bitmap width
	bih.biHeight = image.height;                    // bitmap height

	RGBQUAD Palette[256];

	rgbdata8b_t pRgbData;

	int pixid = 0;
	int *pixelsindex = (int *)malloc( sizeof(int) * image.width * image.height );

	for( int height = image.height - 1; height > 0; height-- )
	{
		for( int width = 0; width < image.width; width++ )
		{
			int pixelid = ( (height * image.width) + width) * 4;

			byte b = image.rgba[pixelid+2];
			byte r = image.rgba[pixelid];
			byte g = image.rgba[pixelid+1];

			pRgbData.b = b;
			pRgbData.g = g;
			pRgbData.r = r;

			pixelsindex[pixid] = GetOrAddPixeltoPallet( pRgbData, Palette );
			pixid++;
		}
	}

	FILE *fp = fopen( pImageName, "wb" );

	if(!fp)
	{
		LogPrintf( "Can't create texture: %s\r\n", pImageName  );
		return false;
	}

	// write the bitmap file header
	fwrite(&bfh, 1, sizeof(BITMAPFILEHEADER), fp);

	// write the bitmap info header
	fwrite(&bih, 1, sizeof(BITMAPINFOHEADER), fp);

	// write pallete
	fwrite(&Palette, 1, sizeof(RGBQUAD) * 256, fp);

	// write data
	for( int ind = 0; ind < image.width * image.height; ind++ )
	{
		fwrite(&pixelsindex[ind], 1, 1, fp);
	}

	fclose(fp);

	return true;
}

bool SaveBMP24( char *pImageName )
{
	BITMAPFILEHEADER bfh;
	memset( &bfh, 0, sizeof( BITMAPFILEHEADER ) );

	// define the bitmap file header
	bfh.bfSize = sizeof(BITMAPFILEHEADER);
	bfh.bfType = 0x4D42;
	bfh.bfReserved1 = 0;
	bfh.bfReserved2 = 0;
	bfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

	BITMAPINFOHEADER bih;
	memset( &bih, 0, sizeof( BITMAPINFOHEADER ) );

	// define the bitmap information header
	bih.biSize = sizeof(BITMAPINFOHEADER);
	bih.biPlanes = 1;
	bih.biBitCount = 24;                        // 24-bit
	bih.biCompression = BI_RGB;                // no compression
	bih.biSizeImage = image.width * image.height * 3;    // width * height * (RGB bytes)
	bih.biXPelsPerMeter = 0;
	bih.biYPelsPerMeter = 0;
	bih.biClrUsed = 0;
	bih.biClrImportant = 0;
	bih.biWidth = image.width;                        // bitmap width
	bih.biHeight = image.height;                    // bitmap height

	FILE *fp =fopen( pImageName, "wb" );

	if(!fp)
	{
		LogPrintf( "Can't create texture: %s\r\n", pImageName  );
		return false;
	}

	// write the bitmap file header
	fwrite(&bfh, 1, sizeof(BITMAPFILEHEADER), fp);

	// write the bitmap info header
	fwrite(&bih, 1, sizeof(BITMAPINFOHEADER), fp);

	for( int height = image.height - 1; height > 0; height-- )
	{
		for( int width = 0; width < image.width; width++ )
		{
			int pixelid = ( (height * image.width) + width) * 4;

			byte r = image.rgba[pixelid];
			byte g = image.rgba[pixelid+1];
			byte b = image.rgba[pixelid+2];

			fwrite(&b, 1, 1, fp);
			fwrite(&g, 1, 1, fp);
			fwrite(&r, 1, 1, fp);
		}

	}

	fclose(fp);

	return true;
}

//bool CopyTextureFiles2836( studiohdr_t *pStudioHdr )
bool CopyTextureFiles2836(studiohdr_v36_t *pStudioHdr)
{
	char pAppDir[MAX_PATH];
	memset( pAppDir, 0, sizeof(pAppDir) );
	GetCurrentDirectory( sizeof(pAppDir), pAppDir );

	char pConfigName[256];
	memset(pConfigName, 0, sizeof(pConfigName));
	sprintf(pConfigName, "%s\\config.ini", pAppDir );

	char pMaterialsPath[256];
	memset(pMaterialsPath, 0, sizeof(pMaterialsPath));

	if(!GetPrivateProfileString( "Configuration", "materialspath", NULL, pMaterialsPath, sizeof(pMaterialsPath), pConfigName ) )
	{
		LogPrintf( "Can't get materialspath\r\n" );
		return 0;
	}

	for( int i = 0; i < pStudioHdr->numtextures; i++ )
	{
		mstudiotexture_t *pTexture = pStudioHdr->pTexture( i );

		for( int j = 0; j < pStudioHdr->numcdtextures; j++ )
		{
			char pTextureName[MAX_PATH];
			sprintf( pTextureName, "%s%s%s.vtf", pMaterialsPath, pStudioHdr->pCdtexture(j), pTexture->pszName() );
			COM_WinFixSlashes( pTextureName );

			FILE *fp = fopen( pTextureName, "rb" );

			if( !fp )
			{
			//	LogPrintf( "Can't open file: %s\r\n", pTextureName );
				continue;
			}

			fseek( fp, 0, SEEK_END );

			int len = ftell( fp );
			rewind( fp );

			unsigned char *vtfhdr = ( unsigned char  * )malloc( len );
			fread( vtfhdr, 1, len, fp );

			fclose(fp);

			if( Image_LoadVTF( pTextureName, vtfhdr, len )  )
			{
				char pNewTexureFile[MAX_PATH];
				sprintf( pNewTexureFile, "%s\\%s\\%s.bmp",  pAppDir, g_modelsrcpath, pTexture->pszName() );
			//	SaveBMP24( pNewTexureFile );
				SaveBMP8( pNewTexureFile );
			}

/*
			char pNewTexureFile[MAX_PATH];
			sprintf( pNewTexureFile, "%s\\%s\\%s.vtf",  pAppDir, g_modelsrcpath, pTexture->pszName() );

			CopyFile( pTextureName, pNewTexureFile, FALSE );
*/
		}
	}

	return true;
}


//bool CopyTextureFiles37( studiohdr37_t *pStudioHdr )
bool CopyTextureFiles37(studiohdr_t *pStudioHdr)
{
	char pAppDir[MAX_PATH];
	memset( pAppDir, 0, sizeof(pAppDir) );
	GetCurrentDirectory( sizeof(pAppDir), pAppDir );

	char pConfigName[256];
	memset(pConfigName, 0, sizeof(pConfigName));
	sprintf(pConfigName, "%s\\config.ini", pAppDir );

	char pMaterialsPath[256];
	memset(pMaterialsPath, 0, sizeof(pMaterialsPath));

	if(!GetPrivateProfileString( "Configuration", "materialspath", NULL, pMaterialsPath, sizeof(pMaterialsPath), pConfigName ) )
	{
		LogPrintf( "Can't get materialspath\r\n" );
		return 0;
	}

	for( int i = 0; i < pStudioHdr->numtextures; i++ )
	{
		mstudiotexture_t *pTexture = pStudioHdr->pTexture( i );

		for( int j = 0; j < pStudioHdr->numcdtextures; j++ )
		{
			char pTextureName[MAX_PATH];
			sprintf( pTextureName, "%s%s%s.vtf", pMaterialsPath, pStudioHdr->pCdtexture(j), pTexture->pszName() );
			COM_WinFixSlashes( pTextureName );

			FILE *fp = fopen( pTextureName, "rb" );

			if( !fp )
			{
			//	LogPrintf( "Can't open file: %s\r\n", pTextureName );
				continue;
			}

			fseek( fp, 0, SEEK_END );

			int len = ftell( fp );
			rewind( fp );

			unsigned char *vtfhdr = ( unsigned char  * )malloc( len );
			fread( vtfhdr, 1, len, fp );

			fclose(fp);

			if( Image_LoadVTF( pTextureName, vtfhdr, len )  )
			{
				char pNewTexureFile[MAX_PATH];
				sprintf( pNewTexureFile, "%s\\%s\\%s.bmp",  pAppDir, g_modelsrcpath, pTexture->pszName() );
			//	SaveBMP24( pNewTexureFile );
				SaveBMP8( pNewTexureFile );
			}

/*
			char pNewTexureFile[MAX_PATH];
			sprintf( pNewTexureFile, "%s\\%s\\%s.vtf",  pAppDir, g_modelsrcpath, pTexture->pszName() );

			CopyFile( pTextureName, pNewTexureFile, FALSE );
*/
		}
	}

	return true;
}




int main( int argc, char **argv )
{
	char const *pArgFilename = Sys_FindArg( "-file", NULL );

	if(!pArgFilename)
	{
		LogPrintf( "Use option: -file <model name>\r\n" );
		return 0;
	}

	MathLib_Init(2.2f, 2.2f, 0.0f, 2.0f, false, false, false, false);

	char pFilename[MAX_PATH];
	strncpy( pFilename, pArgFilename, sizeof( pFilename ) );

	StripExtension( pFilename );

//	studiohdr_t *pStudioHdr = LoadSudioHdr( pFilename );
	studiohdr_v36_t *pStudioHdr = LoadSudioHdr(pFilename);
	OptimizedModel::FileHeader_t *pVtxHdr = LoadVtxHdr( pFilename );

	LogPrintf( "Load model: %s\r\n", pStudioHdr->name );

	if( pStudioHdr->version > 27 && pStudioHdr->version < 31 )
	{
		//size mstudiomodel_t in 30 = 120 bytes
		GenerateQCFile2836( pStudioHdr );
		GenerateReferenceSMD2830( pStudioHdr, pVtxHdr );
		GenerateAnimationSMD2836( pStudioHdr );
		CopyTextureFiles2836( pStudioHdr );
	}
	else if( pStudioHdr->version > 30 && pStudioHdr->version < 37 )
	{
		GenerateQCFile2836( pStudioHdr );
		GenerateReferenceSMD3136( pStudioHdr, pVtxHdr );
		GenerateAnimationSMD2836( pStudioHdr );
		CopyTextureFiles2836( pStudioHdr );
	}
	else if( pStudioHdr->version == 37 )
	{
	//	GenerateQCFile37( (studiohdr37_t *)pStudioHdr );
	//	GenerateReferenceSMD37((studiohdr37_t *)pStudioHdr, pVtxHdr);
	//	GenerateAnimationSMD37((studiohdr37_t *)pStudioHdr);
	//	CopyTextureFiles37((studiohdr37_t *)pStudioHdr);
		GenerateQCFile37((studiohdr_t *)pStudioHdr);
		GenerateReferenceSMD37((studiohdr_t *)pStudioHdr, pVtxHdr);
		GenerateAnimationSMD37((studiohdr_t *)pStudioHdr);
		CopyTextureFiles37((studiohdr_t *)pStudioHdr);
	}

	return 1;
}

