//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Water renderer class
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Managers/VuViewportManager.h"
#include "VuEngine/Containers/VuArray.h"
#include "VuEngine/Math/VuVector2.h"
#include "VuEngine/Math/VuMatrix.h"

class VuWaterRendererTextureDesc;
class VuWaterRendererParams;
struct WaterRendererDrawData;
class VuWaterSurface;
class VuWaterShader;
class VuWaterSurface;
class VuWaterShader;
class VuWaterTexture;
class VuWaterRenderVertex;
class VuWaterShaderVertex;
class VuCamera;


//*****************************************************************************
// Water renderer interface
//*****************************************************************************
class VuWaterRenderer
{
	friend class VuWater;

protected:
	VuWaterRenderer(bool bAsynchronous);
	~VuWaterRenderer();

public:
	void	setWaterTextureDesc(const VuWaterRendererTextureDesc &desc);

	void	submit(const VuWaterRendererParams &params);

	bool	isBusy();
	void	flush();
	void	kick();
	void	synchronize();

	VuWaterTexture	*getWaterTexture()	{ return mpWaterTexture; }

	struct VuJobData
	{
		const VuWaterSurface	*mpSurface;
		const VuCamera			*mpCamera;
		int						mViewportIndex;
		VuMatrix				mMatModel;
		float					mMaxWaveDepth;
		float					mMaxWaveHeight;
		int						mMinRecursionDepth;
		float					mMinPatchSize;
		float					mDistExtentRatio;
	};

	struct VuPatch
	{
		enum eEdgeFlags { LEFT = 0x1, RIGHT = 0x2, BOTTOM = 0x4, TOP = 0x8 };
		VuVector2	mCenter;
		float		mExtent;
		float		mDist;
		VUUINT32	mEdgeFlags;
	};

	struct VuBuffer
	{
		VuArray<VuVector2>	mVerts;
		VuArray<VUUINT16>	mIndices;
	};

	struct VuRenderStats
	{
		int		mSurfaceCount;
		int		mPatchCount;
		int		mVertexCount;
		int		mIndexCount;
		int		mMaxVertexCount;
		int		mMaxIndexCount;
	};

protected:
	friend struct WaterRendererDrawData;
	void			drawColor(const WaterRendererDrawData *pDrawData);

private:
	typedef VuArray<VuPatch> Patches;
	typedef VuArray<VuWaterShaderVertex> VertexArray;
	typedef VuArray<VUUINT16> IndexArray;

	struct VuPatchBuffer
	{
		VertexArray	mVertexArray;
		IndexArray	mIndexArray;
	};

	struct VuPatchData
	{
		const VuWaterSurface	*mpSurface;
		VuVector3				mSurfacePos;
		float					mSurfaceRotZ;
		VuVector2				mSurfaceSize;
		float					mMaxWaveDepth;
		float					mMaxWaveHeight;
		int						mPatchCount;
		int						mVertexStart;
		int						mVertexCount;
		int						mIndexStart;
		int						mIndexCount;
	};
	typedef VuArray<VuPatchData> PatchData;
	typedef VuArray<VuVector3> DebugVerts;

	void			updateDevStats();
	void			buildBuffers();
	VUUINT16		addVert(VuBuffer *pBuffer, float x, float y);
	void			addTri(VuBuffer *pBuffer, VUUINT16 i0, VUUINT16 i1, VUUINT16 i2);
	void			buildPatches();
	void			buildPatches(int clipDepth, int clipX, int clipY, const VuVector2 &vCenter, float fExtent);
	void			calcEdgeFlags();
	void			drawPatches();
	void			drawNormals(VuWaterRenderVertex *pVerts, int vertCount);
	void			drawFlow(VuWaterRenderVertex *pVerts, int vertCount);

	static void		threadProc(void *pParam) { static_cast<VuWaterRenderer *>(pParam)->threadProc(); }
	void			threadProc();
	void			buildSurfaceDrawData();
	void			buildVertexIndexData(VertexArray &vertexArray, IndexArray &indexArray);
	void			addWaterMapInfluence(VuBuffer *pBuffer, VuPatch *pPatch, VuWaterShaderVertex *pVerts, int vertCount, const VuWaterSurface *pSurface);
	void			addLightMapInfluence(VuBuffer *pBuffer, VuPatch *pPatch, VuWaterShaderVertex *pVerts, int vertCount, const VuWaterSurface *pSurface);

	VuWaterTexture	*mpWaterTexture;
	VuJobData		mJobData;
	Patches			mPatches;
	bool			mbDebugNormals;
	bool			mbDebugFlow;
	bool			mClipMaps;
	VuBuffer		mBuffers[16];
	VuRenderStats	mStats;
	VuPatchBuffer	mPatchBuffers[2];
	PatchData		mPatchData[2][VuViewportManager::MAX_VIEWPORTS];
	int				mCurSubmitPatchBuffer;
	int				mCurRenderPatchBuffer;
	DebugVerts		mDebugVerts;

	// synchronization
	VUHANDLE			mhThread;
	bool				mbAsynchronousWaterRenderer;
	bool				mbWorkerThreadActive;
	bool				mbTerminateThread;
	float				mWaterRendererOverlapTime;
	VUHANDLE			mWorkAvailableEvent;
	VUHANDLE			mWorkCompletedEvent;
};


//*****************************************************************************
// Parameters used for rendering
//*****************************************************************************
class VuWaterRendererParams
{
public:
	VuWaterRendererParams(const VuWaterSurface *pSurface, const VuWaterShader *pShader, const VuCamera *pCamera):
		mpSurface(pSurface),
		mpShader(pShader),
		mpCamera(pCamera)
	{}

	const VuWaterSurface	*mpSurface;
	const VuWaterShader		*mpShader;
	const VuCamera			*mpCamera;
};

//*****************************************************************************
// Description used to configure water texture.
//*****************************************************************************
class VuWaterRendererTextureDesc
{
public:
	VuWaterRendererTextureDesc():
		mGravity(9.801f),
		mWorldSize(10.0f),
		mWindSpeed(10.0f),
		mDirectionalPower(0.0f),
		mSuppressionWaveLength(0.0f),
		mHeightFactor(1.0f),
		mTimeFactor(1.0f),
		mNormalTextureScale(0.5f)
	{}

	float	mGravity;				// m/s^2
	float	mWorldSize;				// meters
	float	mWindSpeed;				// m/s
	float	mDirectionalPower;
	float	mSuppressionWaveLength;	// meters
	float	mHeightFactor;
	float	mTimeFactor;
	float	mNormalTextureScale;	// multiplied by foam texture size
};