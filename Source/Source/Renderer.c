#include <Renderer.h>
#include <km2dbg.h>
#include <Memory.h>
#include <Log.h>

/* Using the macro interface seems to make things worse
 * The recommended L5 is slower than L4 and L3 -- INVESTIGATE */
#define _KM_USE_VERTEX_MACRO_
#define _KM_USE_VERTEX_MACRO_L3_
#include <kamui2.h>
#include <kamuix.h>

static KMSYSTEMCONFIGSTRUCT g_Kamui2Configuration;
static KMSTRIPHEAD g_StripHead00;
static KMVERTEXBUFFDESC g_VertexBufferDescription;
static KMSTRIPCONTEXT g_DefaultStripContext =
{
	0x90,							/* Size */
	{
		KM_OPAQUE_POLYGON,			/* List type */
		KM_USERCLIP_DISABLE,		/* User clip mode */
		KM_NORMAL_POLYGON,			/* Shadow mode */
		KM_FLOATINGCOLOR,			/* Intensity mode */
		KM_FALSE,					/* Bump map */
		KM_TRUE						/* Gouraud shading */
	}, /* KMSTRIPCONTROL */
	{
		KM_GREATER,					/* Depth compare */
		KM_NOCULLING,				/* Culling mode */
		KM_FALSE,					/* Disable Z writing */
		KM_FALSE,					/* Mipmap D calculation control */
		KM_MODIFIER_NORMAL_POLY		/* Modifier instruction */
	}, /* KMOBJECTCONTROL */	
	{
		0,							/* Base */
		0							/* Offset */
	}, /* type */
	{
		KM_ONE,						/* Source blending mode */
		KM_ZERO,					/* Destination blending mode */
		KM_FALSE,					/* Source select */
		KM_FALSE,					/* Destination select */
		KM_NOFOG,					/* Fog mode */
		KM_FALSE,					/* Colour clamp */
		KM_FALSE,					/* Use vertex alpha */
		KM_FALSE,					/* Ignore texture alpha */
		KM_NOFLIP,					/* Flip UV */
		KM_NOCLAMP,					/* Clamp UV */
		KM_BILINEAR,				/* Filter mode */
		KM_FALSE,					/* Super sample mode */
		KM_MIPMAP_D_ADJUST_1_00,	/* Mipmap adjust */
		KM_MODULATE,				/* Texture shading mode */
		0,							/* Palette bank */
		NULL						/* Texture surface description */
	} /* KMIMAGECONTROL[ 1 ] */
};

static void Kamui2AssertCallback( PKMASSERTIONFAIL p_pInfo )
{
	LOG_Debug( "-- KAMUI 2 ASSERT --" );
	LOG_Debug( "\tError message: %s", p_pInfo->pszErrMsg );
	LOG_Debug( "\tFormula:       %s", p_pInfo->pszFormula);
	LOG_Debug( "\tFile:          %s", p_pInfo->pszFile );
	LOG_Debug( "\tLine:          %d", p_pInfo->nLine );
}

static void Kamui2FatalErrorCallback( PKMVOID p_pArgs )
{
	LOG_Debug( "FATAL" );
}

static void Kamui2StripOverrunCallback( PKMVOID p_pArgs )
{
	LOG_Debug( "STRIP" );
}

static void Kamui2TextureOverflowCallback( PKMVOID p_pArgs )
{
	LOG_Debug( "TEXTURE" );
}

Sint32 REN_Initialise( PRENDERER p_pRenderer,
	const PRENDERER_CONFIGURATION p_pRendererConfiguration )
{
	KMUINT32 PassIndex = 0UL;
	KMSTATUS Kamui2Status;
	PKMDWORD pVertexBuffer;
	PKMDWORD pTextureWorkArea;

	g_Kamui2Configuration.dwSize = sizeof( g_Kamui2Configuration );

	g_Kamui2Configuration.flags =
		KM_CONFIGFLAG_ENABLE_CLEAR_FRAMEBUFFER |
		KM_CONFIGFLAG_NOWAITVSYNC |
		KM_CONFIGFLAG_ENABLE_2V_LATENCY |
		KM_CONFIGFLAG_NOWAIT_FINISH_TEXTUREDMA;
	
	g_Kamui2Configuration.ppSurfaceDescArray =
		p_pRendererConfiguration->ppSurfaceDescription;
	g_Kamui2Configuration.fb.nNumOfFrameBuffer =
		p_pRendererConfiguration->FramebufferCount;
	
	g_Kamui2Configuration.nTextureMemorySize =
		p_pRendererConfiguration->TextureMemorySize;
	g_Kamui2Configuration.nNumOfTextureStruct =
		p_pRendererConfiguration->MaximumTextureCount;
	g_Kamui2Configuration.nNumOfSmallVQStruct =
		p_pRendererConfiguration->MaximumSmallVQTextureCount;

	pTextureWorkArea = MEM_AllocateFromMemoryBlock(
		p_pRendererConfiguration->pMemoryBlock,
		p_pRendererConfiguration->MaximumTextureCount * 24 +
			p_pRendererConfiguration->MaximumSmallVQTextureCount * 76,
		"Texture work area" );

	if( pTextureWorkArea == NULL )
	{
		LOG_Debug( "[REN_Initialise] <ERROR> Failed to allocate memory for "
			"the texture work area" );

		return REN_TEXTUREWORKAREAMEMORY;
	}

	g_Kamui2Configuration.pTextureWork = pTextureWorkArea;

	pVertexBuffer = MEM_AllocateFromMemoryBlock(
		p_pRendererConfiguration->pMemoryBlock,
		p_pRendererConfiguration->VertexBufferSize, "Vertex buffer" );

	if( pVertexBuffer == NULL )
	{
		LOG_Debug( "[REN_Initialise] <ERROR> Failed to allocate memory for "
			"the vertesx buffer" );

		return REN_VERTEXBUFFERMEMORY;
	}

	g_Kamui2Configuration.pVertexBuffer =
		( PKMDWORD )MEM_SH4_P2NonCachedMemory( pVertexBuffer );
	g_Kamui2Configuration.pBufferDesc = &g_VertexBufferDescription;
	g_Kamui2Configuration.nVertexBufferSize =
		p_pRendererConfiguration->VertexBufferSize;

	g_Kamui2Configuration.nNumOfVertexBank = 1;

	g_Kamui2Configuration.nPassDepth = p_pRendererConfiguration->PassCount;

	for( ; PassIndex < p_pRendererConfiguration->PassCount; ++PassIndex )
	{
		KMUINT32 BufferIndex = 0;

		for( ; BufferIndex < KM_MAX_DISPLAY_LIST; ++BufferIndex )
		{
			g_Kamui2Configuration.Pass[ PassIndex ].fBufferSize[ BufferIndex ]
				= p_pRendererConfiguration->PassInfo[ PassIndex ].fBufferSize[
					BufferIndex ];
			g_Kamui2Configuration.Pass[ PassIndex ].dwOPBSize[ BufferIndex ] =
				p_pRendererConfiguration->PassInfo[ PassIndex ].dwOPBSize[
					BufferIndex ];
		}

		g_Kamui2Configuration.Pass[ PassIndex ].nDirectTransferList =
			p_pRendererConfiguration->PassInfo[
				PassIndex ].nDirectTransferList;

		g_Kamui2Configuration.Pass[ PassIndex ].dwRegionArrayFlag =
			p_pRendererConfiguration->PassInfo[ PassIndex ].dwRegionArrayFlag;
	}

	Kamui2Status = kmSetSystemConfiguration( &g_Kamui2Configuration );

	if( Kamui2Status != KMSTATUS_SUCCESS )
	{
		if( Kamui2Status == KMSTATUS_NOT_ENOUGH_MEMORY )
		{
			LOG_Debug( "[REN_Initialise] <ERROR> No more memory left" );

			return REN_OUTOFMEMORY;
		}

		LOG_Debug( "[REN_Initialise] <ERROR> Unknown error initialising "
			"Kamui 2" );

		return REN_FATALERROR;
	}

	memset( &g_StripHead00, 0, sizeof( g_StripHead00 ) );
	kmGenerateStripHead00( &g_StripHead00, &g_DefaultStripContext );

	p_pRenderer->pMemoryBlock = p_pRendererConfiguration->pMemoryBlock;

	/* Allocate memory for 2,000 vertices of type 05 */
	p_pRenderer->pVertices05 = MEM_AllocateFromMemoryBlock(
		p_pRenderer->pMemoryBlock, 2000 * sizeof( KMVERTEX5 ),
		"Renderer: Temporary vertices [TYPE 05]" );

	if( p_pRenderer->pVertices05 == NULL )
	{
		LOG_Debug( "[REN_Initialise] Failed to allocate memory for the "
			"temporary vertices" );

		return REN_OUTOFMEMORY;
	}

	LOG_Debug( "[REN_Initialise] Vertices allocated at 0x%08X",
		p_pRenderer->pVertices05 );

#if defined ( SCREAM_BUILD_DEBUG )
	if( kmSetAssertCallback( ( PKMCALLBACKFUNC )Kamui2AssertCallback ) !=KMSTATUS_SUCCESS )
	{
		LOG_Debug( "FUCKED UP" );
	}
#endif /* SCREAM_BUILD_DEBUG */
	if( kmSetFatalErrorCallback( Kamui2FatalErrorCallback, NULL ) != KMSTATUS_SUCCESS )
	{
		LOG_Debug( "FUCKED UP 2" );
	}
	if( kmSetStripOverRunCallback( Kamui2StripOverrunCallback, NULL ) != KMSTATUS_SUCCESS )
	{
		LOG_Debug( "FUCKED UP 3" );
	}
	if( kmSetTexOverflowCallback( Kamui2TextureOverflowCallback, NULL ) != KMSTATUS_SUCCESS )
	{
		LOG_Debug( "FUCKED UP 4" );
	}

	p_pRenderer->pTextureWorkArea = pTextureWorkArea;
	p_pRenderer->pVertexBuffer = pVertexBuffer;

	LOG_Debug( "[REN_Initialise] Renderer initialised" );

	return REN_OK;
}

void REN_Terminate( PRENDERER p_pRenderer )
{
	MEM_FreeFromMemoryBlock( p_pRenderer->pMemoryBlock,
		p_pRenderer->pTextureWorkArea );
	MEM_FreeFromMemoryBlock( p_pRenderer->pMemoryBlock,
		p_pRenderer->pVertexBuffer );
	MEM_FreeFromMemoryBlock( p_pRenderer->pMemoryBlock,
		p_pRenderer->pVertices05 );
}

void REN_SetClearColour( float p_Red, float p_Green, float p_Blue )
{
	KMSTRIPHEAD BackgroundStripHead;
	KMVERTEX_01 BackgroundClear[ 3 ];
	KMPACKEDARGB BorderColour;

	memset( &BackgroundStripHead, 0, sizeof( BackgroundStripHead ) );
	kmGenerateStripHead01( &BackgroundStripHead, &g_DefaultStripContext );

	BackgroundClear[ 0 ].ParamControlWord = KM_VERTEXPARAM_NORMAL;
	BackgroundClear[ 0 ].fX = 0.0f;
	BackgroundClear[ 0 ].fY = 479.0f;
	BackgroundClear[ 0 ].u.fZ = 0.0001f;
	BackgroundClear[ 0 ].fBaseAlpha = 1.0f;
	BackgroundClear[ 0 ].fBaseRed = p_Red;
	BackgroundClear[ 0 ].fBaseGreen = p_Green;
	BackgroundClear[ 0 ].fBaseBlue = p_Blue;

	BackgroundClear[ 1 ].ParamControlWord = KM_VERTEXPARAM_NORMAL;
	BackgroundClear[ 1 ].fX = 639.0f;
	BackgroundClear[ 1 ].fY = 0.0f;
	BackgroundClear[ 1 ].u.fZ = 0.0001f;
	BackgroundClear[ 1 ].fBaseAlpha = 1.0f;
	BackgroundClear[ 1 ].fBaseRed = p_Red;
	BackgroundClear[ 1 ].fBaseGreen = p_Green;
	BackgroundClear[ 1 ].fBaseBlue = p_Blue;

	BackgroundClear[ 2 ].ParamControlWord = KM_VERTEXPARAM_ENDOFSTRIP;
	BackgroundClear[ 2 ].fX = 639.0f;
	BackgroundClear[ 2 ].fY = 479.0f;
	BackgroundClear[ 2 ].u.fZ = 0.0001f;
	BackgroundClear[ 2 ].fBaseAlpha = 1.0f;
	BackgroundClear[ 2 ].fBaseRed = p_Red;
	BackgroundClear[ 2 ].fBaseGreen = p_Green;
	BackgroundClear[ 2 ].fBaseBlue = p_Blue;

	kmSetBackGround( &BackgroundStripHead, KM_VERTEXTYPE_01,
		&BackgroundClear[ 0 ], &BackgroundClear[ 1 ], &BackgroundClear[ 2 ] );

	BorderColour.dwPacked = 0;

	kmSetBorderColor( BorderColour );
}

Sint32 REN_Clear( void )
{
	KMSTATUS Status;

	Status = kmBeginScene( &g_Kamui2Configuration );

	if( Status != KMSTATUS_SUCCESS )
	{
		LOG_Debug( "[REN_Clear] <ERROR> Failed to begin the scene" );

		return REN_BEGINSCENE;
	}

	Status = kmBeginPass( g_Kamui2Configuration.pBufferDesc );

	if( Status != KMSTATUS_SUCCESS )
	{
		LOG_Debug( "[REN_Clear] <ERROR> Failed to begin the pass" );

		return REN_BEGINPASS;
	}

	return REN_OK;
}

Sint32 REN_SwapBuffers( void )
{
	KMSTATUS Status;
	KMINT32 RenderResult;

	Status = kmEndPass( g_Kamui2Configuration.pBufferDesc );

	if( Status != KMSTATUS_SUCCESS )
	{
		LOG_Debug( "[REN_SwapBuffers] <ERROR> Failed to end the pass" );

		return REN_ENDPASS;
	}

	RenderResult = kmRender( KM_RENDER_FLIP );

	if( RenderResult < 0 )
	{
		LOG_Debug( "[REN_SwapBuffers] <ERROR> Render failed" );

		return REN_RENDERFAIL;
	}

	Status = kmEndScene( &g_Kamui2Configuration );

	if( Status != KMSTATUS_SUCCESS )
	{
		LOG_Debug( "[REN_SwapBuffers] <ERROR> Failed to end the scene" );

		return REN_ENDSCENE;
	}

	return REN_OK;
}

