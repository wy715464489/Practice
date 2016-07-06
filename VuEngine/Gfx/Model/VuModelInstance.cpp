//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Model Instance
// 
//*****************************************************************************

#include "VuModelInstance.h"
#include "VuEngine/HAL/Gfx/VuGfx.h"
#include "VuEngine/Gfx/VuGfxUtil.h"
#include "VuEngine/Gfx/Font/VuFontDraw.h"
#include "VuEngine/Gfx/GfxScene/VuGfxSceneInfo.h"
#include "VuEngine/Gfx/GfxScene/VuGfxSceneMesh.h"
#include "VuEngine/Gfx/GfxScene/VuGfxSceneMeshPart.h"
#include "VuEngine/Gfx/GfxSort/VuGfxSort.h"
#include "VuEngine/Gfx/Camera/VuCamera.h"
#include "VuEngine/Dev/VuDev.h"


//*****************************************************************************
VuModelInstance::VuModelInstance() :
	mColor(255, 255, 255),
	mDynamicLightColor(0, 0, 0),
	mDynamicLightGroupMask(0xffffffff),
	mbTranslucentDepthEnabled(false),
	mWaterZ(-1e9),
	mpMaterialSubstIF(VUNULL)
{
}

//*****************************************************************************
VuModelInstance::~VuModelInstance()
{
}

//*****************************************************************************
void VuModelInstance::drawSceneInfo(const VuMatrix &modelMat, const VuGfxDrawInfoParams &params, const char *text) const
{
	// draw scene info
	if ( params.mFlags & VuGfxDrawInfoParams::SCENE_INFO )
	{
		VuFontDraw *pFontDraw = VuGfxUtil::IF()->fontDraw();
		VuFontDrawParams fdParams;
		fdParams.mSize = 16;
		fdParams.mColor = params.mDevTextColor;

		// set FSL
		int prevFSL = VuGfxSort::IF()->getFullScreenLayer();
		VuGfxSort::IF()->setFullScreenLayer(VuGfxSort::FSL_HUD);

		// background
		int displayWidth, displayHeight;
		VuGfx::IF()->getDisplaySize(VUNULL, displayWidth, displayHeight);
		float displayAspectRatio = (float)displayWidth/displayHeight;
		float width = 14*0.01f*fdParams.mSize/displayAspectRatio;
		float height = 9*0.01f*fdParams.mSize;
		VuRect rect(0.05f, 0.05f, width, height);
		VuGfxUtil::IF()->drawFilledRectangle2d(GFX_SORT_DEPTH_STEP, VuColor(0, 0, 0, 128), rect);

		// text
		pFontDraw->drawString(0, VuDev::IF()->getFont(), text, fdParams, rect, 0);

		// restore FSL
		VuGfxSort::IF()->setFullScreenLayer(prevFSL);
	}
}

//*****************************************************************************
void VuModelInstance::drawName(const char *strName, const VuAabb &aabb, const VuMatrix &worldMat, const VuGfxDrawInfoParams &params) const
{
	VuVector3 vPos(aabb.getCenter());
	vPos = worldMat.transform(vPos);
	vPos = params.mCamera.worldToScreen(vPos);

	if ( vPos.mZ >= 0 && vPos.mZ <= 1 )
	{
		if ( vPos.mX >= 0 && vPos.mX <= 1 && vPos.mY >= 0 && vPos.mY <= 1 )
		{
			VuRect rect(vPos.mX, vPos.mY, 0.0f, 0.0f);

			VuFontDraw *pFontDraw = VuGfxUtil::IF()->fontDraw();
			VuFontDrawParams fdParams;
			fdParams.mSize = 16;
			fdParams.mWeight = 110;
			fdParams.mSoftness = 0;
			fdParams.mColor = params.mDevTextColor;
			fdParams.mOutlineWeight = 5;
			fdParams.mOutlineSoftness = 0;

			pFontDraw->drawString(vPos.mZ, VuDev::IF()->getFont(), strName, fdParams, rect, VUGFX_TEXT_DRAW_HCENTER|VUGFX_TEXT_DRAW_VCENTER);
		}
	}
}

//*****************************************************************************
void VuModelInstance::drawMeshInfo(VuGfxSceneMesh *pMesh, const VuMatrix &transform, const VuGfxDrawInfoParams &params) const
{
	// draw mesh name
	if ( params.mFlags & VuGfxDrawInfoParams::MESH_NAMES )
		drawName(pMesh->mstrName.c_str(), pMesh->getAabb(), transform, params);

	// draw mesh bounds
	if ( params.mFlags & VuGfxDrawInfoParams::MESH_BOUNDS )
		VuGfxUtil::IF()->drawAabbLines(params.mDevLineColor, pMesh->getAabb(), transform*params.mCamera.getViewProjMatrix());

	// draw mesh part bounds
	if ( params.mFlags & VuGfxDrawInfoParams::MESH_PART_BOUNDS )
		for ( VuGfxSceneMesh::Parts::iterator iter = pMesh->mParts.begin(); iter != pMesh->mParts.end(); iter++ )
			if ( params.mCamera.isAabbVisible((*iter)->mAabb, transform) )
				VuGfxUtil::IF()->drawAabbLines(params.mDevLineColor, (*iter)->mAabb, transform*params.mCamera.getViewProjMatrix());
}
