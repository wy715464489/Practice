//*****************************************************************************
//
//  Copyright (c) 2005-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Interface class to Debug library.
// 
//*****************************************************************************

#include <stdio.h>
#include <stdarg.h>
#include "VuDev.h"

#include "VuEngine/VuEngine.h"
#include "VuEngine/HAL/Gfx/VuGfx.h"
#include "VuEngine/HAL/File/VuFile.h"
#include "VuEngine/Gfx/Font/VuFontDraw.h"
#include "VuEngine/Gfx/Font/VuFont.h"
#include "VuEngine/Gfx/VuGfxUtil.h"
#include "VuEngine/Gfx/GfxSort/VuGfxSort.h"
#include "VuEngine/Gfx/Shaders/VuBasicShaders.h"
#include "VuEngine/Managers/VuDrawManager.h"
#include "VuEngine/Managers/VuViewportManager.h"
#include "VuEngine/Json/VuJsonContainer.h"
#include "VuEngine/Json/VuJsonReader.h"
#include "VuEngine/Json/VuJsonWriter.h"
#include "VuEngine/Util/VuBinaryDataUtil.h"
#include "VuEngine/Util/VuStringUtil.h"
#include "VuEngine/Dev/VuDevConfig.h"
#include "VuEngine/Dev/VuDevMenu.h"


#if VU_DISABLE_DEV

// the interface
IMPLEMENT_SYSTEM_COMPONENT(VuDev, VuDev);

#else

extern VUBYTE gDevFontData[];
extern int gDevFontDataSize;

// defines
#define MAX_NUM_DEBUG_STRINGS 256
#define MAX_SIZE_DEBUG_STRING 256


struct DebugString
{
	VuVector3 mPos;
	int mFlags;
	VuColor mColor;
	char mStr[MAX_SIZE_DEBUG_STRING];
};
class VuDevImpl : public VuDev
{
public:
	VuDevImpl();

	virtual bool					init();
	virtual void					postInit();
	virtual void					release();
	virtual void					draw();

	virtual VuFont					*getFont()	{ return mpFont; }
	virtual void					printf(const VuVector3 &pos, int flags, const VuColor &color, const char *fmt, ...);
	virtual void					drawLine(const VuVector3 &v0, const VuVector3 &v1, const VuColor &color);
	virtual void					drawLine(const VuVector3 &v0, const VuColor &c0, const VuVector3 &v1, const VuColor &c1);
	virtual void					drawBox(const VuMatrix &mat, const VuColor &color);
	virtual void					drawSphere(const VuVector3 &vPos,
											float fRadius,
											const VuColor &color,
											int axisSubdivCount,
											int heightSubdivCount);
	virtual void					drawCylinder(const VuColor &color, float fHeight, float fRadius, int numSides, const VuMatrix &matModel);
	virtual void					drawArrow(const VuColor &color, float fLength, float fHeadLength, float fHeadWidth, const VuMatrix &matModel);
	virtual void					drawArc(const VuVector3 &pos, 
											const VuVector3 &axis, 
											const VuVector3 &right, 
											float angle, 
											float radius, 
											int segmentCount, 
											const VuColor &color);
	virtual void					printJsonContainer(const VuJsonContainer &container) const;

	virtual const std::string		&getBuildNumber() const;
	virtual void					setBuildNumber(int buildNumber);

private:
	inline bool		isLineVisible(const VuVector3 &v0, const VuVector3 &v1);

	typedef VuArray<DebugString> Strings;
	typedef VuArray<VuVertex3dXyzCol> LineVerts;

	VuFont			*mpFont;
	Strings			mStrings;
	LineVerts		mLineVerts;
	std::string		mBuildNumber;
	bool			mbDrawBuildNumber;
};

// the interface
IMPLEMENT_SYSTEM_COMPONENT(VuDev, VuDevImpl);


//*****************************************************************************
VuDevImpl::VuDevImpl():
	mpFont(VUNULL)
{
	mbDrawBuildNumber = VuEngine::IF()->options().mbDevDrawBuildNumber;
}

//*****************************************************************************
bool VuDevImpl::init()
{
	// load dev font from static data
	mpFont = new VuFont;
	VuBinaryDataReader reader(gDevFontData, gDevFontDataSize);
	if ( !mpFont->load(reader) )
		return VUERROR("Failed to create dev font.");

	// start rendering
	VuDrawManager::IF()->registerHandler(this, &VuDevImpl::draw);

	// reserve some space
	mLineVerts.reserve(2048);
	mStrings.reserve(256);

	return true;
}

//*****************************************************************************
void VuDevImpl::postInit()
{
	if ( VuDevMenu::IF() )
		VuDevMenu::IF()->addBool("Dev/DrawBuildNumber", mbDrawBuildNumber);
}

//*****************************************************************************
void VuDevImpl::release()
{
	// stop rendering
	VuDrawManager::IF()->unregisterHandler(this);

	delete mpFont;
}

//*****************************************************************************
void VuDevImpl::draw()
{
	VuGfxSort::IF()->setFullScreenLayer(VuGfxSort::FSL_GAME);
	VuGfxSort::IF()->setReflectionLayer(VuGfxSort::REFLECTION_OFF);

	// draw debug strings
	if ( mStrings.size() )
	{
		VuGfxSort::IF()->setViewportLayer(VuGfxSort::VPL_HUD);

		for ( int iViewport = 0; iViewport < VuViewportManager::IF()->getViewportCount(); iViewport++ )
		{
			VuGfxSort::IF()->setViewport(iViewport);

			const VuCamera &camera = VuViewportManager::IF()->getViewport(iViewport).mCamera;

			VuFontDrawParams fdParams;
			for ( DebugString *pDS = &mStrings.begin(); pDS != &mStrings.end(); pDS++ )
			{
				fdParams.mColor = pDS->mColor;
				VuVector3 screenPos = camera.worldToScreen(pDS->mPos);
				if ( screenPos.mZ > 0 && screenPos.mZ < 1 )
					VuGfxUtil::IF()->fontDraw()->drawString(0, VuDev::IF()->getFont(), pDS->mStr, fdParams, VuRect(screenPos.mX, screenPos.mY, 0.0f, 0.0f), pDS->mFlags);
			}
		}

		mStrings.clear();
	}

	// draw debug lines
	if ( mLineVerts.size() )
	{
		VuGfxSort::IF()->setViewportLayer(VuGfxSort::VPL_WORLD);

		struct DrawData
		{
			static void callback(void *data)
			{
				DrawData *pData = static_cast<DrawData *>(data);
				void *pVerts = pData + 1;

				VuGfxUtil::IF()->basicShaders()->set3dXyzColConstants(pData->mViewProjMatrix);

				VuGfx::IF()->setTexture(0, VUNULL);
				VuGfx::IF()->drawPrimitiveUP(VUGFX_PT_LINELIST, pData->mVertCount/2, pVerts);
			}

			VuMatrix	mViewProjMatrix;
			int			mVertCount;
		};

		for ( int iViewport = 0; iViewport < VuViewportManager::IF()->getViewportCount(); iViewport++ )
		{
			VuGfxSort::IF()->setViewport(iViewport);

			// do this in batches of 65534 verts (Xbox limit is 65535 verts)
			int vertCount = mLineVerts.size();
			int batchCount = 1 + vertCount/65535;
			int vertStart = 0;
			for ( int iBatch = 0; iBatch < batchCount; iBatch++ )
			{
				int curCount = VuMin(vertCount, 65534);

				int size = sizeof(DrawData) + curCount*sizeof(mLineVerts[0]);

				DrawData *pData = static_cast<DrawData *>(VuGfxSort::IF()->allocateCommandMemory(size));
				void *pVerts = pData + 1;

				pData->mViewProjMatrix = VuViewportManager::IF()->getViewport(iViewport).mCamera.getViewProjMatrix();
				pData->mVertCount = curCount;
				VU_MEMCPY(pVerts, curCount*sizeof(mLineVerts[0]), &mLineVerts[vertStart], curCount*sizeof(mLineVerts[0]));

				VuGfxSort::IF()->submitDrawCommand<false>(VuGfxSort::TRANS_OPAQUE, VuGfxUtil::IF()->basicShaders()->get3dXyzColMaterial(VuBasicShaders::FLV_OPAQUE), VUNULL, &DrawData::callback);

				vertCount -= curCount;
				vertStart += curCount;
			}
		}

		mLineVerts.clear();
	}

	VuGfxSort::IF()->setViewport(0);

	// draw build #
	if ( mbDrawBuildNumber )
	{
		VuGfxSort::IF()->setFullScreenLayer(VuGfxSort::FSL_END);
		VuGfxSort::IF()->setViewportLayer(VuGfxSort::VPL_END);

		VuFontDrawParams fdParams;
		fdParams.mSize = 22;
		fdParams.mWeight = 130;
		fdParams.mOutlineWeight = 2;

		VuGfxUtil::IF()->fontDraw()->drawString(0, VuDev::IF()->getFont(), mBuildNumber.c_str(), fdParams, VuRect(0.05f, 0.05f, 0.9f, 0.9f), VUGFX_TEXT_DRAW_RIGHT|VUGFX_TEXT_DRAW_TOP);
	}
}

//*****************************************************************************
void VuDevImpl::printf(const VuVector3 &pos, int flags, const VuColor &color, const char *fmt, ...)
{
	mStrings.resize(mStrings.size() + 1);
	DebugString *pDS = &mStrings.back();
	pDS->mPos = pos;
	pDS->mFlags = flags;
	pDS->mColor = color;

	va_list args;
	va_start(args, fmt);
	VU_VSNPRINTF(pDS->mStr, sizeof(pDS->mStr), sizeof(pDS->mStr) - 1, fmt, args);
	va_end(args);
	pDS->mStr[sizeof(pDS->mStr)-1] = '\0';
}

//*****************************************************************************
void VuDevImpl::drawLine(const VuVector3 &v0, const VuVector3 &v1, const VuColor &color)
{
	if ( isLineVisible(v0, v1) )
	{
		mLineVerts.resize(mLineVerts.size() + 2);

		VuVertex3dXyzCol *pv0 = &mLineVerts.back() - 1;
		VuVertex3dXyzCol *pv1 = &mLineVerts.back();

		pv0->mXyz[0] = v0.mX; pv0->mXyz[1] = v0.mY; pv0->mXyz[2] = v0.mZ; pv0->mColor = color;
		pv1->mXyz[0] = v1.mX; pv1->mXyz[1] = v1.mY; pv1->mXyz[2] = v1.mZ; pv1->mColor = color;
	}
}

//*****************************************************************************
void VuDevImpl::drawLine(const VuVector3 &v0, const VuColor &c0, const VuVector3 &v1, const VuColor &c1)
{
	if ( isLineVisible(v0, v1) )
	{
		mLineVerts.resize(mLineVerts.size() + 2);

		VuVertex3dXyzCol *pv0 = &mLineVerts.back() - 1;
		VuVertex3dXyzCol *pv1 = &mLineVerts.back();

		pv0->mXyz[0] = v0.mX; pv0->mXyz[1] = v0.mY; pv0->mXyz[2] = v0.mZ; pv0->mColor = c0;
		pv1->mXyz[0] = v1.mX; pv1->mXyz[1] = v1.mY; pv1->mXyz[2] = v1.mZ; pv1->mColor = c1;
	}
}

//*****************************************************************************
void VuDevImpl::drawBox(const VuMatrix &mat, const VuColor &color)
{
	VuVector3 v0 = mat.getTrans() - 0.5f*mat.getAxisX() - 0.5f*mat.getAxisY() - 0.5f*mat.getAxisZ();
	VuVector3 v1 = mat.getTrans() + 0.5f*mat.getAxisX() - 0.5f*mat.getAxisY() - 0.5f*mat.getAxisZ();
	VuVector3 v2 = mat.getTrans() - 0.5f*mat.getAxisX() + 0.5f*mat.getAxisY() - 0.5f*mat.getAxisZ();
	VuVector3 v3 = mat.getTrans() + 0.5f*mat.getAxisX() + 0.5f*mat.getAxisY() - 0.5f*mat.getAxisZ();
	VuVector3 v4 = mat.getTrans() - 0.5f*mat.getAxisX() - 0.5f*mat.getAxisY() + 0.5f*mat.getAxisZ();
	VuVector3 v5 = mat.getTrans() + 0.5f*mat.getAxisX() - 0.5f*mat.getAxisY() + 0.5f*mat.getAxisZ();
	VuVector3 v6 = mat.getTrans() - 0.5f*mat.getAxisX() + 0.5f*mat.getAxisY() + 0.5f*mat.getAxisZ();
	VuVector3 v7 = mat.getTrans() + 0.5f*mat.getAxisX() + 0.5f*mat.getAxisY() + 0.5f*mat.getAxisZ();

	drawLine(v0, v1, color);
	drawLine(v2, v3, color);
	drawLine(v0, v2, color);
	drawLine(v1, v3, color);
	drawLine(v0, v4, color);
	drawLine(v1, v5, color);
	drawLine(v2, v6, color);
	drawLine(v3, v7, color);
	drawLine(v4, v5, color);
	drawLine(v6, v7, color);
	drawLine(v4, v6, color);
	drawLine(v5, v7, color);
}

//*****************************************************************************
void VuDevImpl::drawSphere(	const VuVector3 &vPos,
							float fRadius,
							const VuColor &color,
							int axisSubdivCount,
							int heightSubdivCount)
{
	// draw horizontal lines (latitudes)
	for ( int i = 1; i < heightSubdivCount; i++ )
	{
		float thi = VU_PI*i/heightSubdivCount;
		float sinThi, cosThi;
		VuSinCos(thi, sinThi, cosThi);
		float theta = 0, step = VU_2PI/axisSubdivCount;
		for ( int j = 0; j < axisSubdivCount; j++ )
		{
			VuVector3 v0(vPos);
			v0.mX += fRadius*sinThi*VuCos(theta);
			v0.mY += fRadius*sinThi*VuSin(theta);
			v0.mZ += fRadius*cosThi;

			VuVector3 v1(vPos);
			v1.mX += fRadius*sinThi*VuCos(theta + step);
			v1.mY += fRadius*sinThi*VuSin(theta + step);
			v1.mZ += fRadius*cosThi;

			drawLine(v0, v1, color);

			theta += step;
		}
	}

	// draw vertical lines (longitudes)
	for ( int i = 0; i < axisSubdivCount; i++ )
	{
		float theta = VU_2PI*i/axisSubdivCount;
		float sinTheta, cosTheta;
		VuSinCos(theta, sinTheta, cosTheta);
		float thi = 0, step = VU_PI/heightSubdivCount;
		for ( int j = 0; j < heightSubdivCount; j++ )
		{
			VuVector3 v0(vPos);
			v0.mX += fRadius*VuSin(thi)*cosTheta;
			v0.mY += fRadius*VuSin(thi)*sinTheta;
			v0.mZ += fRadius*VuCos(thi);

			VuVector3 v1(vPos);
			v1.mX += fRadius*VuSin(thi + step)*cosTheta;
			v1.mY += fRadius*VuSin(thi + step)*sinTheta;
			v1.mZ += fRadius*VuCos(thi + step);

			drawLine(v0, v1, color);

			thi += step;
		}
	}
}

//*****************************************************************************
void VuDevImpl::drawCylinder(const VuColor &color, float fHeight, float fRadius, int numSides, const VuMatrix &matModel)
{
	float fStep = 2.0f*VU_PI/numSides;
	float fCurAngle = 0.0f;
	float fNextAngle = fStep;
	for ( int iSide = 0; iSide < numSides; iSide++ )
	{
		VuVector2 v0 = fRadius*VuVector2(VuCos(fCurAngle), VuSin(fCurAngle));
		VuVector2 v1 = fRadius*VuVector2(VuCos(fNextAngle), VuSin(fNextAngle));

		VuVector3 verts[4];
		verts[0] = VuVector3(v0.mX, v0.mY, -0.5f*fHeight);
		verts[1] = VuVector3(v1.mX, v1.mY, -0.5f*fHeight);
		verts[2] = VuVector3(v1.mX, v1.mY,  0.5f*fHeight);
		verts[3] = VuVector3(v0.mX, v0.mY,  0.5f*fHeight);

		for ( int i = 0; i < 4; i++ )
			verts[i] = matModel.transform(verts[i]);

		drawLine(verts[0], verts[1], color);
		drawLine(verts[1], verts[2], color);
		drawLine(verts[2], verts[3], color);

		fCurAngle = fNextAngle;
		fNextAngle += fStep;
	}
}

//*****************************************************************************
void VuDevImpl::drawArrow(const VuColor &color, float fLength, float fHeadLength, float fHeadWidth, const VuMatrix &matModel)
{
	VuVector3 verts[5];

	verts[0] = VuVector3(0,           0,                     0); // base
	verts[1] = VuVector3(0,           fLength,               0); // tip
	verts[2] = VuVector3(0,           fLength - fHeadLength, 0); // head base
	verts[3] = VuVector3(-fHeadWidth, fLength - fHeadLength, 0); // head left
	verts[4] = VuVector3( fHeadWidth, fLength - fHeadLength, 0); // head right

	for ( int i = 0; i < 5; i++ )
		verts[i] = matModel.transform(verts[i]);

	drawLine(verts[0], verts[2], color); // shaft
	drawLine(verts[3], verts[4], color); // head base
	drawLine(verts[1], verts[3], color); // head left
	drawLine(verts[1], verts[4], color); // head right
}

//*****************************************************************************
void VuDevImpl::drawArc(const VuVector3 &pos, 
						const VuVector3 &axis, 
						const VuVector3 &right, 
						float angle, 
						float radius, 
						int segmentCount, 
						const VuColor &color)
{
	VuVector3 fwd = VuCross(axis, right).normal();

	VuMatrix mat;
	mat.loadIdentity();
	mat.setAxisX(right);
	mat.setAxisY(fwd);
	mat.setAxisZ(axis);
	mat.setTrans(pos);

	VuVector3 prevPos = mat.transform(VuVector3(radius, 0.0f, 0.0f));
	float radiansPerSegment = angle / segmentCount;

	for(int iSegment = 0; iSegment < segmentCount; iSegment++)
	{
		mat.rotateZLocal(radiansPerSegment);

		VuVector3 pos = mat.transform(VuVector3(radius, 0.0f, 0.0f));

		drawLine(prevPos, pos, color);
		
		prevPos = pos;
	}
}

//*****************************************************************************
void VuDevImpl::printJsonContainer(const VuJsonContainer &container) const
{
	VuJsonWriter writer;
	std::string str;

	writer.saveToString(container, str);

	VUUINT len = (VUUINT)str.length();
	VUUINT offset = 0;

	while(len > 0)
	{
		VUUINT chars = VuMin(len, (VUUINT)512);

		VUPRINTF("%s", str.substr(offset, chars).c_str());
		
		len -= chars;
		offset += chars;
	}

	VUPRINTF("\n");
}

//*****************************************************************************
const std::string &VuDevImpl::getBuildNumber() const
{
	return mBuildNumber;
}

//*****************************************************************************
void VuDevImpl::setBuildNumber(int buildNumber)
{
	char strBuildNumber[64];
	VuStringUtil::buildNumberFormat(buildNumber, strBuildNumber, sizeof(strBuildNumber));

	VUPRINTF("BUILD #: %s\n", strBuildNumber);

	mBuildNumber = strBuildNumber;
}

//*****************************************************************************
inline bool VuDevImpl::isLineVisible(const VuVector3 &v0, const VuVector3 &v1)
{
	VuVector3 vc = 0.5f*(v0 + v1);
	float radius = (v1 - v0).mag();

	// simple culling
	for ( int iViewport = 0; iViewport < VuViewportManager::IF()->getViewportCount(); iViewport++ )
		if ( VuViewportManager::IF()->getViewport(iViewport).mCamera.isSphereVisible(vc, radius) )
			return true;

	return false;
}

#endif // VU_DISABLE_DEV
