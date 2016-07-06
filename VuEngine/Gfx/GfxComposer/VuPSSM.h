//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  PSSM implementation class
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Components/3dDraw/Vu3dDrawManager.h"
#include "VuEngine/Math/VuMatrix.h"
#include "VuEngine/Containers/VuArray.h"

class VuShaderProgram;
class VuPipelineState;
class VuCamera;
class VuTexture;
class VuMatrix;
class VuShadowRenderTarget;
class VuAabb;


class VuPSSM
{
public:
	VuPSSM();
	~VuPSSM();

	void				setCount(int count)					{ mCount = count; }
	void				setSize(int size)					{ mSize = size; }
	void				setRejectionScale(float fScale)		{ mRejectionScale = fScale; }
	void				setSplitPositions(float split1, float split2 = 0, float split3 = 0);

	void				submitCommands(const VuCamera &camera, VUUINT32 zoneMask, bool bReflection, const VuVector4 &reflectionPlane = VuVector4(0,0,0,0));

	int					getTextureCount()		{ return mCount; }
	VuTexture			*getTexture(int index);

	struct VuMatExt
	{
		VUHANDLE				mhConstShadowTextureMatrices;
		VUHANDLE				mhConstShadowMapSize;
		VUHANDLE				mhConstShadowMapTexelSize;
		VUHANDLE				mhConstShadowMapSplits;
		int						miSampShadowMaps[4];
		int						mShadowMapCount;
	};
	static void			resolveConstants(VuShaderProgram *pSP, VuMatExt *pMatExt);
	static void			setConstants(VuShaderProgram *pSP, const VuMatExt *pMatExt);

private:
	typedef VuArray<VuMatrix> TextureMatrices;
	typedef VuArray<float> Splits;

	void				updateResources();
	void				destroyResources();

	void				updateSplitRatios(const VuCamera &camera);
	void				calcLightMatrix(const VuVector3 &lightPos, const VuVector3 &lightDir, VuMatrix &lightMatrix);
	void				calcLightAabb(int iSplit, const VuCamera &camera, const VuMatrix &lightMatrix, VuAabb &lightAabb);
	void				calcLightCropMatrix(int iSplit, const VuAabb &lightAabb, const VuMatrix &lightMatrix, VuMatrix &lightCropMatrix);
	void				calcTextScaleBiasMatrix(int iSplit, VuMatrix &texScaleBiasMatrix);
	void				submitRenderConstants();

	int						mCount;
	int						mSize;
	float					mRejectionScale;
	VuShadowRenderTarget	*mpShadowRenderTarget;
	TextureMatrices			mTextureMatrices;
	Splits					mSplitPositions;
	Splits					mSplitRatios;
};
