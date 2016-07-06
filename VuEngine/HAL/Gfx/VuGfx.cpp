//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Interface class to Gfx library.
// 
//*****************************************************************************

#include "VuGfx.h"
#include "VuEngine/HAL/Gfx/VuTextureData.h"
#include "VuEngine/Managers/VuConfigManager.h"
#include "VuEngine/Dev/VuDevStat.h"
#include "VuEngine/Dev/VuDevConfig.h"


//*****************************************************************************
VuGfx::VuGfx():
	mFlipInterval(1),
	mSyncGPU(false),
	mPrimitiveCount(0),
	mDrawCallCount(0),
	mPrevPrimitiveCount(0),
	mPrevDrawCallCount(0),
	mMaxPrimitiveCount(0),
	mMaxDrawCallCount(0),
	mClipPlane(0,0,0,0)
{
}

//*****************************************************************************
void VuGfx::postInit()
{
	// dev stats
	if ( VuDevStat::IF() )
	{
		VuDevStat::IF()->addPage("Gfx", VuRect(70, 2, 28, 16));
	}

	if ( VuDevConfig::IF() && VuDevConfig::IF()->getParam("VisualizeTextureMipLevels").asBool() )
	{
		VuTextureData::enableVisualizeMipLevels(true);
	}

	VuConfigManager::IF()->registerIntHandler("Gfx/FlipInterval", this, &VuGfx::configFlipInterval);
}

//*****************************************************************************
void VuGfx::preRelease()
{
	VuConfigManager::IF()->unregisterIntHandler("Gfx/FlipInterval", this);
}

//*****************************************************************************
bool VuGfx::supportsVertexDeclType(const std::string &platform, eGfxDeclType declType)
{
	if (platform == "Win32")
	{
		static bool sSupportedDeclTypes[] =
		{
			true,	// VUGFX_DECL_TYPE_FLOAT1,		
			true,	// VUGFX_DECL_TYPE_FLOAT2,		
			true,	// VUGFX_DECL_TYPE_FLOAT3,		
			true,	// VUGFX_DECL_TYPE_FLOAT4,		
			false,	// VUGFX_DECL_TYPE_BYTE4,		
			false,	// VUGFX_DECL_TYPE_BYTE4N,	
			true,	// VUGFX_DECL_TYPE_UBYTE4,		
			true,	// VUGFX_DECL_TYPE_UBYTE4N,	
			true,	// VUGFX_DECL_TYPE_SHORT2N,	
			true,	// VUGFX_DECL_TYPE_SHORT4N,	
			false,	// VUGFX_DECL_TYPE_FLOAT16_2,	
			false,	// VUGFX_DECL_TYPE_FLOAT16_4,	
		};
		VU_COMPILE_TIME_ASSERT(sizeof(sSupportedDeclTypes)/sizeof(sSupportedDeclTypes[0]) == VUGFX_DECL_TYPE_COUNT);

		return sSupportedDeclTypes[declType];
	}
	else if (platform == "Android" || platform == "Ios" || platform == "BB10")
	{
		static bool sSupportedDeclTypes[] =
		{
			true,	// VUGFX_DECL_TYPE_FLOAT1,		
			true,	// VUGFX_DECL_TYPE_FLOAT2,		
			true,	// VUGFX_DECL_TYPE_FLOAT3,		
			true,	// VUGFX_DECL_TYPE_FLOAT4,		
			true,	// VUGFX_DECL_TYPE_BYTE4,		
			true,	// VUGFX_DECL_TYPE_BYTE4N,	
			true,	// VUGFX_DECL_TYPE_UBYTE4,		
			true,	// VUGFX_DECL_TYPE_UBYTE4N,	
			true,	// VUGFX_DECL_TYPE_SHORT2N,	
			true,	// VUGFX_DECL_TYPE_SHORT4N,	
			false,	// VUGFX_DECL_TYPE_FLOAT16_2,	
			false,	// VUGFX_DECL_TYPE_FLOAT16_4,	
		};
		VU_COMPILE_TIME_ASSERT(sizeof(sSupportedDeclTypes)/sizeof(sSupportedDeclTypes[0]) == VUGFX_DECL_TYPE_COUNT);

		return sSupportedDeclTypes[declType];
	}
	else if ( platform == "Windows" )
	{
		static bool sSupportedDeclTypes[] =
		{
			true,	// VUGFX_DECL_TYPE_FLOAT1,		
			true,	// VUGFX_DECL_TYPE_FLOAT2,		
			true,	// VUGFX_DECL_TYPE_FLOAT3,		
			true,	// VUGFX_DECL_TYPE_FLOAT4,		
			false,	// VUGFX_DECL_TYPE_BYTE4,		
			false,	// VUGFX_DECL_TYPE_BYTE4N,	
			true,	// VUGFX_DECL_TYPE_UBYTE4,		
			true,	// VUGFX_DECL_TYPE_UBYTE4N,	
			true,	// VUGFX_DECL_TYPE_SHORT2N,	
			true,	// VUGFX_DECL_TYPE_SHORT4N,	
			false,	// VUGFX_DECL_TYPE_FLOAT16_2,	
			false,	// VUGFX_DECL_TYPE_FLOAT16_4,	
		};
		VU_COMPILE_TIME_ASSERT(sizeof(sSupportedDeclTypes)/sizeof(sSupportedDeclTypes[0]) == VUGFX_DECL_TYPE_COUNT);

		return sSupportedDeclTypes[declType];
	}
	else if (platform == "Ps4")
	{
		static bool sSupportedDeclTypes[] =
		{
			true,	// VUGFX_DECL_TYPE_FLOAT1,		
			true,	// VUGFX_DECL_TYPE_FLOAT2,		
			true,	// VUGFX_DECL_TYPE_FLOAT3,		
			true,	// VUGFX_DECL_TYPE_FLOAT4,		
			true,	// VUGFX_DECL_TYPE_BYTE4,		
			true,	// VUGFX_DECL_TYPE_BYTE4N,	
			true,	// VUGFX_DECL_TYPE_UBYTE4,		
			true,	// VUGFX_DECL_TYPE_UBYTE4N,	
			true,	// VUGFX_DECL_TYPE_SHORT2N,	
			true,	// VUGFX_DECL_TYPE_SHORT4N,	
			true,	// VUGFX_DECL_TYPE_FLOAT16_2,
			true,	// VUGFX_DECL_TYPE_FLOAT16_4,
		};
		VU_COMPILE_TIME_ASSERT(sizeof(sSupportedDeclTypes) / sizeof(sSupportedDeclTypes[0]) == VUGFX_DECL_TYPE_COUNT);
	}
	else if ( platform == "Xb1" )
	{
		static bool sSupportedDeclTypes[] =
		{
			true,	// VUGFX_DECL_TYPE_FLOAT1,		
			true,	// VUGFX_DECL_TYPE_FLOAT2,		
			true,	// VUGFX_DECL_TYPE_FLOAT3,		
			true,	// VUGFX_DECL_TYPE_FLOAT4,		
			false,	// VUGFX_DECL_TYPE_BYTE4,		
			false,	// VUGFX_DECL_TYPE_BYTE4N,	
			true,	// VUGFX_DECL_TYPE_UBYTE4,		
			true,	// VUGFX_DECL_TYPE_UBYTE4N,	
			true,	// VUGFX_DECL_TYPE_SHORT2N,	
			true,	// VUGFX_DECL_TYPE_SHORT4N,	
			false,	// VUGFX_DECL_TYPE_FLOAT16_2,	
			false,	// VUGFX_DECL_TYPE_FLOAT16_4,	
		};
		VU_COMPILE_TIME_ASSERT(sizeof(sSupportedDeclTypes)/sizeof(sSupportedDeclTypes[0]) == VUGFX_DECL_TYPE_COUNT);

		return sSupportedDeclTypes[declType];
	}
	else
	{
		VUASSERT(0, "VuGfx::supportsVertexDeclType() unsupported platform");
	}

	return true;
}

//*****************************************************************************
bool VuGfx::supportsTextureFormat(const std::string &platform, VuGfxFormat format)
{
	if ( platform == "Win32")
	{
		static bool sSupportedFormats[] =
		{
			false, // VUGFX_FORMAT_UNKNOWN
			false, // VUGFX_FORMAT_D24S8
			true,  // VUGFX_FORMAT_V8U8
			true,  // VUGFX_FORMAT_LIN_V8U8
			true,  // VUGFX_FORMAT_LIN_R16G16B16A16_SNORM
			true,  // VUGFX_FORMAT_A8R8G8B8
			true,  // VUGFX_FORMAT_R8
			true,  // VUGFX_FORMAT_LIN_R8
			true,  // VUGFX_FORMAT_R16
			true,  // VUGFX_FORMAT_LIN_R16
			true,  // VUGFX_FORMAT_L8A8
			true,  // VUGFX_FORMAT_LIN_L8A8
			true,  // VUGFX_FORMAT_R16F
			true,  // VUGFX_FORMAT_R32F
			true,  // VUGFX_FORMAT_R16G16F
		};
		VU_COMPILE_TIME_ASSERT(sizeof(sSupportedFormats)/sizeof(sSupportedFormats[0]) == VUGFX_FORMAT_COUNT);

		return sSupportedFormats[format];
	}
	else if ( platform == "Android" || platform == "Ios" || platform == "BB10" )
	{
		static bool sSupportedFormats[] =
		{
			false, // VUGFX_FORMAT_UNKNOWN
			false, // VUGFX_FORMAT_D24S8
			false, // VUGFX_FORMAT_V8U8
			false, // VUGFX_FORMAT_LIN_V8U8
			false, // VUGFX_FORMAT_LIN_R16G16B16A16_SNORM
			true,  // VUGFX_FORMAT_A8R8G8B8
			true,  // VUGFX_FORMAT_R8
			true,  // VUGFX_FORMAT_LIN_R8
			false, // VUGFX_FORMAT_R16
			false, // VUGFX_FORMAT_LIN_R16
			true,  // VUGFX_FORMAT_L8A8
			true,  // VUGFX_FORMAT_LIN_L8A8
			false, // VUGFX_FORMAT_R16F
			false, // VUGFX_FORMAT_R32F
			false, // VUGFX_FORMAT_R16G16F
		};
		VU_COMPILE_TIME_ASSERT(sizeof(sSupportedFormats)/sizeof(sSupportedFormats[0]) == VUGFX_FORMAT_COUNT);

		return sSupportedFormats[format];
	}
	else if ( platform == "Windows" )
	{
		static bool sSupportedFormats[] =
		{
			false, // VUGFX_FORMAT_UNKNOWN
			false, // VUGFX_FORMAT_D24S8
			true,  // VUGFX_FORMAT_V8U8
			true,  // VUGFX_FORMAT_LIN_V8U8
			true,  // VUGFX_FORMAT_LIN_R16G16B16A16_SNORM
			true,  // VUGFX_FORMAT_A8R8G8B8
			true,  // VUGFX_FORMAT_R8
			true,  // VUGFX_FORMAT_LIN_R8
			true,  // VUGFX_FORMAT_R16
			true,  // VUGFX_FORMAT_LIN_R16
			false, // VUGFX_FORMAT_L8A8
			false, // VUGFX_FORMAT_LIN_L8A8
			false, // VUGFX_FORMAT_R16F
			false, // VUGFX_FORMAT_R32F
			false, // VUGFX_FORMAT_R16G16F
		};
		VU_COMPILE_TIME_ASSERT(sizeof(sSupportedFormats)/sizeof(sSupportedFormats[0]) == VUGFX_FORMAT_COUNT);

		return sSupportedFormats[format];
	}
	else if (platform == "Ps4")
	{
		static bool sSupportedFormats[] =
		{
			false, // VUGFX_FORMAT_UNKNOWN
			false, // VUGFX_FORMAT_D24S8
			true,  // VUGFX_FORMAT_V8U8
			true,  // VUGFX_FORMAT_LIN_V8U8
			true,  // VUGFX_FORMAT_LIN_R16G16B16A16_SNORM
			true,  // VUGFX_FORMAT_A8R8G8B8
			true,  // VUGFX_FORMAT_R8
			true,  // VUGFX_FORMAT_LIN_R8
			true,  // VUGFX_FORMAT_R16
			true,  // VUGFX_FORMAT_LIN_R16
			true,  // VUGFX_FORMAT_L8A8
			true,  // VUGFX_FORMAT_LIN_L8A8
			true,  // VUGFX_FORMAT_R16F
			true,  // VUGFX_FORMAT_R32F
			true,  // VUGFX_FORMAT_R16G16F
		};
		VU_COMPILE_TIME_ASSERT(sizeof(sSupportedFormats) / sizeof(sSupportedFormats[0]) == VUGFX_FORMAT_COUNT);

		return sSupportedFormats[format];
	}
	else if ( platform == "Xb1")
	{
		static bool sSupportedFormats[] =
		{
			false, // VUGFX_FORMAT_UNKNOWN
			false, // VUGFX_FORMAT_D24S8
			true,  // VUGFX_FORMAT_V8U8
			true,  // VUGFX_FORMAT_LIN_V8U8
			true,  // VUGFX_FORMAT_LIN_R16G16B16A16_SNORM
			true,  // VUGFX_FORMAT_A8R8G8B8
			true,  // VUGFX_FORMAT_R8
			true,  // VUGFX_FORMAT_LIN_R8
			true,  // VUGFX_FORMAT_R16
			true,  // VUGFX_FORMAT_LIN_R16
			true,  // VUGFX_FORMAT_L8A8
			true,  // VUGFX_FORMAT_LIN_L8A8
			true,  // VUGFX_FORMAT_R16F
			true,  // VUGFX_FORMAT_R32F
			true,  // VUGFX_FORMAT_R16G16F
		};
		VU_COMPILE_TIME_ASSERT(sizeof(sSupportedFormats)/sizeof(sSupportedFormats[0]) == VUGFX_FORMAT_COUNT);

		return sSupportedFormats[format];
	}
	else
	{
		VUASSERT(0, "VuGfx::supportsTextureFormat() unsupported platform");
	}

	return true;
}

//*****************************************************************************
void VuGfx::resetStats()
{
	mPrevPrimitiveCount = mPrimitiveCount;
	mPrevDrawCallCount = mDrawCallCount;

	mMaxPrimitiveCount = VuMax(mMaxPrimitiveCount, mPrimitiveCount);
	mMaxDrawCallCount = VuMax(mMaxDrawCallCount, mDrawCallCount);
	
	mPrimitiveCount = 0;
	mDrawCallCount = 0;
}

//*****************************************************************************
void VuGfx::printStats()
{
	if ( VuDevStat::IF() )
	{
		if ( VuDevStatPage *pPage = VuDevStat::IF()->getCurPage() )
		{
			if ( pPage->getName() == "Gfx" )
			{
				pPage->clear();

				int displayWidth, displayHeight;
				getDisplaySize(VUNULL, displayWidth, displayHeight);
				pPage->printf("Display Size: %dx%d\n", displayWidth, displayHeight);

				pPage->printf("Primitives: max-%dK cur-%dK\n", mMaxPrimitiveCount/1000, mPrimitiveCount/1000);
				pPage->printf("Draw Calls: max-%d cur-%d\n", mMaxDrawCallCount, mDrawCallCount);
			}
		}
	}
}

//*****************************************************************************
int VuGfx::calcVertexCount(VuGfxPrimitiveType primitiveType, int primitiveCount)
{
	static int sLookup[][2] =
	{
		{ 1, 0 },	// VUGFX_PT_POINTLIST		vc = pc
		{ 2, 0 },	// VUGFX_PT_LINELIST		vc = pc*2
		{ 1, 1 },	// VUGFX_PT_LINESTRIP		vc = pc + 1
		{ 3, 0 },	// VUGFX_PT_TRIANGLELIST	vc = pc*3
		{ 1, 2 },	// VUGFX_PT_TRIANGLESTRIP	vc = pc + 2
	};
	VU_COMPILE_TIME_ASSERT(sizeof(sLookup)/sizeof(sLookup[0]) == VUGFX_PRIMITIVE_TYPE_COUNT);

	const int *pEntry = sLookup[primitiveType];
	return primitiveCount*pEntry[0] + pEntry[1];
}
