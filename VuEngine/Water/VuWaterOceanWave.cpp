//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Water ocean wave implementation
// 
//*****************************************************************************

#include <float.h>
#include "VuWaterOceanWave.h"
#include "VuWater.h"
#include "VuEngine/Gfx/Camera/VuCamera.h"
#include "VuEngine/HAL/Gfx/VuGfx.h"
#include "VuEngine/Gfx/VuGfxUtil.h"
#include "VuEngine/Math/VuFFT.h"
#include "VuEngine/Math/VuRand.h"
#include "VuEngine/Dev/VuDevProfile.h"


IMPLEMENT_RTTI(VuWaterBaseOceanWave, VuWaterWave);
IMPLEMENT_RTTI(VuWaterInfiniteOceanWave, VuWaterBaseOceanWave);
IMPLEMENT_RTTI(VuWaterRectangularOceanWave, VuWaterBaseOceanWave);
IMPLEMENT_RTTI(VuWaterCircularOceanWave, VuWaterBaseOceanWave);


//*****************************************************************************
VuWaterBaseOceanWave::VuWaterBaseOceanWave(const VuWaterBaseOceanWaveDesc &desc):
	mGridPow(0),
	mGridDim(1),
	mGridBits(0),
	mGridResolution(0),
	mAge(0),
	mCurBuffer(0),
	mLastBuffer(1)
{
	setDesc(desc);
}

//*****************************************************************************
VuWaterBaseOceanWave::~VuWaterBaseOceanWave()
{
	if ( mDesc.mComplexity )
		freeResources();
}

//*****************************************************************************
bool VuWaterBaseOceanWave::tick(float fdt)
{
	if ( refCount() == 1 )
		return false; // done

	mAge += fdt;

	// swap buffer
	mCurBuffer = (mCurBuffer + 1)%BUFFER_COUNT;
	mLastBuffer = (mLastBuffer + 1)%BUFFER_COUNT;

	updateFFT();

	return true; // not done
}

//*****************************************************************************
void VuWaterBaseOceanWave::modifyHeightMultiplier(float heightMultiplier)
{
	mDesc.mHeightMultiplier = heightMultiplier;
}

//*****************************************************************************
void VuWaterBaseOceanWave::setDesc(const VuWaterBaseOceanWaveDesc &desc)
{
	VUASSERT(desc.mGravity > 0, "VuWaterBaseOceanWave::setDesc invalid gravity");

	if ( desc.mComplexity != mDesc.mComplexity )
	{
		// free resources 
		if ( mDesc.mComplexity )
		{
			freeResources();
		}

		// allocate resources
		if ( desc.mComplexity )
		{
			mGridPow = desc.mComplexity;
			mGridDim = 1 << mGridPow;
			mGridBits = mGridDim - 1;

			allocateResources();

			for ( int i = 0; i < mGridDim; i++ )
			{
				mpFFTSpeq[1][2*(i+1)-1] = 0;
				mpFFTSpeq[1][2*(i+1)] = 0;
			}
		}
	}

	// use new description
	mDesc = desc;
	mGridResolution = mGridDim/desc.mBinSize;

	// (FFT wave travels in -x direction)
	mFFTWaveDirection = mDesc.mWaveDirection - VU_PI;
	VuSinCos(mFFTWaveDirection, mSinFFTWaveDirection, mCosFFTWaveDirection);

	// rebuild patch info
	buildPatchInfo();

	// recalculate dispersion and initial fourier amplitudes
	calculateDispersion();
	calculateInitialFourierAmplitudes();

	// calculate one frame (into buffer 0)
	mCurBuffer = 0;
	updateFFT();

	// copy current buffer into other buffers (used for vertical velocity)
	for ( int i = 1; i < BUFFER_COUNT; i++ )
		mBuffers[i].copyFrom(mBuffers[0], mGridDim);

	mCurBuffer = 0;
	mLastBuffer = 1;
	mBuffers[mLastBuffer].mBufferAge -= 1.0f;
}

//*****************************************************************************
void VuWaterBaseOceanWave::allocateResources()
{
	mpDispersion = new float[mGridDim*(mGridDim/2)];
	mpFourierAmplitudes = new VuPackedVector2[mGridDim*(mGridDim/2)];
	mpFFTData = VuFFTAllocateFloatTensor3(1, 1, 1, mGridDim, 1, mGridDim);
	mpFFTSpeq = VuFFTAllocateFloatTensor2(1, 1, 1, mGridDim << 1);
	mpPatchInfo = new VuOceanWavePatchInfo[mGridDim*mGridDim];
	for ( int i = 0; i < BUFFER_COUNT; i++ )
		mBuffers[i].allocateResources(mGridDim);
}

//*****************************************************************************
void VuWaterBaseOceanWave::freeResources()
{
	delete[] mpDispersion;
	delete[] mpFourierAmplitudes;
	VuFFTFreeFloatTensor3(mpFFTData, 1, 1, 1, mGridDim, 1, mGridDim);
	VuFFTFreeFloatTensor2(mpFFTSpeq, 1, 1, 1, mGridDim << 1);
	delete[] mpPatchInfo;
	for ( int i = 0; i < BUFFER_COUNT; i++ )
		mBuffers[i].freeResources();
}

//*****************************************************************************
void VuWaterBaseOceanWave::buildPatchInfo()
{
	for ( int i = 0; i < mGridDim; i++ )
	{
		for ( int j = 0; j < mGridDim; j++ )
		{
			VuOceanWavePatchInfo *pPatch = &mpPatchInfo[(j<<mGridPow) + i];

			int il = (i - 1)&mGridBits;
			int i0 = i;
			int i1 = (i + 1)&mGridBits;
			int in = (i + 2)&mGridBits;

			int jl = (j - 1)&mGridBits;
			int j0 = j;
			int j1 = (j + 1)&mGridBits;
			int jn = (j + 2)&mGridBits;

			pPatch->mIndexI[0] = (i0<<mGridPow) + j0;
			pPatch->mIndexI[1] = (i1<<mGridPow) + j0;
			pPatch->mIndexI[2] = (i1<<mGridPow) + j1;
			pPatch->mIndexI[3] = (i0<<mGridPow) + j1;
			pPatch->mIndexO[0] = (i0<<mGridPow) + jl;
			pPatch->mIndexO[1] = (i1<<mGridPow) + jl;
			pPatch->mIndexO[2] = (in<<mGridPow) + j0;
			pPatch->mIndexO[3] = (in<<mGridPow) + j1;
			pPatch->mIndexO[4] = (i1<<mGridPow) + jn;
			pPatch->mIndexO[5] = (i0<<mGridPow) + jn;
			pPatch->mIndexO[6] = (il<<mGridPow) + j1;
			pPatch->mIndexO[7] = (il<<mGridPow) + j0;
		}
	}
}

//*****************************************************************************
void VuWaterBaseOceanWave::calculateDispersion()
{
	float *pDispersion = mpDispersion;
	for ( int i = 0; i < mGridDim; i++ )
	{
		for ( int j = 0; j < mGridDim/2; j++ )
		{
			VuVector2 k = VuVector2(i - mGridDim/2, j - mGridDim/2)*(2*VU_PI/mDesc.mBinSize);

			*pDispersion = VuSqrt(k.mag()*mDesc.mGravity);

			pDispersion++;
		}
	}
}

//*****************************************************************************
void VuWaterBaseOceanWave::calculateInitialFourierAmplitudes()
{
	VuRand rand(-1);

	float fL = mDesc.mWindSpeed*mDesc.mWindSpeed/mDesc.mGravity;

	// assume wind direction is always in -x
	VuVector2 vWindDirection = VuVector2(0, -1);

	VuPackedVector2 *pFourierAmplitude = mpFourierAmplitudes;
	for ( int i = 0; i < mGridDim; i++ )
	{
		for ( int j = 0; j < mGridDim/2; j++ )
		{
			VuVector2 k = VuVector2(i - mGridDim/2, j - mGridDim/2)*(2*VU_PI/mDesc.mBinSize);

			// calculate phillips spectrum
			float fPh = 0;
			if ( fL > 0 )
			{
				float fMagSquaredk = k.magSquared();
				if ( fMagSquaredk > 0 )
				{
					// calculate directional term
					float fDirectionalTerm = VuDot(k.normal(), vWindDirection);
					fDirectionalTerm = VuPow(fDirectionalTerm, mDesc.mDirectionalPower);
					fDirectionalTerm = VuAbs(fDirectionalTerm);

					// calculate small wave suppression term
					float fSmallWaveSuppressionTerm = VuExp(-fMagSquaredk*mDesc.mSuppressionWaveLength*mDesc.mSuppressionWaveLength);

					fPh = 0.0081f; // phillips spectrum constant
					fPh *= VuExp(-1/(fMagSquaredk*fL*fL));
					fPh /= fMagSquaredk*fMagSquaredk;
					fPh *= fDirectionalTerm;
					fPh *= fSmallWaveSuppressionTerm;
				}
			}

			fPh = VuSqrt(0.5f*fPh);

			pFourierAmplitude->mX = rand.gaussRand()*fPh;
			pFourierAmplitude->mY = rand.gaussRand()*fPh;

			pFourierAmplitude++;
		}
	}
}

//*****************************************************************************
void VuWaterBaseOceanWave::updateFFT()
{
	VU_PROFILE_SIM("Ocean Wave FFT");

	// calculate fourier amplitudes for current time
	calculateCurrentFourierAmplitudes();

	// perform inverse FFT
	VuFFTReal3(mpFFTData, mpFFTSpeq, 1, mGridDim, mGridDim, -1);

	// calculate height values
	calculateHeights();

	// build patches
	buildPatches();

	// save buffer age
	mBuffers[mCurBuffer].mBufferAge = mAge;
}

//*****************************************************************************
void VuWaterBaseOceanWave::calculateCurrentFourierAmplitudes()
{
	float *pDispersion = mpDispersion;
	VuPackedVector2 *pFourierAmplitude = mpFourierAmplitudes;
	float *pFFTSpeq = &mpFFTSpeq[1][1];
	for ( int i = 0; i < mGridDim; i++ )
	{
		float *pFFTData = &mpFFTData[1][i+1][1];
		for ( int j = 0; j < mGridDim/2; j++ )
		{
			float fSinTerm, fCosTerm;
			VuSinCosEst(VuModAngle((float)(*pDispersion*mAge)), fSinTerm, fCosTerm);

			pFFTData[0] = pFourierAmplitude->mX*fCosTerm - pFourierAmplitude->mY*fSinTerm;
			pFFTData[1] = pFourierAmplitude->mY*fCosTerm + pFourierAmplitude->mX*fSinTerm;

			pFFTData += 2;
			pDispersion++;
			pFourierAmplitude++;
		}

		pFFTSpeq[0] = 0;
		pFFTSpeq[1] = 0;
		pFFTSpeq += 2;
	}
}

//*****************************************************************************
void VuWaterBaseOceanWave::calculateHeights()
{
	float fSign = 1.0f;
	float *pHeight = mBuffers[mCurBuffer].mpHeights;
	for ( int i = 0; i < mGridDim; i++ )
	{
		float *pFFTData = &mpFFTData[1][i+1][1];
		for ( int j = 0; j < mGridDim; j++ )
		{
			*pHeight = *pFFTData*fSign*mDesc.mHeightMultiplier;

			fSign *= -1.0f;
			pFFTData++;
			pHeight++;
		}
		fSign *= -1.0f;
	}

#if 0
	static float fMax = 0;
	pHeight = mpHeights;
	for ( int i = 0; i < mGridDim; i++ )
	{
		for ( int j = 0; j < mGridDim; j++ )
		{
			fMax = VuMax(fMax, *pHeight);
			pHeight++;
		}
	}
	VUPRINTF("%10.5f\n", fMax);
#endif
}

//*****************************************************************************
void VuWaterBaseOceanWave::buildPatches()
{
	float *pHeights = mBuffers[mCurBuffer].mpHeights;
	VuPatch<float> *pPatch = mBuffers[mCurBuffer].mpPatches;
	const VuOceanWavePatchInfo *pInfo = mpPatchInfo;

	for ( int i = 0; i < mGridDim; i++ )
	{
		for ( int i = 0; i < mGridDim; i++ )
		{
			pPatch->set(
				pHeights[pInfo->mIndexI[0]],
				pHeights[pInfo->mIndexI[1]],
				pHeights[pInfo->mIndexI[2]],
				pHeights[pInfo->mIndexI[3]],
				pHeights[pInfo->mIndexO[0]],
				pHeights[pInfo->mIndexO[1]],
				pHeights[pInfo->mIndexO[2]],
				pHeights[pInfo->mIndexO[3]],
				pHeights[pInfo->mIndexO[4]],
				pHeights[pInfo->mIndexO[5]],
				pHeights[pInfo->mIndexO[6]],
				pHeights[pInfo->mIndexO[7]]
			);

			pPatch++;
			pInfo++;
		}
	}
}

//*****************************************************************************
int VuWaterBaseOceanWave::getPatchIndex(float x, float y, float &u, float &v)
{
	x *= mGridResolution;
	y *= mGridResolution;

	int i = VuFloorInt(x);
	int j = VuFloorInt(y);

	u = x - i;
	v = y - j;

	i &= mGridBits;
	j &= mGridBits;

	return (j<<mGridPow) + i;
}

//*****************************************************************************
void VuOceanWaveBuffer::copyFrom(const VuOceanWaveBuffer &other, int gridDim)
{
	VU_MEMCPY(mpHeights, sizeof(mpHeights[0])*gridDim*gridDim, other.mpHeights, sizeof(mpHeights[0])*gridDim*gridDim);
	VU_MEMCPY(mpPatches, sizeof(mpPatches[0])*gridDim*gridDim, other.mpPatches, sizeof(mpPatches[0])*gridDim*gridDim);
}

//*****************************************************************************
void VuOceanWaveBuffer::allocateResources(int gridDim)
{
	mpHeights = new float[gridDim*gridDim];
	mpPatches = new VuPatch<float>[gridDim*gridDim];
}

//*****************************************************************************
void VuOceanWaveBuffer::freeResources()
{
	delete[] mpHeights;
	delete[] mpPatches;
}

//*****************************************************************************
VuWaterInfiniteOceanWave::VuWaterInfiniteOceanWave(const VuWaterInfiniteOceanWaveDesc &desc):
	VuWaterBaseOceanWave(desc)
{
}

//*****************************************************************************
void VuWaterInfiniteOceanWave::modify(const VuWaterInfiniteOceanWaveDesc &desc)
{
	setDesc(desc);
}

//*****************************************************************************
void VuWaterInfiniteOceanWave::getSurfaceData(VuWaterSurfaceDataParams &params)
{
	if ( params.mVertexType == VuWaterSurfaceDataParams::VT_PHYSICS )
		if ( params.mClipType == VuWaterSurfaceDataParams::CT_NOCLIP )
			getSurfaceData<VuWaterSurfaceDataParams::VT_PHYSICS, VuWaterSurfaceDataParams::CT_NOCLIP>(params);
		else
			getSurfaceData<VuWaterSurfaceDataParams::VT_PHYSICS, VuWaterSurfaceDataParams::CT_CLIP>(params);
	else
		if ( params.mClipType == VuWaterSurfaceDataParams::CT_NOCLIP )
			getSurfaceData<VuWaterSurfaceDataParams::VT_RENDER, VuWaterSurfaceDataParams::CT_NOCLIP>(params);
		else
			getSurfaceData<VuWaterSurfaceDataParams::VT_RENDER, VuWaterSurfaceDataParams::CT_CLIP>(params);
}

//*****************************************************************************
template<int VERTEX_TYPE, int CLIP_TYPE>
void VuWaterInfiniteOceanWave::getSurfaceData(VuWaterSurfaceDataParams &params)
{
	VuOceanWaveBuffer *pCurBuffer = &mBuffers[mCurBuffer];

	VuWaterVertex vert;
	if ( VERTEX_TYPE == VuWaterSurfaceDataParams::VT_PHYSICS )
		vert.configure(params.mpPhysicsVertex);
	else
		vert.configure(params.mpRenderVertex);

	for ( int iVert = 0; iVert < params.mVertCount; iVert++ )
	{
		if ( CLIP_TYPE == VuWaterSurfaceDataParams::CT_NOCLIP || params.mppWaterSurface[iVert] == params.mpWaterClipSurface )
		{
			// transform into local space
			float localX = vert.mpPosition->mX*mCosFFTWaveDirection - vert.mpPosition->mY*mSinFFTWaveDirection;
			float localY = vert.mpPosition->mY*mCosFFTWaveDirection + vert.mpPosition->mX*mSinFFTWaveDirection;

			// determine patch (and calculate patch u/v)
			float u, v;
			int index = getPatchIndex(localX, localY, u, v);
			VuPatch<float> *pCurPatch = &pCurBuffer->mpPatches[index];
			
			// handle dz/dxy
			float fHeight = 0;
			if ( VERTEX_TYPE == VuWaterSurfaceDataParams::VT_RENDER )
			{
				float dheight_du, dheight_dv;
				fHeight = pCurPatch->interpolate(u, v, dheight_du, dheight_dv);
				dheight_du *= mGridResolution;
				dheight_dv *= mGridResolution;

				// transform back to world space
				float dheight_dx = dheight_du*mCosFFTWaveDirection + dheight_dv*mSinFFTWaveDirection;
				float dheight_dy = dheight_dv*mCosFFTWaveDirection - dheight_du*mSinFFTWaveDirection;

				vert.mpDzDxy->mX += dheight_dx;
				vert.mpDzDxy->mY += dheight_dy;
			}
			else
			{
				fHeight = pCurPatch->interpolate(u, v);
			}

			*vert.mpHeight += fHeight;

			if ( VERTEX_TYPE == VuWaterSurfaceDataParams::VT_PHYSICS )
			{
				VuOceanWaveBuffer *pLastBuffer = &mBuffers[mLastBuffer];
				VuPatch<float> *pLastPatch = &pLastBuffer->mpPatches[index];
				float fLastHeight = pLastPatch->interpolate(u, v);
				vert.mpDxyzDt->mZ += (float)((fHeight - fLastHeight)/(mAge - pLastBuffer->mBufferAge));
			}
		}

		// next vert
		vert.mpPosition = (VuPackedVector3 *)((VUBYTE *)vert.mpPosition + params.mStride);
		vert.mpHeight = (float *)((VUBYTE *)vert.mpHeight + params.mStride);
		if ( VERTEX_TYPE == VuWaterSurfaceDataParams::VT_PHYSICS )
			vert.mpDxyzDt = (VuPackedVector3 *)((VUBYTE *)vert.mpDxyzDt + params.mStride);
		else
			vert.mpDzDxy = (VuPackedVector2 *)((VUBYTE *)vert.mpDzDxy + params.mStride);
	}
}

//*****************************************************************************
VuWaterRectangularOceanWave::VuWaterRectangularOceanWave(const VuWaterRectangularOceanWaveDesc &desc):
	VuWaterBaseOceanWave(desc),
	mPosition(desc.mPosition),
	mSizeX(desc.mSizeX),
	mSizeY(desc.mSizeY),
	mDecayRatioX(desc.mDecayRatioX),
	mDecayRatioY(desc.mDecayRatioY)
{
	updateBounds();
}

//*****************************************************************************
void VuWaterRectangularOceanWave::modify(const VuWaterRectangularOceanWaveDesc &desc)
{
	setDesc(desc);

	mPosition = desc.mPosition;
	mSizeX = desc.mSizeX;
	mSizeY = desc.mSizeY;
	mDecayRatioX = desc.mDecayRatioX;
	mDecayRatioY = desc.mDecayRatioY;

	updateBounds();
	VuWater::IF()->rebinWave(this);
}

//*****************************************************************************
void VuWaterRectangularOceanWave::getSurfaceData(VuWaterSurfaceDataParams &params)
{
	if ( params.mVertexType == VuWaterSurfaceDataParams::VT_PHYSICS )
		if ( params.mClipType == VuWaterSurfaceDataParams::CT_NOCLIP )
			getSurfaceData<VuWaterSurfaceDataParams::VT_PHYSICS, VuWaterSurfaceDataParams::CT_NOCLIP>(params);
		else
			getSurfaceData<VuWaterSurfaceDataParams::VT_PHYSICS, VuWaterSurfaceDataParams::CT_CLIP>(params);
	else
		if ( params.mClipType == VuWaterSurfaceDataParams::CT_NOCLIP )
			getSurfaceData<VuWaterSurfaceDataParams::VT_RENDER, VuWaterSurfaceDataParams::CT_NOCLIP>(params);
		else
			getSurfaceData<VuWaterSurfaceDataParams::VT_RENDER, VuWaterSurfaceDataParams::CT_CLIP>(params);
}

//*****************************************************************************
void VuWaterRectangularOceanWave::debugDraw2d()
{
	VuGfxUtil *pGfxUtil = VuGfxUtil::IF();

	// set up matrix
	VuMatrix mat = pGfxUtil->getMatrix();
	mat.translateLocal(VuVector3(mPosition.mX, mPosition.mY, 0));
	mat.scaleLocal(VuVector3(mSizeX, mSizeY, 1));

	pGfxUtil->pushMatrix(mat);
	{
		pGfxUtil->drawRectangleOutline2d(0, VuColor(64,255,64,128), VuRect(-0.5f, -0.5f, 1.0f, 1.0f));
	}
	pGfxUtil->popMatrix();
}

//*****************************************************************************
void VuWaterRectangularOceanWave::updateBounds()
{
	// aabb
	VuVector2 vCenter(mPosition.mX, mPosition.mY);
	VuVector2 vExtents(0.5f*mSizeX, 0.5f*mSizeY);

	VuVector2 aabbMin = vCenter - vExtents;
	VuVector2 aabbMax = vCenter + vExtents;

	mBoundingAabb.mMin = VuVector3(aabbMin.mX, aabbMin.mY, mPosition.mZ);
	mBoundingAabb.mMax = VuVector3(aabbMax.mX, aabbMax.mY, mPosition.mZ);

	// disk
	mBoundingDiskCenter = vCenter;
	mBoundingDiskRadius = vExtents.mag();
}

//*****************************************************************************
template<int VERTEX_TYPE, int CLIP_TYPE>
void VuWaterRectangularOceanWave::getSurfaceData(VuWaterSurfaceDataParams &params)
{
	VuOceanWaveBuffer *pCurBuffer = &mBuffers[mCurBuffer];

	VuWaterVertex vert;
	if ( VERTEX_TYPE == VuWaterSurfaceDataParams::VT_PHYSICS )
		vert.configure(params.mpPhysicsVertex);
	else
		vert.configure(params.mpRenderVertex);

	for ( int iVert = 0; iVert < params.mVertCount; iVert++ )
	{
		if ( CLIP_TYPE == VuWaterSurfaceDataParams::CT_NOCLIP || params.mppWaterSurface[iVert] == params.mpWaterClipSurface )
		{
			// transform into local space
			float relX = vert.mpPosition->mX - mPosition.mX;
			float relY = vert.mpPosition->mY - mPosition.mY;

			float localX = relX*mCosFFTWaveDirection - relY*mSinFFTWaveDirection;
			float localY = relY*mCosFFTWaveDirection + relX*mSinFFTWaveDirection;

			// determine patch (and calculate patch u/v)
			float u, v;
			int index = getPatchIndex(localX, localY, u, v);
			VuPatch<float> *pCurPatch = &pCurBuffer->mpPatches[index];

			// check if within range
			float scaleX = 2.0f/mSizeX;
			float ratioX = scaleX*relX;
			float absRatioX = VuAbs(ratioX);
			if ( absRatioX < 1 )
			{
				float scaleY = 2.0f/mSizeY;
				float ratioY = scaleY*relY;
				float absRatioY = VuAbs(ratioY);
				if ( absRatioY < 1 )
				{
					// calculate damping
					float dampingx = 1.0f;
					if ( absRatioX > mDecayRatioX )
						dampingx = (absRatioX - 1)/(mDecayRatioX - 1);

					float dampingy = 1.0f;
					if ( absRatioY > mDecayRatioY )
						dampingy = (absRatioY - 1)/(mDecayRatioY - 1);

					// handle dz/dxy
					float height = 0;
					if ( VERTEX_TYPE == VuWaterSurfaceDataParams::VT_RENDER )
					{
						float dheight_du, dheight_dv;
						height = pCurPatch->interpolate(u, v, dheight_du, dheight_dv);
						dheight_du *= mGridResolution;
						dheight_dv *= mGridResolution;

						// transform back to world space
						float dheight_dx = dheight_du*mCosFFTWaveDirection + dheight_dv*mSinFFTWaveDirection;
						float dheight_dy = dheight_dv*mCosFFTWaveDirection - dheight_du*mSinFFTWaveDirection;

						height = height*dampingx;
						height = height*dampingy;

						// x damping?
						if ( absRatioX > mDecayRatioX )
						{
							//float absRatioX = VuAbs(ratioX);
							float dabsRatioX_dx = ratioX < 0 ? -scaleX : scaleX;

							//dampingx = (absRatioX - 1)/(mDecayRatioX - 1);
							float ddampingx_dx = dabsRatioX_dx/(mDecayRatioX - 1);

							//height = height*dampingx;
							dheight_dx = height*ddampingx_dx + dheight_dx*dampingx;
						}

						// y damping?
						if ( absRatioY > mDecayRatioY )
						{
							//float absRatioY = VuAbs(ratioY);
							float dabsRatioY_dy = ratioY < 0 ? -scaleY : scaleY;

							//dampingy = (absRatioY - 1)/(mDecayRatioY - 1);
							float ddampingy_dy = dabsRatioY_dy/(mDecayRatioY - 1);

							//height = height*dampingy;
							dheight_dy = height*ddampingy_dy + dheight_dy*dampingy;
						}

						vert.mpDzDxy->mX += dheight_dx;
						vert.mpDzDxy->mY += dheight_dy;
					}
					else
					{
						height = pCurPatch->interpolate(u, v);

						height *= dampingx;
						height *= dampingy;
					}

					*vert.mpHeight += height;

					if ( VERTEX_TYPE == VuWaterSurfaceDataParams::VT_PHYSICS )
					{
						VuOceanWaveBuffer *pLastBuffer = &mBuffers[mLastBuffer];
						VuPatch<float> *pLastPatch = &pLastBuffer->mpPatches[index];
						float lastHeight = pLastPatch->interpolate(u, v);
						lastHeight *= dampingx;
						lastHeight *= dampingy;

						float dheight_dt = (float)((height - lastHeight)/(mAge - pLastBuffer->mBufferAge));

						vert.mpDxyzDt->mZ += dheight_dt;
					}
				}
			}
		}

		// next vert
		vert.mpPosition = (VuPackedVector3 *)((VUBYTE *)vert.mpPosition + params.mStride);
		vert.mpHeight = (float *)((VUBYTE *)vert.mpHeight + params.mStride);
		if ( VERTEX_TYPE == VuWaterSurfaceDataParams::VT_PHYSICS )
			vert.mpDxyzDt = (VuPackedVector3 *)((VUBYTE *)vert.mpDxyzDt + params.mStride);
		else
			vert.mpDzDxy = (VuPackedVector2 *)((VUBYTE *)vert.mpDzDxy + params.mStride);
	}
}

//*****************************************************************************
VuWaterCircularOceanWave::VuWaterCircularOceanWave(const VuWaterCircularOceanWaveDesc &desc):
	VuWaterBaseOceanWave(desc),
	mPosition(desc.mPosition),
	mRadius(desc.mRadius),
	mDecayRatio(desc.mDecayRatio)
{
	updateBounds();
}

//*****************************************************************************
void VuWaterCircularOceanWave::modify(const VuWaterCircularOceanWaveDesc &desc)
{
	setDesc(desc);

	mPosition = desc.mPosition;
	mRadius = desc.mRadius;
	mDecayRatio = desc.mDecayRatio;

	updateBounds();
	VuWater::IF()->rebinWave(this);
}

//*****************************************************************************
void VuWaterCircularOceanWave::getSurfaceData(VuWaterSurfaceDataParams &params)
{
	if ( params.mVertexType == VuWaterSurfaceDataParams::VT_PHYSICS )
		if ( params.mClipType == VuWaterSurfaceDataParams::CT_NOCLIP )
			getSurfaceData<VuWaterSurfaceDataParams::VT_PHYSICS, VuWaterSurfaceDataParams::CT_NOCLIP>(params);
		else
			getSurfaceData<VuWaterSurfaceDataParams::VT_PHYSICS, VuWaterSurfaceDataParams::CT_CLIP>(params);
	else
		if ( params.mClipType == VuWaterSurfaceDataParams::CT_NOCLIP )
			getSurfaceData<VuWaterSurfaceDataParams::VT_RENDER, VuWaterSurfaceDataParams::CT_NOCLIP>(params);
		else
			getSurfaceData<VuWaterSurfaceDataParams::VT_RENDER, VuWaterSurfaceDataParams::CT_CLIP>(params);
}

//*****************************************************************************
void VuWaterCircularOceanWave::debugDraw2d()
{
	VuGfxUtil *pGfxUtil = VuGfxUtil::IF();

	// set up matrix
	VuMatrix mat = pGfxUtil->getMatrix();
	mat.translateLocal(VuVector3(mPosition.mX, mPosition.mY, 0));
	mat.scaleLocal(VuVector3(mRadius, mRadius, 1));

	pGfxUtil->pushMatrix(mat);
	{
		pGfxUtil->drawEllipseOutline2d(0, VuColor(192,192,192), VuRect(-1, -1, 2, 2), 32);
	}
	pGfxUtil->popMatrix();
}

//*****************************************************************************
void VuWaterCircularOceanWave::updateBounds()
{
	// aabb
	VuVector2 vCenter(mPosition.mX, mPosition.mY);
	VuVector2 vExtents(mRadius, mRadius);

	VuVector2 aabbMin = vCenter - vExtents;
	VuVector2 aabbMax = vCenter + vExtents;

	mBoundingAabb.mMin = VuVector3(aabbMin.mX, aabbMin.mY, mPosition.mZ);
	mBoundingAabb.mMax = VuVector3(aabbMax.mX, aabbMax.mY, mPosition.mZ);

	// disk
	mBoundingDiskCenter = vCenter;
	mBoundingDiskRadius = mRadius;
}

//*****************************************************************************
template<int VERTEX_TYPE, int CLIP_TYPE>
void VuWaterCircularOceanWave::getSurfaceData(VuWaterSurfaceDataParams &params)
{
	VuOceanWaveBuffer *pCurBuffer = &mBuffers[mCurBuffer];

	VuWaterVertex vert;
	if ( VERTEX_TYPE == VuWaterSurfaceDataParams::VT_PHYSICS )
		vert.configure(params.mpPhysicsVertex);
	else
		vert.configure(params.mpRenderVertex);

	for ( int iVert = 0; iVert < params.mVertCount; iVert++ )
	{
		if ( CLIP_TYPE == VuWaterSurfaceDataParams::CT_NOCLIP || params.mppWaterSurface[iVert] == params.mpWaterClipSurface )
		{
			// transform into local space
			float relX = vert.mpPosition->mX - mPosition.mX;
			float relY = vert.mpPosition->mY - mPosition.mY;

			float localX = relX*mCosFFTWaveDirection - relY*mSinFFTWaveDirection;
			float localY = relY*mCosFFTWaveDirection + relX*mSinFFTWaveDirection;

			// determine patch (and calculate patch u/v)
			float u, v;
			int index = getPatchIndex(localX, localY, u, v);
			VuPatch<float> *pCurPatch = &pCurBuffer->mpPatches[index];

			// check if within range
			float distSquared = localX*localX + localY*localY;
			float dist = VuSqrt(distSquared);
			float ratio = dist/mRadius;
			if ( ratio < 1 )
			{
				// calculate damping
				float damping = 1.0f;
				if ( ratio > mDecayRatio )
					damping = (ratio - 1)/(mDecayRatio - 1);

				// handle dz/dxy
				float height = 0;
				if ( VERTEX_TYPE == VuWaterSurfaceDataParams::VT_RENDER )
				{
					float dheight_du, dheight_dv;
					height = pCurPatch->interpolate(u, v, dheight_du, dheight_dv);
					dheight_du *= mGridResolution;
					dheight_dv *= mGridResolution;

					// transform back to world space
					float dheight_dx = dheight_du*mCosFFTWaveDirection + dheight_dv*mSinFFTWaveDirection;
					float dheight_dy = dheight_dv*mCosFFTWaveDirection - dheight_du*mSinFFTWaveDirection;

					height = height*damping;

					// damping?
					if ( ratio > mDecayRatio )
					{
						//float localX = relX*mCosFFTWaveDirection - relY*mSinFFTWaveDirection;
						float dlocalX_dx = mCosFFTWaveDirection;
						float dlocalX_dy = -mSinFFTWaveDirection;

						//float localY = relY*mCosFFTWaveDirection + relX*mSinFFTWaveDirection;
						float dlocalY_dx = mSinFFTWaveDirection;
						float dlocalY_dy = mCosFFTWaveDirection;

						//float distSquared = localX*localX + localY*localY;
						float ddistSquared_dx = 2*localX*dlocalX_dx + 2*localY*dlocalY_dx;
						float ddistSquared_dy = 2*localX*dlocalX_dy + 2*localY*dlocalY_dy;

						//float dist = VuSqrt(distSquared);
						float ddist_dx = ddistSquared_dx/(2*dist);
						float ddist_dy = ddistSquared_dy/(2*dist);

						//float ratio = dist/mRadius;
						float dratio_dx = ddist_dx/mRadius;
						float dratio_dy = ddist_dy/mRadius;

						// damping = (ratio - 1)/(mDecayRatio - 1);
						float ddamping_dx = dratio_dx/(mDecayRatio - 1);
						float ddamping_dy = dratio_dy/(mDecayRatio - 1);

						//height = height*damping;
						dheight_dx = height*ddamping_dx + dheight_dx*damping;
						dheight_dy = height*ddamping_dy + dheight_dy*damping;
					}

					vert.mpDzDxy->mX += dheight_dx;
					vert.mpDzDxy->mY += dheight_dy;
				}
				else
				{
					height = pCurPatch->interpolate(u, v);

					height *= damping;
				}

				*vert.mpHeight += height;

				if ( VERTEX_TYPE == VuWaterSurfaceDataParams::VT_PHYSICS )
				{
					VuOceanWaveBuffer *pLastBuffer = &mBuffers[mLastBuffer];
					VuPatch<float> *pLastPatch = &pLastBuffer->mpPatches[index];
					float lastHeight = pLastPatch->interpolate(u, v);
					lastHeight *= damping;

					float dheight_dt = (float)((height - lastHeight)/(mAge - pLastBuffer->mBufferAge));

					vert.mpDxyzDt->mZ += dheight_dt;
				}
			}
		}

		// next vert
		vert.mpPosition = (VuPackedVector3 *)((VUBYTE *)vert.mpPosition + params.mStride);
		vert.mpHeight = (float *)((VUBYTE *)vert.mpHeight + params.mStride);
		if ( VERTEX_TYPE == VuWaterSurfaceDataParams::VT_PHYSICS )
			vert.mpDxyzDt = (VuPackedVector3 *)((VUBYTE *)vert.mpDxyzDt + params.mStride);
		else
			vert.mpDzDxy = (VuPackedVector2 *)((VUBYTE *)vert.mpDzDxy + params.mStride);
	}
}
