//*****************************************************************************
//
//  Copyright (c) 2006-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Basic shaders.
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Util/VuColor.h"

class VuMatrix;
class VuVector3;
class VuShaderProgram;
class VuGfxSortMaterial;
class VuVertexDeclaration;
class VuBasicShadersImpl;
class VuTexture;
class VuCompiledShaderAsset;
class VuVertexDeclarationParams;


// vertex declarations
struct VuVertex2dXyz
{
	float		mXyz[3];
};
struct VuVertex2dXyzUv
{
	float		mXyz[3];
	float		mUv[2];
};
struct VuVertex2dXyzCol
{
	float		mXyz[3];
	VUUINT32	mColor;
};
struct VuVertex3dXyz
{
	float		mXyz[3];
};
struct VuVertex3dXyzUv
{
	float		mXyz[3];
	float		mUv[2];
};
struct VuVertex3dXyzNor
{
	float		mXyz[3];
	float		mNor[3];
};
struct VuVertex3dXyzCol
{
	float		mXyz[3];
	VUUINT32	mColor;
};


class VuBasicShaders
{
public:
	bool	init();
	void	release();

	enum eFlavor { FLV_OPAQUE, FLV_MODULATED, FLV_ADDITIVE, FLV_DEPTH, NUM_FLAVORS };

	// 2d interface

	VuGfxSortMaterial	*get2dXyzMaterial(eFlavor flavor);
	void				set2dXyzConstants(const VuMatrix &transform, const VuColor &color = VuColor(255,255,255));

	VuGfxSortMaterial	*get2dXyzUvMaterial(eFlavor flavor);
	void				set2dXyzUvConstants(const VuMatrix &transform, const VuColor &color = VuColor(255,255,255));
	void				set2dXyzUvTexture(VuTexture *pTexture);

	VuGfxSortMaterial	*get2dXyzColMaterial(eFlavor flavor);
	void				set2dXyzColConstants(const VuMatrix &transform);

	VuGfxSortMaterial	*get2dXyzUvMaskMaterial(eFlavor flavor);
	void				set2dXyzUvMaskConstants(const VuMatrix &transform, const VuColor &color = VuColor(255, 255, 255));
	void				set2dXyzUvMaskTextures(VuTexture *pTexture, VuTexture *pMaskTexture);


	// 3d interface

	VuGfxSortMaterial	*get3dXyzMaterial(eFlavor flavor);
	void				set3dXyzConstants(const VuMatrix &modelViewProjMatrix, const VuColor &color = VuColor(255,255,255));

	VuGfxSortMaterial	*get3dXyzUvMaterial(eFlavor flavor);
	void				set3dXyzUvConstants(const VuMatrix &modelViewProjMatrix, const VuColor &color = VuColor(255,255,255));
	void				set3dXyzUvTexture(VuTexture *pTexture);

	VuGfxSortMaterial	*get3dXyzColMaterial(eFlavor flavor);
	void				set3dXyzColConstants(const VuMatrix &modelViewProjMatrix);

	VuGfxSortMaterial	*get3dXyzNorMaterial(eFlavor flavor);
	void				set3dXyzNorConstants(const VuMatrix &modelMatrix, const VuMatrix &viewProjMatrix, const VuVector3 &dirLightWorld, const VuColor &color = VuColor(255,255,255));

private:
	class VuBasicShader
	{
	public:
		VuShaderProgram		*mpShaderProgram;
		VuGfxSortMaterial	*mpMaterials[NUM_FLAVORS];

		bool create(const char *strShaderAsset, const VuVertexDeclarationParams &params);
		void release();
	};

	VuBasicShader	m2dXyz;
	struct Consts2dXyz
	{
		VUHANDLE	mhColor;
		VUHANDLE	mhTransform;
	};
	Consts2dXyz		mConsts2dXyz;

	VuBasicShader	m2dXyzUv;
	struct Consts2dXyzUv
	{
		VUHANDLE	mhColor;
		VUHANDLE	mhTransform;
		int			miTexture;
	};
	Consts2dXyzUv	mConsts2dXyzUv;

	VuBasicShader	m2dXyzCol;
	struct Consts2dXyzCol
	{
		VUHANDLE	mhTransform;
	};
	Consts2dXyzCol	mConsts2dXyzCol;

	VuBasicShader	m2dXyzUvMask;
	struct Consts2dXyzUvMask
	{
		VUHANDLE	mhColor;
		VUHANDLE	mhTransform;
		int			miTexture;
		int			miMaskTexture;
	};
	Consts2dXyzUvMask	mConsts2dXyzUvMask;

	VuBasicShader	m3dXyz;
	struct Consts3dXyz
	{
		VUHANDLE	mhColor;
		VUHANDLE	mhModelViewProjMatrix;
	};
	Consts3dXyz		mConsts3dXyz;

	VuBasicShader	m3dXyzUv;
	struct Consts3dXyzUv
	{
		VUHANDLE	mhColor;
		VUHANDLE	mhModelViewProjMatrix;
		int			miTexture;
	};
	Consts3dXyzUv	mConsts3dXyzUv;

	VuBasicShader	m3dXyzCol;
	struct Consts3dXyzCol
	{
		VUHANDLE	mhModelViewProjMatrix;
	};
	Consts3dXyzCol	mConsts3dXyzCol;

	VuBasicShader	m3dXyzNor;
	struct Consts3dXyzNor
	{
		VUHANDLE	mhDirLightWorld;
		VUHANDLE	mhColor;
		VUHANDLE	mhModelViewProjMatrix;
		VUHANDLE	mhModelMatrix;
	};
	Consts3dXyzNor	mConsts3dXyzNor;
};
