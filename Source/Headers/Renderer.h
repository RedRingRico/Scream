#ifndef __SCREAM_RENDERER_H__
#define __SCREAM_RENDERER_H__

#include <Memory.h>
#include <kamui2.h>
#include <sg_xpt.h>

#define REN_OK 0
#define REN_ERROR 1
#define REN_FATALERROR -1
#define REN_VERTEXBUFFERMEMORY -2
#define REN_TEXTUREWORKAREAMEMORY -3
#define REN_OUTOFMEMORY -10
#define REN_BEGINSCENE -11
#define REN_BEGINPASS -12
#define REN_ENDPASS -13
#define REN_RENDERFAIL -14
#define REN_ENDSCENE -15

typedef struct _tagRENDERER_CONFIGURATION
{
	PPKMSURFACEDESC		ppSurfaceDescription;
	KMUINT32			FramebufferCount;
	KMUINT32			TextureMemorySize;
	KMUINT32			MaximumTextureCount;
	KMUINT32			MaximumSmallVQTextureCount;
	PKMVERTEXBUFFDESC	pVertexBufferDescription;
	KMUINT32			VertexBufferSize;
	KMUINT32			PassCount;
	KMPASSINFO			PassInfo[ KM_MAX_DISPLAY_LIST_PASS ];
	PMEMORY_BLOCK		pMemoryBlock;
}RENDERER_CONFIGURATION, *PRENDERER_CONFIGURATION;

typedef struct _tagRENDERER
{
	Uint32			VisiblePolygons;
	Uint32			CulledPolygons;
	Uint32			GeneratedPolygons;
	PKMVERTEX_05	pVertices05;
	PKMDWORD		pVertexBuffer;
	PKMDWORD		pTextureWorkArea;
	PMEMORY_BLOCK	pMemoryBlock;
}RENDERER, *PRENDERER;

Sint32 REN_Initialise( PRENDERER p_pRenderer,
	const PRENDERER_CONFIGURATION p_pRendererConfiguration );
void REN_Terminate( PRENDERER p_pRenderer );

void REN_SetClearColour( float p_Red, float p_Green, float p_Blue );

Sint32 REN_Clear( void );
Sint32 REN_SwapBuffers( void );

void REN_DrawPrimitives00( PKMSTRIPHEAD p_pStripHead, PKMVERTEX_00 p_pVertices,
	KMUINT32 p_Count );

#endif /* __SCREAM_RENDERER_H__ */
