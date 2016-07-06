//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Water texture
// 
//*****************************************************************************

#include "VuWaterTexture.h"
#include "VuWater.h"
#include "VuEngine/HAL/Gfx/VuGfx.h"
#include "VuEngine/HAL/Gfx/VuTexture.h"
#include "VuEngine/Gfx/GfxSort/VuGfxSort.h"
#include "VuEngine/Gfx/VuGfxUtil.h"
#include "VuEngine/Managers/VuTickManager.h"
#include "VuEngine/Managers/VuDrawManager.h"
#include "VuEngine/Math/VuPackedVector.h"
#include "VuEngine/Math/VuFFT.h"
#include "VuEngine/Math/VuRand.h"
#include "VuEngine/Dev/VuDevProfile.h"
#include "VuEngine/Dev/VuDevMenu.h"


#define TEXTURE_POW 6
#define TEXTURE_SIZE (1<<TEXTURE_POW)
#define TEXTURE_MASK (TEXTURE_SIZE - 1)
#define TEXTURE_MIP_LEVELS (TEXTURE_POW + 1)
#define TEXTURE_MIP_SIZE(level) (TEXTURE_SIZE >> level)

//*****************************************************************************
VuWaterTexture::VuWaterTexture():
	mActiveTexture(0),
	mbShowTexture(false),
	mAge(0.0)
{
	// set up dev menu/stats
	VuDevMenu::IF()->addBool("Water/Texture/Show", mbShowTexture);
	VuDevMenu::IF()->addFloat("Water/Texture/Gravity", mSimDesc.mGravity, 1.0f, 1.0f);
	VuDevMenu::IF()->addFloat("Water/Texture/WorldSize", mSimDesc.mWorldSize, 1.0f, 0.0f);
	VuDevMenu::IF()->addFloat("Water/Texture/WindSpeed", mSimDesc.mWindSpeed, 0.25f, 0.0f);
	VuDevMenu::IF()->addFloat("Water/Texture/DirectionalPower", mSimDesc.mDirectionalPower, 0.25f, 0.0f);
	VuDevMenu::IF()->addFloat("Water/Texture/SuppressionWaveLength", mSimDesc.mSuppressionWaveLength, 0.1f, 0.0f);
	VuDevMenu::IF()->addFloat("Water/Texture/HeightFactor", mSimDesc.mHeightFactor, 0.1f, 0.0f);
	VuDevMenu::IF()->addFloat("Water/Texture/TimeFactor", mSimDesc.mTimeFactor, 0.1f, 0.0f);
	
	mFormat = VUGFX_FORMAT_LIN_V8U8;
	if ( !VuGfx::supportsTextureFormat(VUPLATFORM, mFormat) )
		mFormat = VUGFX_FORMAT_LIN_L8A8;

	VuTextureState state;
	for ( int i = 0; i < TEXTURE_COUNT; i++ )
		mpTextures[i] = VuGfx::IF()->createTexture(TEXTURE_SIZE, TEXTURE_SIZE, VUGFX_USAGE_DYNAMIC, mFormat, state);

	mpDispersion = new float[TEXTURE_SIZE*(TEXTURE_SIZE/2)];
	mpFourierAmplitudes = new VuPackedVector2[TEXTURE_SIZE*(TEXTURE_SIZE/2)];
	mpFFTData = VuFFTAllocateFloatTensor3(1, 1, 1, TEXTURE_SIZE, 1, TEXTURE_SIZE);
	mpFFTSpeq = VuFFTAllocateFloatTensor2(1, 1, 1, TEXTURE_SIZE << 1);
	mpHeights = new float[TEXTURE_SIZE*TEXTURE_SIZE];
	mpNormals = new VUUINT16 *[TEXTURE_MIP_LEVELS];
	for ( int i = 0; i < TEXTURE_MIP_LEVELS; i++ )
		mpNormals[i] = new VUUINT16[TEXTURE_MIP_SIZE(i)*TEXTURE_MIP_SIZE(i)];

	for ( int i = 0; i < TEXTURE_SIZE; i++ )
	{
		mpFFTSpeq[1][2*(i+1)-1] = 0;
		mpFFTSpeq[1][2*(i+1)] = 0;
	}

	// register tick/draw functions
	VuTickManager::IF()->registerHandler(this, &VuWaterTexture::tick, "Build");
	VuDrawManager::IF()->registerHandler(this, &VuWaterTexture::submit);

	// initialize FFT
	initializeFFT();
}

//*****************************************************************************
VuWaterTexture::~VuWaterTexture()
{
	for ( int i = 0; i < TEXTURE_COUNT; i++ )
		mpTextures[i]->removeRef();

	delete[] mpDispersion;
	delete[] mpFourierAmplitudes;
	VuFFTFreeFloatTensor3(mpFFTData, 1, 1, 1, TEXTURE_SIZE, 1, TEXTURE_SIZE);
	VuFFTFreeFloatTensor2(mpFFTSpeq, 1, 1, 1, TEXTURE_SIZE << 1);
	delete[] mpHeights;
	for ( int i = 0; i < TEXTURE_MIP_LEVELS; i++ )
		delete[] mpNormals[i];
	delete[] mpNormals;

	VuTickManager::IF()->unregisterHandlers(this);
	VuDrawManager::IF()->unregisterHandler(this);
}

//*****************************************************************************
void VuWaterTexture::tick(float fdt)
{
	mAge += mSimDesc.mTimeFactor*fdt;
}

//*****************************************************************************
void VuWaterTexture::submit()
{
	if ( !VuWater::IF()->getNormalMapEnabled() )
		return;

	struct DrawData
	{
		static void callback(void *data)
		{
			DrawData *pData = static_cast<DrawData *>(data);
			pData->mpWaterTexture->draw();
		}

		VuWaterTexture	*mpWaterTexture;
	};

	DrawData *pData = static_cast<DrawData *>(VuGfxSort::IF()->allocateCommandMemory(sizeof(DrawData)));
	pData->mpWaterTexture = this;

	VuGfxSort::IF()->setFullScreenLayer(VuGfxSort::FSL_GAME);
	VuGfxSort::IF()->setViewport(0);
	VuGfxSort::IF()->setReflectionLayer(0);
	VuGfxSort::IF()->setViewportLayer(VuGfxSort::VPL_BEGIN);
	VuGfxSort::IF()->submitCommand(VuGfxSort::TRANS_WATER_COLOR, 0, &DrawData::callback);

	if ( mbShowTexture )
	{
		VuGfxSort::IF()->setFullScreenLayer(VuGfxSort::FSL_UI);
		VuGfxSort::IF()->setViewportLayer(VuGfxSort::VPL_UI);

		int displayWidth, displayHeight;
		VuGfx::IF()->getDisplaySize(VUNULL, displayWidth, displayHeight);
		float displayAspectRatio = (float)displayWidth/displayHeight;

		float fSizeX = 0.2f;
		float fSizeY = fSizeX*displayAspectRatio;

		float fStartX = 1 - fSizeX;
		float fStartY = 1 - fSizeY;

		VuGfxUtil::IF()->drawTexture2d(0, getActiveTexture(), VuRect(fStartX, fStartY, fSizeX, fSizeY));
	}
}

//*****************************************************************************
void VuWaterTexture::draw()
{
	VU_PROFILE_GFX("WaterTexture");

	mActiveTexture = (mActiveTexture + 1)%TEXTURE_COUNT;

	// has desc changed?
	if ( memcmp(&mGfxDesc, &mSimDesc, sizeof(mSimDesc)) != 0 )
	{
		mGfxDesc = mSimDesc;
		initializeFFT();
	}

	// update fft
	updateFFT();

	// calculate normals for mip 0
	calculateNormals();

	// write level 0 to texture
	writeNormalsToTexture(0, mpTextures[mActiveTexture]);

	for ( int i = 1; i < TEXTURE_MIP_LEVELS; i++ )
	{
		buildMipLevel(i);
		writeNormalsToTexture(i, mpTextures[mActiveTexture]);
	}
}

//*****************************************************************************
void VuWaterTexture::initializeFFT()
{
	// recalculate dispersion and initial fourier amplitudes
	calculateDispersion();
	calculateInitialFourierAmplitudes();
}

//*****************************************************************************
void VuWaterTexture::calculateDispersion()
{
	float fGravity = mGfxDesc.mGravity;

	float *pDispersion = mpDispersion;
	for ( int i = 0; i < TEXTURE_SIZE; i++ )
	{
		for ( int j = 0; j < TEXTURE_SIZE/2; j++ )
		{
			VuVector2 k = VuVector2(i - TEXTURE_SIZE/2, j - TEXTURE_SIZE/2)*(2*VU_PI/mGfxDesc.mWorldSize);

			*pDispersion = VuSqrt(k.mag()*fGravity);

			pDispersion++;
		}
	}
}

//*****************************************************************************
void VuWaterTexture::calculateInitialFourierAmplitudes()
{
	VuRand rand(-1);

	float fGravity = mGfxDesc.mGravity;
	float fL = mGfxDesc.mWindSpeed*mGfxDesc.mWindSpeed/fGravity;

	// assume wind direction is always in -x
	VuVector2 vWindDirection = VuVector2(0, -1);

	VuPackedVector2 *pFourierAmplitude = mpFourierAmplitudes;
	for ( int i = 0; i < TEXTURE_SIZE; i++ )
	{
		for ( int j = 0; j < TEXTURE_SIZE/2; j++ )
		{
			VuVector2 k = VuVector2(i - TEXTURE_SIZE/2, j - TEXTURE_SIZE/2)*(2*VU_PI/mGfxDesc.mWorldSize);

			// calculate phillips spectrum
			float fPh = 0;
			if ( fL > 0 )
			{
				float fMagSquaredk = k.magSquared();
				if ( fMagSquaredk > 0 )
				{
					// calculate directional term
					float fDirectionalTerm = VuDot(k.normal(), vWindDirection);
					fDirectionalTerm = VuPow(fDirectionalTerm, mGfxDesc.mDirectionalPower);
					fDirectionalTerm = VuAbs(fDirectionalTerm);

					// calculate small wave suppression term
					float fSmallWaveSuppressionTerm = VuExp(-fMagSquaredk*mGfxDesc.mSuppressionWaveLength*mGfxDesc.mSuppressionWaveLength);

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
void VuWaterTexture::updateFFT()
{
	// calculate fourier amplitudes for current time
	calculateCurrentFourierAmplitudes();

	// perform inverse FFT
	VuFFTReal3(mpFFTData, mpFFTSpeq, 1, TEXTURE_SIZE, TEXTURE_SIZE, -1);

	// calculate height values
	calculateHeights();
}

//*****************************************************************************
void VuWaterTexture::calculateCurrentFourierAmplitudes()
{
	float *pDispersion = mpDispersion;
	VuPackedVector2 *pFourierAmplitude = mpFourierAmplitudes;
	float *pFFTSpeq = &mpFFTSpeq[1][1];
	for ( int i = 0; i < TEXTURE_SIZE; i++ )
	{
		float *pFFTData = &mpFFTData[1][i+1][1];
		for ( int j = 0; j < TEXTURE_SIZE/2; j++ )
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
void VuWaterTexture::calculateHeights()
{
	float fSign = 1.0f;
	float *pHeight = mpHeights;
	for ( int i = 0; i < TEXTURE_SIZE; i++ )
	{
		float *pFFTData = &mpFFTData[1][i+1][1];
		for ( int j = 0; j < TEXTURE_SIZE; j++ )
		{
			*pHeight = *pFFTData*fSign*mGfxDesc.mHeightFactor;

			fSign *= -1.0f;
			pFFTData++;
			pHeight++;
		}
		fSign *= -1.0f;
	}
}

//*****************************************************************************
void VuWaterTexture::calculateNormals()
{
	if ( mFormat == VUGFX_FORMAT_LIN_V8U8 )
	{
		VUINT8 *pDst = (VUINT8 *)mpNormals[0];
		for ( int y = 0; y < TEXTURE_SIZE; y++ )
		{
			for ( int x = 0; x < TEXTURE_SIZE; x++ )
			{
				float height00 = mpHeights[(y<<TEXTURE_POW) + x];
				float height10 = mpHeights[(y<<TEXTURE_POW) + ((x + 1)&TEXTURE_MASK)];
				float height01 = mpHeights[(((y + 1)&TEXTURE_MASK)<<TEXTURE_POW) + x];

				float gradx = height10 - height00;
				float grady = height01 - height00;

				pDst[0] = (VUINT8)VuRound(gradx*127.0f);
				pDst[1] = (VUINT8)VuRound(grady*127.0f);

				pDst += 2;
			}
		}
	}
	else if ( mFormat == VUGFX_FORMAT_LIN_L8A8 )
	{
		VUUINT8 *pDst = (VUUINT8 *)mpNormals[0];
		for ( int y = 0; y < TEXTURE_SIZE; y++ )
		{
			for ( int x = 0; x < TEXTURE_SIZE; x++ )
			{
				float height00 = mpHeights[(y<<TEXTURE_POW) + x];
				float height10 = mpHeights[(y<<TEXTURE_POW) + ((x + 1)&TEXTURE_MASK)];
				float height01 = mpHeights[(((y + 1)&TEXTURE_MASK)<<TEXTURE_POW) + x];

				float gradx = height10 - height00;
				float grady = height01 - height00;

				pDst[0] = (VUUINT8)VuRound(gradx*127.0f + 128.0f);
				pDst[1] = (VUUINT8)VuRound(grady*127.0f + 128.0f);

				pDst += 2;
			}
		}
	}
}

//*****************************************************************************
void VuWaterTexture::buildMipLevel(int level)
{
	int levelSrc = level - 1;
	int levelDst = level;
	int sizeSrc = TEXTURE_MIP_SIZE(levelSrc);
	int sizeDst = TEXTURE_MIP_SIZE(levelDst);

	if ( mFormat == VUGFX_FORMAT_LIN_V8U8 )
	{
		VUINT8 *pSrc0 = (VUINT8 *)mpNormals[levelSrc];
		VUINT8 *pSrc1 = (VUINT8 *)mpNormals[levelSrc] + sizeSrc*2;
		VUINT8 *pDst = (VUINT8 *)mpNormals[levelDst];
		for ( int y = 0; y < sizeDst; y++ )
		{
			for ( int x = 0; x < sizeDst; x++ )
			{
				pDst[0] = ((int)pSrc0[0] + (int)pSrc0[2] + (int)pSrc1[0] + (int)pSrc1[2])/4;
				pDst[1] = ((int)pSrc0[1] + (int)pSrc0[3] + (int)pSrc1[1] + (int)pSrc1[3])/4;

				pSrc0 += 4;
				pSrc1 += 4;
				pDst += 2;
			}
			pSrc0 += sizeSrc*2;
			pSrc1 += sizeSrc*2;
		}
	}
	else if ( mFormat == VUGFX_FORMAT_LIN_L8A8 )
	{
		VUUINT8 *pSrc0 = (VUUINT8 *)mpNormals[levelSrc];
		VUUINT8 *pSrc1 = (VUUINT8 *)mpNormals[levelSrc] + sizeSrc*2;
		VUUINT8 *pDst = (VUUINT8 *)mpNormals[levelDst];
		for ( int y = 0; y < sizeDst; y++ )
		{
			for ( int x = 0; x < sizeDst; x++ )
			{
				pDst[0] = ((int)pSrc0[0] + (int)pSrc0[2] + (int)pSrc1[0] + (int)pSrc1[2] + 2) >> 2;
				pDst[1] = ((int)pSrc0[1] + (int)pSrc0[3] + (int)pSrc1[1] + (int)pSrc1[3] + 2) >> 2;
				
				pSrc0 += 4;
				pSrc1 += 4;
				pDst += 2;
			}
			pSrc0 += sizeSrc*2;
			pSrc1 += sizeSrc*2;
		}
	}
}

//*****************************************************************************
void VuWaterTexture::writeNormalsToTexture(int level, VuTexture *pTexture)
{
	int levelSize = TEXTURE_MIP_SIZE(level);

	pTexture->setData(level, mpNormals[level], levelSize*levelSize*2);
}
