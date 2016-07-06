//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Water texture implementation
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Water/VuWaterRenderer.h"
#include "VuEngine/HAL/Gfx/VuGfxTypes.h"

class VuTexture;
class VuPackedVector2;


class VuWaterTexture 
{
public:
	VuWaterTexture();
	~VuWaterTexture();

	void			setDesc(const VuWaterRendererTextureDesc &desc)	{ mSimDesc = desc; }
	VuTexture		*getActiveTexture()								{ return mpTextures[mActiveTexture]; }

	const VuWaterRendererTextureDesc	&getGfxDesc() { return mGfxDesc; }

private:
	void			tick(float fdt);
	void			submit();
	void			draw();
	void			initializeFFT();
	void			calculateDispersion();
	void			calculateInitialFourierAmplitudes();
	void			updateFFT();
	void			calculateCurrentFourierAmplitudes();
	void			calculateHeights();
	void			calculateNormals();
	void			buildMipLevel(int level);
	void			writeNormalsToTexture(int level, VuTexture *pTexture);

	VuGfxFormat		mFormat;

	VuWaterRendererTextureDesc	mSimDesc;
	VuWaterRendererTextureDesc	mGfxDesc;

	enum { TEXTURE_COUNT = 2 };
	VuTexture		*mpTextures[TEXTURE_COUNT];
	int				mActiveTexture;

	float			*mpDispersion;
	VuPackedVector2	*mpFourierAmplitudes;
	float			***mpFFTData;
	float			**mpFFTSpeq;
	float			*mpHeights;
	VUUINT16		**mpNormals;

	bool			mbShowTexture;
	double			mAge;
};
