//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Gfx util
// 
//*****************************************************************************

#pragma once

#include "VuEngine/VuSystemComponent.h"
#include "VuEngine/HAL/Gfx/VuGfxTypes.h"
#include "VuEngine/Math/VuMatrix.h"
#include "VuEngine/Math/VuRect.h"
#include "VuEngine/Util/VuColor.h"
#include "VuEngine/Containers/VuStack.h"
#include "VuEngine/Containers/VuArray.h"
#include "VuEngine/Gfx/GfxSort/VuGfxSort.h"

class VuBasicShaders;
class VuCollisionShader;
class VuDepthShader;
class VuShadowShader;
class VuBlobShadowShader;
class VuDropShadowShader;
class VuPostProcess;
class VuFontDraw;
class VuTexture;
class VuMatrix;
class VuAabb;
class VuDepthStencilState;

struct VuSetRenderTargetParams;


class VuGfxUtil : public VuSystemComponent
{
	DECLARE_SYSTEM_COMPONENT(VuGfxUtil)

protected:
	// called by engine
	friend class VuEngine;
	bool	init();
	void	release();

public:
	VuGfxUtil();
	~VuGfxUtil();

	// sub-systems
	VuBasicShaders		*basicShaders()		{ return mpBasicShaders; }
	VuCollisionShader	*collisionShader()	{ return mpCollisionShader; }
	VuDepthShader		*depthShader()		{ return mpDepthShader; }
	VuShadowShader		*shadowShader()		{ return mpShadowShader; }
	VuBlobShadowShader	*blobShadowShader()	{ return mpBlobShadowShader; }
	VuDropShadowShader	*dropShadowShader()	{ return mpDropShadowShader; }
	VuFontDraw			*fontDraw()			{ return mpFontDraw; }
	VuPostProcess		*postProcess()		{ return mpPostProcess; }
	VuTexture			*whiteTexture()		{ return mpWhiteTexture; }
	VuVertexBuffer		*blackVertexColors(){ return mpBlackVertexColors; }

	// configuration
	bool				getLowTextureLOD()				{ return mLowTextureLOD; }
	bool				getLowModelLOD()				{ return mLowModelLOD; }
	bool				getUltraModelLOD()				{ return mUltraModelLOD; }
	int					getShaderLOD()					{ return mShaderLOD; }

	// default render state
	bool				setDefaultRenderState(); // only call from gfx thread
	VuDepthStencilState	*getDefaultDepthStencilState() { return mpDefaultDepthStencilState; }
	VuDepthStencilState	*getLessEqualDepthStencilState() { return mpLessEqualDepthStencilState; }
	VuDepthStencilState	*getPostProcessDepthStencilState() { return mpPostProcessDepthStencilState; }

	// transform control
	void			pushMatrix(const VuMatrix &mat);
	void			popMatrix();
	const VuMatrix	&getMatrix();

	void			pushTextScale(float scale);
	void			popTextScale();
	float			getTextScale();

	// 2d drawing
	void			drawTexture2d(float depth, VuTexture *pTexture, const VuRect &dstRect = VuRect(0,0,1,1), const VuRect &srcRect = VuRect(0,0,1,1), VUUINT transType = VuGfxSort::TRANS_UI_MODULATE);
	void			drawTexture2d(float depth, VuTexture *pTexture, const VuColor &color, const VuRect &dstRect = VuRect(0,0,1,1), const VuRect &srcRect = VuRect(0,0,1,1), VUUINT transType = VuGfxSort::TRANS_UI_MODULATE);
	void			drawMaskedTexture2d(float depth, VuTexture *pTexture, VuTexture *pMaskTexture, const VuColor &color, const VuRect &dstRect = VuRect(0, 0, 1, 1), const VuRect &srcRect = VuRect(0, 0, 1, 1));
	void			drawFilledRectangle2d(float depth, const VuColor &color, const VuRect &rect = VuRect(0,0,1,1));
	void			drawFilledTriangle2d(float depth, const VuColor &color, const VuVector2 &p0, const VuVector2 &p1, const VuVector2 &p2);
	void			drawRectangleOutline2d(float depth, const VuColor &color, const VuRect &rect = VuRect(0,0,1,1));
	void			drawEllipseOutline2d(float depth, const VuColor &color, const VuRect &rect, int numSegments);
	void			drawLine2d(float depth, const VuColor &color, const VuVector2 &p0, const VuVector2 &p1);
	void			drawLine2d(float depth, const VuVector2 &p0, const VuColor &col0, const VuVector2 &p1, const VuColor &col1);
	void			drawLines2d(float depth, VuGfxPrimitiveType type, const VuColor &color, const VuVector2 *pVerts, int vertCount);
	void			drawAxisInfo(float depth, const VuMatrix &orientation, const VuRect &rect);

	// 3d line drawing
	void			drawLine3d(const VuColor &color, const VuVector3 &p0, const VuVector3 &p1, const VuMatrix &modelViewProjMat);
	void			drawLine3d(const VuVector3 &p0, const VuColor &col0, const VuVector3 &p1, const VuColor &col1, const VuMatrix &modelViewProjMat);
	void			drawLines3d(VuGfxPrimitiveType type, const VuColor &color, const VuVector3 *pVerts, int vertCount, const VuMatrix &modelViewProjMat);
	void			drawAabbLines(const VuColor &color, const VuAabb &aabb, const VuMatrix &modelViewProjMat);
	void			drawCylinderLines(const VuColor &color, float fHeight, float fRadius, int numSides, const VuMatrix &modelViewProjMat);
	void			drawSphereLines(const VuColor &color, float fRadius, int axisSubdivCount, int heightSubdivCount, const VuMatrix &modelViewProjMat);
	void			drawConeLines(const VuColor &color, float fHeight, float fRadius, int numSides, const VuMatrix &modelViewProjMat);
	void			drawArrowLines(const VuColor &color, float fLength, float fHeadLength, float fHeadWidth, const VuMatrix &modelViewProjMat);
	void			drawArcLines(const VuColor &color, const VuVector3 &pos, const VuVector3 &normal, const VuVector3 &axis, float minAngle, float maxAngle, float radius, int segmentCount, bool drawSect, const VuMatrix &modelViewProjMat);

	// 3d solid drawing
	void			drawTriangleStrip(const VuColor &color, const VuVector3 *pVerts, int vertCount, const VuMatrix &modelViewProjMat);
	void			drawAabbSolid(const VuColor &color, const VuAabb &aabb, const VuMatrix &modelMat, const VuMatrix &viewProjMat, const VuVector3 &dirLight = VuVector3(0,0,-1));
	void			drawCylinderSolid(const VuColor &color, float fHeight, float fRadius, int numSides, const VuMatrix &modelMat, const VuMatrix &viewProjMat, const VuVector3 &dirLight = VuVector3(0,0,-1));
	void			drawSphereSolid(const VuColor &color, float fRadius, int axisSubdivCount, int heightSubdivCount, const VuMatrix &modelMat, const VuMatrix &viewProjMat, const VuVector3 &dirLight = VuVector3(0,0,-1));
	void			drawConeSolid(const VuColor &color, float fHeight, float fRadius, int numSides, const VuMatrix &modelMat, const VuMatrix &viewProjMat, const VuVector3 &dirLight = VuVector3(0,0,-1));
	void			drawCapsuleSolid(const VuColor &color, float fHeight, float fRadius, int numSides, const VuMatrix &modelMat, const VuMatrix &viewProjMat, const VuVector3 &dirLight = VuVector3(0,0,-1));

	// misc commands
	void			submitClearCommand(VUUINT32 flags, const VuColor &color = VuColor(0,0,0), float depth = 1.0f, int sequenceNo = 0);
	void			submitSetViewportCommand(const VuRect &rect, int sequenceNo = 0);
	void			submitSetRenderTargetCommand(const VuSetRenderTargetParams &params, int sequenceNo = 0);
	void			clearScreenWithRect(VUUINT32 flags, const VuColor &color, float depth); // only call from gfx thread


	// index buffer to be used for rendering quads (not thread safe! use only from rendering thread)
	const VUUINT16	*getQuadIndexBuffer(int quadCount);

private:
	VuTexture		*createWhiteTexture(int width, int height);
	VuVertexBuffer	*createBlackVertexColors();
	void			growQuadIndexBuffer(int quadCount);

	void				configLowTextureLOD(bool value);
	void				configLowModelLOD(bool value);
	void				configUltraModelLOD(bool value);
	void				configShaderLOD(int value);

	VuBasicShaders		*mpBasicShaders;
	VuCollisionShader	*mpCollisionShader;
	VuDepthShader		*mpDepthShader;
	VuShadowShader		*mpShadowShader;
	VuBlobShadowShader	*mpBlobShadowShader;
	VuDropShadowShader	*mpDropShadowShader;
	VuFontDraw			*mpFontDraw;
	VuPostProcess		*mpPostProcess;
	VuTexture			*mpWhiteTexture;
	VuVertexBuffer		*mpBlackVertexColors;
	VuDepthStencilState	*mpDefaultDepthStencilState;
	VuDepthStencilState	*mpLessEqualDepthStencilState;
	VuDepthStencilState	*mpPostProcessDepthStencilState;
	VuPipelineState		*mpClearColorPipelineState;
	VuPipelineState		*mpClearNoColorPipelineState;
	VuDepthStencilState	*mpClearDepthDepthStencilState;
	VuDepthStencilState	*mpClearNoDepthDepthStencilState;

	// configuration
	bool				mLowTextureLOD;
	bool				mLowModelLOD;
	bool				mUltraModelLOD;
	int					mShaderLOD;

	std::stack<VuMatrix>	mMatrixStack;
	std::stack<float>		mTextScaleStack;

	VuArray<VUUINT16>		mQuadIndexBuffer;
};

inline void VuGfxUtil::drawTexture2d(float depth, VuTexture *pTexture, const VuRect &dstRect, const VuRect &srcRect, VUUINT transType)
{
	drawTexture2d(depth, pTexture, VuColor(255,255,255), dstRect, srcRect, transType);
}
