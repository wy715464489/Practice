//*****************************************************************************
//
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  GfxSort class
// 
//*****************************************************************************

#pragma once

#include "VuEngine/VuSystemComponent.h"
#include "VuEngine/Containers/VuArray.h"
#include "VuGfxSortMaterial.h"
#include "VuGfxSortMesh.h"

class VuEngine;
class VuShaderProgram;
class VuVertexDeclaration;
class VuPipelineState;
class VuDepthStencilState;
class VuCamera;
class VuGfxSettings;
class VuDevStatPage;
class VuGfxSortDevStat;


class VuGfxSort : public VuSystemComponent
{
	DECLARE_SYSTEM_COMPONENT(VuGfxSort)

protected:

	// called by engine
	friend class VuEngine;
	bool			init(bool bAsynchronous);
	virtual void	postInit();
	virtual void	preRelease();
	virtual void	release();

public:
	VuGfxSort();
	~VuGfxSort();

	// full screen layers
	enum eFullScreenLayer
	{
		FSL_BEGIN,

		FSL_GAME,
		FSL_EFFECTS,
		FSL_HUD,
		FSL_UI,
		FSL_FADE,
		FSL_DIALOG,
		FSL_MESSAGE_BOX,
		FSL_TOAST,
		FSL_WATER_DEBUG,
		FSL_DEV_CONSOLE,
		FSL_DEV_STAT,
		FSL_DEV_PROFILE,
		FSL_DEV_MENU,

		FSL_END
	};

	// reflection layers
	enum eReflectionLayer
	{
		REFLECTION_ON,
		REFLECTION_OFF,
	};

	// viewport layers
	enum eViewportLayer
	{
		VPL_BEGIN,

		VPL_SHADOW1,
		VPL_SHADOW2,
		VPL_SHADOW3,
		VPL_SHADOW4,
		VPL_SSAO,
		VPL_DEPTH,
		VPL_SKYBOX,
		VPL_WORLD,
		VPL_HUD,
		VPL_UI,

		VPL_END
	};

	enum eTranslucencyType
	{
		TRANS_BEGIN,

		TRANS_OPAQUE,
		TRANS_ALPHA_TEST,
		TRANS_FOLIAGE,
		TRANS_SKYBOX,
		TRANS_TIRE_TRACK,
		TRANS_BLOB_SHADOW,
		TRANS_MODULATE_BELOW_WATER,
		TRANS_ADDITIVE_BELOW_WATER,
		TRANS_WATER_COLOR,
		TRANS_DEPTH_PASS,
		TRANS_COLOR_PASS,
		TRANS_MODULATE_ABOVE_WATER,
		TRANS_ADDITIVE_ABOVE_WATER,
		TRANS_WATER_DEPTH,
		TRANS_MODULATE_CLIP_WATER,
		TRANS_ADDITIVE_CLIP_WATER,
		TRANS_UI_OPAQUE,
		TRANS_UI_MODULATE,
		TRANS_UI_ADDITIVE,

		TRANS_END
	};

	void				draw();

	// suspend/resume
	void				suspend();
	void				resume();
	bool				isSuspended() { return mSuspendedCount > 0; }

	// flush any asynchronous rendering
	void				flush();

	// create/release sort materials
	VuGfxSortMaterial	*createMaterial(VuPipelineState *pPipelineState, const VuGfxSortMaterialDesc &desc);
	VuGfxSortMaterial	*duplicateMaterial(VuGfxSortMaterial *pMaterial);
	void				releaseMaterial(VuGfxSortMaterial *pMaterial);

	// create/release sort meshes
	VuGfxSortMesh		*createMesh(const VuGfxSortMeshDesc &desc);
	VuGfxSortMesh		*duplicateMesh(VuGfxSortMesh *pMesh);
	void				releaseMesh(VuGfxSortMesh *pMesh);

	// set/get state (during submission)
	inline void		setScreen(VUUINT screen);
	inline void		setFullScreenLayer(VUUINT fullScreenLayer);
	inline void		setViewport(VUUINT viewport);
	inline void		setReflectionLayer(VUUINT reflectionLayer);
	inline void		setViewportLayer(VUUINT viewportLayer);
	inline VUUINT	getScreen();
	inline VUUINT	getFullScreenLayer();
	inline VUUINT	getViewport();
	inline VUUINT	getReflectionLayer();
	inline VUUINT	getViewportLayer();

	// get state (during rendering)
	inline VUUINT	getRenderScreen();
	inline VUUINT	getRenderFullScreenLayer();
	inline VUUINT	getRenderViewport();
	inline VUUINT	getRenderReflectionLayer();
	inline VUUINT	getRenderViewportLayer();
	inline VUUINT	getRenderTranslucencyType();

	// submit commands
	typedef void (*Callback)(void *data);
	inline void		submitCommand(VUUINT transType, VUUINT sequenceNo, Callback cb);

	// depth-sorted draw command with material/mesh
	template<bool bDepthFirst>
	inline void		submitDrawCommand(VUUINT transType, VuGfxSortMaterial *pMat, VuGfxSortMesh *pMesh, Callback cb, float depth = 0);

	// allocate command memory
	inline void		*allocateCommandMemory(int size, int alignment = 16);

	// resize command memory (must be most recently allocated memory)
	inline void		resizeCommandMemory(void *p, int newSize);

	// translate memory address to/from offset
	inline VUUINT32	translateCommandMemoryPointerToOffset(void *p);
	inline void		*translateCommandMemoryOffsetToPointer(VUUINT32 offset);

	// submit camera (saves camera for current reflection layer, to be accessed during rendering)
	void			submitCamera(const VuCamera &camera, int sequenceNo = 0);
	const VuCamera	&getRenderCamera();

	// submit gfx settings (saves gfx settings for current viewport, to be accessed during rendering)
	void				submitGfxSettings(const VuGfxSettings &settings);
	const VuGfxSettings	&getRenderGfxSettings();

private:
	struct VuCommand
	{
		// flags
		VUUINT64			mSortKey;
		Callback			mCallback;
		VUUINT32			mCallbackDataOffset;
		VuGfxSortMaterial	*mpMaterial;
		VuGfxSortMesh		*mpMesh;
		VUUINT16			mTranslucencyType;
	};

	typedef VuArray<VuGfxSortMaterial *> Materials;
	typedef VuArray<VuGfxSortMesh *> Meshes;
	typedef VuArray<VUBYTE> MemoryBuffer;
	typedef VuArray<VuCommand> CommandBuffer;
	typedef VuArray<VUUINT64> SortedKeys;
	typedef VuArray<int> KeySortOrder;

	static void			threadProc(void *pParam) { static_cast<VuGfxSort *>(pParam)->threadProc(); }
	void				threadProc();

	void				tickDecision(float fdt);

	void				drawFrame();
	void				sortCommands();
	void				submitCommands();
	void				changeMaterial(VuGfxSortMaterial *pOldMat, VuGfxSortMaterial *pNewMat);
	void				changeMesh(VuGfxSortMesh *pOldMesh, VuGfxSortMesh *pNewMesh);
	void				setGlobalConstants(const VuGfxSortMaterial *pMat);
	void				printDevStats();

	int					mSuspendedCount;
	VUHANDLE			mhThread;
	bool				mbWorkerThreadActive;
	Materials			mMaterials;
	int					mMaterialCount;
	Meshes				mMeshes;
	int					mMeshCount;
	VUUINT32			mCommandMemoryOffset;
	MemoryBuffer		mCommandMemory[2];
	CommandBuffer		mCommandBuffer[2];
	int					mCurSubmitBuffer;
	int					mCurRenderBuffer;

	VUUINT64			mCurSubmissionLayerKey;
	VUUINT64			mCurRenderLayerKey;

	SortedKeys			mSortedKeys;
	KeySortOrder		mKeySortOrder;

	// time
	float				mTime;

	// synchronization
	VUHANDLE			mKickWorkEvent;
	VUHANDLE			mWorkCompletedEvent;
	volatile bool		mbTerminateThread;

	// stats
	struct Stats
	{
		int					mMaterialChanges;
		int					mMeshChanges;
		int					mPSChanges;
		int					mConstChanges;
		int					mTextureChanges;
		int					mVBChanges;
		int					mIBChanges;
	};
	Stats				mStatsCur;
	Stats				mStatsPrev;
	VuGfxSortDevStat	*mpDevStat;

	// depth-stencil states
	typedef VuArray<VuDepthStencilState *> DepthStencilStates;
	DepthStencilStates	mDepthStencilStates;

#ifdef VUDEBUG
	VUUINT				mDebugScreen;
	eFullScreenLayer	mDebugFullScreenLayer;
	VUUINT				mDebugViewport;
	eReflectionLayer	mDebugReflectionLayer;
	eViewportLayer		mDebugViewportLayer;

	VUUINT				mDebugRenderScreen;
	eFullScreenLayer	mDebugRenderFullScreenLayer;
	VUUINT				mDebugRenderViewport;
	eReflectionLayer	mDebugRenderReflectionLayer;
	eViewportLayer		mDebugRenderViewportLayer;
	eTranslucencyType	mDebugRenderTranslucencyType;
#endif
};

#include "VuGfxSort.inl"
