//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Water ocean wave class
// 
//*****************************************************************************

#pragma once

#include "VuWaterWave.h"
#include "VuEngine/Math/VuVector3.h"
#include "VuEngine/Math/VuPatch.h"


//*****************************************************************************
// Internal classes
//*****************************************************************************
class VuOceanWavePatchInfo
{
public:
	int		mIndexI[4];
	int		mIndexO[8];
};

class VuOceanWaveBuffer
{
public:
	VuOceanWaveBuffer() : mBufferAge(0), mpHeights(VUNULL), mpPatches(VUNULL) {}

	void				copyFrom(const VuOceanWaveBuffer &other, int gridDim);

	void				allocateResources(int gridDim);
	void				freeResources();

	double				mBufferAge;
	float				*mpHeights;
	VuPatch<float>		*mpPatches;
};


//*****************************************************************************
// Base ocean wave.  All ocean waves are derived from this.
//*****************************************************************************
class VuWaterBaseOceanWaveDesc
{
public:
	VuWaterBaseOceanWaveDesc() { memset(this, 0, sizeof(*this)); }

	VUUINT		mComplexity;
	float		mBinSize;
	float		mWaveDirection;
	float		mGravity;
	float		mWindSpeed;
	float		mDirectionalPower;
	float		mSuppressionWaveLength;
	float		mHeightMultiplier;
};

class VuWaterBaseOceanWave : public VuWaterWave
{
	DECLARE_RTTI

protected:
	VuWaterBaseOceanWave(const VuWaterBaseOceanWaveDesc &desc);
	~VuWaterBaseOceanWave();

public:
	// VuWaterWave interface
	virtual bool				tick(float fdt);
	void						modifyHeightMultiplier(float heightMultiplier);

protected:
	void						setDesc(const VuWaterBaseOceanWaveDesc &desc);
	void						allocateResources();
	void						freeResources();
	void						buildPatchInfo();
	void						calculateDispersion();
	void						calculateInitialFourierAmplitudes();
	void						updateFFT();
	void						calculateCurrentFourierAmplitudes();
	void						calculateHeights();
	void						buildPatches();
	int							getPatchIndex(float x, float y, float &u, float &v);

	enum { BUFFER_COUNT = 2 };

	VuWaterBaseOceanWaveDesc	mDesc;
	int							mGridPow;
	int							mGridDim;
	int							mGridBits;
	float						mGridResolution;
	float						mFFTWaveDirection;
	float						mSinFFTWaveDirection;
	float						mCosFFTWaveDirection;
	double						mAge;
	float						*mpDispersion;
	VuPackedVector2				*mpFourierAmplitudes;
	float						***mpFFTData;
	float						**mpFFTSpeq;
	VuOceanWavePatchInfo		*mpPatchInfo;
	int							mCurBuffer;
	int							mLastBuffer;
	VuOceanWaveBuffer			mBuffers[BUFFER_COUNT];
};


//*****************************************************************************
// Infinite ocean wave.  This ocean wave is not bounded.
//*****************************************************************************
class VuWaterInfiniteOceanWaveDesc : public VuWaterBaseOceanWaveDesc
{
public:
	VuWaterInfiniteOceanWaveDesc() { memset(this, 0, sizeof(*this)); }
};

class VuWaterInfiniteOceanWave : public VuWaterBaseOceanWave
{
	DECLARE_RTTI

public:
	VuWaterInfiniteOceanWave(const VuWaterInfiniteOceanWaveDesc &desc);

	void			modify(const VuWaterInfiniteOceanWaveDesc &desc);

	// VuWaterWave interface
	virtual void	getSurfaceData(VuWaterSurfaceDataParams &params);

private:
	template<int VERTEX_TYPE, int CLIP_TYPE>
	void						getSurfaceData(VuWaterSurfaceDataParams &params);

protected:
	~VuWaterInfiniteOceanWave() {}
};


//*****************************************************************************
// Rectangular ocean wave.  This ocean wave is bounded by a rectangle.
//*****************************************************************************
class VuWaterRectangularOceanWaveDesc : public VuWaterBaseOceanWaveDesc
{
public:
	VuWaterRectangularOceanWaveDesc() { memset(this, 0, sizeof(*this)); }

	VuVector3	mPosition;
	float		mSizeX;
	float		mSizeY;
	float		mDecayRatioX;
	float		mDecayRatioY;
};

class VuWaterRectangularOceanWave : public VuWaterBaseOceanWave
{
	DECLARE_RTTI

public:
	VuWaterRectangularOceanWave(const VuWaterRectangularOceanWaveDesc &desc);

	void			modify(const VuWaterRectangularOceanWaveDesc &desc);

	// VuWaterWave interface
	virtual void	getSurfaceData(VuWaterSurfaceDataParams &params);
	virtual void	debugDraw2d();

private:
	void			updateBounds();

	template<int VERTEX_TYPE, int CLIP_TYPE>
	void			getSurfaceData(VuWaterSurfaceDataParams &params);

	VuVector3		mPosition;
	float			mSizeX;
	float			mSizeY;
	float			mDecayRatioX;
	float			mDecayRatioY;

protected:
	~VuWaterRectangularOceanWave() {}
};


//*****************************************************************************
// Circular ocean wave.  This ocean wave is bounded by a circle.
//*****************************************************************************
class VuWaterCircularOceanWaveDesc : public VuWaterBaseOceanWaveDesc
{
public:
	VuWaterCircularOceanWaveDesc() { memset(this, 0, sizeof(*this)); }

	VuVector3	mPosition;
	float		mRadius;
	float		mDecayRatio;
};

class VuWaterCircularOceanWave : public VuWaterBaseOceanWave
{
	DECLARE_RTTI

public:
	VuWaterCircularOceanWave(const VuWaterCircularOceanWaveDesc &desc);

	void			modify(const VuWaterCircularOceanWaveDesc &desc);

	// VuWaterWave interface
	virtual void	getSurfaceData(VuWaterSurfaceDataParams &params);

	virtual void	debugDraw2d();

private:
	void			updateBounds();

	template<int VERTEX_TYPE, int CLIP_TYPE>
	void			getSurfaceData(VuWaterSurfaceDataParams &params);

	VuVector3		mPosition;
	float			mRadius;
	float			mDecayRatio;

protected:
	~VuWaterCircularOceanWave() {}
};
