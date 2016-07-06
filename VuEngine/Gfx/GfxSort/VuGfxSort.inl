//*****************************************************************************
//
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  GfxSort inline implementation
// 
//*****************************************************************************

// screen				- 1 bits
// fullscreen layer		- 5 bits
// viewport				- 2 bits
// reflection layer		- 2 bits
// viewport layer		- 4 bits
// translucency type	- 5 bits
// command				- 1 bits
//
// (modulated alpha)               or  (opaque, additive)              or  (command)
// depth				- 24 bits		material sort key	- 10 bits
// material sort key	- 10 bits		mesh sort key		- 10 bits
// mesh sort key		- 10 bits		depth				- 24 bits		sequence no		- 44 bits

#define GFX_SORT_MASK(bits, shift) ((VUUINT64)((1<<bits) - 1) << shift)

#define GFX_SORT_SCREEN_BITS	1
#define GFX_SORT_SCREEN_SHIFT	63
#define GFX_SORT_SCREEN_MASK	GFX_SORT_MASK(GFX_SORT_SCREEN_BITS, GFX_SORT_SCREEN_SHIFT)

#define GFX_SORT_FULLSCREEN_LAYER_BITS	5
#define GFX_SORT_FULLSCREEN_LAYER_SHIFT	58
#define GFX_SORT_FULLSCREEN_LAYER_MASK	GFX_SORT_MASK(GFX_SORT_FULLSCREEN_LAYER_BITS, GFX_SORT_FULLSCREEN_LAYER_SHIFT)

#define GFX_SORT_VIEWPORT_BITS	3
#define GFX_SORT_VIEWPORT_SHIFT	55
#define GFX_SORT_VIEWPORT_MASK	GFX_SORT_MASK(GFX_SORT_VIEWPORT_BITS, GFX_SORT_VIEWPORT_SHIFT)

#define GFX_SORT_REFLECTION_LAYER_BITS	1
#define GFX_SORT_REFLECTION_LAYER_SHIFT	54
#define GFX_SORT_REFLECTION_LAYER_MASK		GFX_SORT_MASK(GFX_SORT_REFLECTION_LAYER_BITS, GFX_SORT_REFLECTION_LAYER_SHIFT)

#define GFX_SORT_VIEWPORT_LAYER_BITS	4
#define GFX_SORT_VIEWPORT_LAYER_SHIFT	50
#define GFX_SORT_VIEWPORT_LAYER_MASK	GFX_SORT_MASK(GFX_SORT_VIEWPORT_LAYER_BITS, GFX_SORT_VIEWPORT_LAYER_SHIFT)

#define GFX_SORT_TRANSLUCENCY_LAYER_BITS	5
#define GFX_SORT_TRANSLUCENCY_LAYER_SHIFT	45
#define GFX_SORT_TRANSLUCENCY_LAYER_MASK	GFX_SORT_MASK(GFX_SORT_TRANSLUCENCY_LAYER_BITS, GFX_SORT_TRANSLUCENCY_LAYER_SHIFT)

#define GFX_SORT_COMMAND_BITS	1
#define GFX_SORT_COMMAND_SHIFT	44
#define GFX_SORT_COMMAND_MASK	GFX_SORT_MASK(GFX_SORT_COMMAND_BITS, GFX_SORT_COMMAND_SHIFT)

#define GFX_SORT_DEPTH_BITS		22
#define GFX_SORT_DEPTH_MULT		((1<<GFX_SORT_DEPTH_BITS) - 1)
#define GFX_SORT_DEPTH_STEP		((1.0f/GFX_SORT_DEPTH_MULT) + FLT_EPSILON)

#define GFX_SORT_MATERIAL_BITS		11
#define GFX_SORT_MAX_MATERIAL_COUNT	(1<<GFX_SORT_MATERIAL_BITS)

#define GFX_SORT_MESH_BITS		11
#define GFX_SORT_MAX_MESH_COUNT	(1<<GFX_SORT_MESH_BITS)


// misc
#define GFX_SORT_MAX_SCREEN_COUNT (1<<GFX_SORT_SCREEN_BITS)
#define GFX_SORT_MAX_VIEWPORT_COUNT (1<<GFX_SORT_VIEWPORT_BITS)


//*****************************************************************************
void VuGfxSort::setScreen(VUUINT screen)
{
	mCurSubmissionLayerKey &= ~GFX_SORT_SCREEN_MASK;
	mCurSubmissionLayerKey |= (VUUINT64)screen << GFX_SORT_SCREEN_SHIFT;

#ifdef VUDEBUG
	mDebugScreen = screen;
#endif
}

//*****************************************************************************
void VuGfxSort::setFullScreenLayer(VUUINT fullScreenLayer)
{
	mCurSubmissionLayerKey &= ~GFX_SORT_FULLSCREEN_LAYER_MASK;
	mCurSubmissionLayerKey |= (VUUINT64)fullScreenLayer << GFX_SORT_FULLSCREEN_LAYER_SHIFT;

#ifdef VUDEBUG
	mDebugFullScreenLayer = (eFullScreenLayer)fullScreenLayer;
#endif
}

//*****************************************************************************
void VuGfxSort::setViewport(VUUINT viewport)
{
	mCurSubmissionLayerKey &= ~GFX_SORT_VIEWPORT_MASK;
	mCurSubmissionLayerKey |= (VUUINT64)viewport << GFX_SORT_VIEWPORT_SHIFT;

#ifdef VUDEBUG
	mDebugViewport = viewport;
#endif
}

//*****************************************************************************
void VuGfxSort::setReflectionLayer(VUUINT reflectionLayer)
{
	mCurSubmissionLayerKey &= ~GFX_SORT_REFLECTION_LAYER_MASK;
	mCurSubmissionLayerKey |= (VUUINT64)reflectionLayer << GFX_SORT_REFLECTION_LAYER_SHIFT;

#ifdef VUDEBUG
	mDebugReflectionLayer = (eReflectionLayer)reflectionLayer;
#endif
}

//*****************************************************************************
void VuGfxSort::setViewportLayer(VUUINT viewportLayer)
{
	mCurSubmissionLayerKey &= ~GFX_SORT_VIEWPORT_LAYER_MASK;
	mCurSubmissionLayerKey |= (VUUINT64)viewportLayer << GFX_SORT_VIEWPORT_LAYER_SHIFT;

#ifdef VUDEBUG
	mDebugViewportLayer = (eViewportLayer)viewportLayer;
#endif
}

//*****************************************************************************
VUUINT VuGfxSort::getScreen()
{
	return VUUINT((mCurSubmissionLayerKey & GFX_SORT_SCREEN_MASK) >> GFX_SORT_SCREEN_SHIFT);
}

//*****************************************************************************
VUUINT VuGfxSort::getFullScreenLayer()
{
	return VUUINT((mCurSubmissionLayerKey & GFX_SORT_FULLSCREEN_LAYER_MASK) >> GFX_SORT_FULLSCREEN_LAYER_SHIFT);
}

//*****************************************************************************
VUUINT VuGfxSort::getViewport()
{
	return VUUINT((mCurSubmissionLayerKey & GFX_SORT_VIEWPORT_MASK) >> GFX_SORT_VIEWPORT_SHIFT);
}

//*****************************************************************************
VUUINT VuGfxSort::getReflectionLayer()
{
	return VUUINT((mCurSubmissionLayerKey & GFX_SORT_REFLECTION_LAYER_MASK) >> GFX_SORT_REFLECTION_LAYER_SHIFT);
}

//*****************************************************************************
VUUINT VuGfxSort::getViewportLayer()
{
	return VUUINT((mCurSubmissionLayerKey & GFX_SORT_VIEWPORT_LAYER_MASK) >> GFX_SORT_VIEWPORT_LAYER_SHIFT);
}

//*****************************************************************************
VUUINT VuGfxSort::getRenderScreen()
{
	return VUUINT((mCurRenderLayerKey & GFX_SORT_SCREEN_MASK) >> GFX_SORT_SCREEN_SHIFT);
}

//*****************************************************************************
VUUINT VuGfxSort::getRenderFullScreenLayer()
{
	return VUUINT((mCurRenderLayerKey & GFX_SORT_FULLSCREEN_LAYER_MASK) >> GFX_SORT_FULLSCREEN_LAYER_SHIFT);
}

//*****************************************************************************
VUUINT VuGfxSort::getRenderViewport()
{
	return VUUINT((mCurRenderLayerKey & GFX_SORT_VIEWPORT_MASK) >> GFX_SORT_VIEWPORT_SHIFT);
}

//*****************************************************************************
VUUINT VuGfxSort::getRenderReflectionLayer()
{
	return VUUINT((mCurRenderLayerKey & GFX_SORT_REFLECTION_LAYER_MASK) >> GFX_SORT_REFLECTION_LAYER_SHIFT);
}

//*****************************************************************************
VUUINT VuGfxSort::getRenderViewportLayer()
{
	return VUUINT((mCurRenderLayerKey & GFX_SORT_VIEWPORT_LAYER_MASK) >> GFX_SORT_VIEWPORT_LAYER_SHIFT);
}

//*****************************************************************************
VUUINT VuGfxSort::getRenderTranslucencyType()
{
	return VUUINT((mCurRenderLayerKey & GFX_SORT_TRANSLUCENCY_LAYER_MASK) >> GFX_SORT_TRANSLUCENCY_LAYER_SHIFT);
}

//*****************************************************************************
void VuGfxSort::submitCommand(VUUINT transType, VUUINT sequenceNo, Callback cb)
{
	VUUINT64 sortKey = mCurSubmissionLayerKey;
	sortKey |= VUUINT64(transType) << GFX_SORT_TRANSLUCENCY_LAYER_SHIFT;
	sortKey |= VUUINT64(0) << GFX_SORT_COMMAND_SHIFT;
	sortKey |= VUUINT64(sequenceNo);

	mCommandBuffer[mCurSubmitBuffer].resize(mCommandBuffer[mCurSubmitBuffer].size() + 1);
	
	VuCommand *pCmd = &mCommandBuffer[mCurSubmitBuffer].back();

	pCmd->mSortKey = sortKey;
	pCmd->mCallback = cb;
	pCmd->mCallbackDataOffset = mCommandMemoryOffset;
	pCmd->mpMaterial = VUNULL;
	pCmd->mpMesh = VUNULL;
	pCmd->mTranslucencyType = (VUUINT16)transType;
}

//*****************************************************************************
template<bool bDepthFirst>
void VuGfxSort::submitDrawCommand(VUUINT transType, VuGfxSortMaterial *pMat, VuGfxSortMesh *pMesh, Callback cb, float depth)
{
	if ( bDepthFirst )
		depth = (1.0f - depth);
	VUUINT32 depth32 = VUUINT32(depth*GFX_SORT_DEPTH_MULT) & GFX_SORT_DEPTH_MULT;

	VUUINT64 sortKey = mCurSubmissionLayerKey;
	sortKey |= VUUINT64(transType) << GFX_SORT_TRANSLUCENCY_LAYER_SHIFT;
	sortKey |= VUUINT64(1) << GFX_SORT_COMMAND_SHIFT;
	if ( bDepthFirst )
	{
		sortKey |= VUUINT64(depth32) << (GFX_SORT_MATERIAL_BITS + GFX_SORT_MESH_BITS);
		sortKey |= VUUINT64(pMat->mSortKey) << GFX_SORT_MESH_BITS;
		if ( pMesh )
			sortKey |= VUUINT64(pMesh->mSortKey);
	}
	else
	{
		sortKey |= VUUINT64(pMat->mSortKey) << (GFX_SORT_DEPTH_BITS + GFX_SORT_MESH_BITS);
		if ( pMesh )
			sortKey |= VUUINT64(pMesh->mSortKey) << GFX_SORT_DEPTH_BITS;
		sortKey |= VUUINT64(depth32);
	}

	mCommandBuffer[mCurSubmitBuffer].resize(mCommandBuffer[mCurSubmitBuffer].size() + 1);
	
	VuCommand *pCmd = &mCommandBuffer[mCurSubmitBuffer].back();

	pCmd->mSortKey = sortKey;
	pCmd->mCallback = cb;
	pCmd->mCallbackDataOffset = mCommandMemoryOffset;
	pCmd->mpMaterial = pMat;
	pCmd->mpMesh = pMesh;
	pCmd->mTranslucencyType = (VUUINT16)transType;
}

//*****************************************************************************
void *VuGfxSort::allocateCommandMemory(int size, int alignment)
{
	mCommandMemoryOffset = VuAlign(mCommandMemory[mCurSubmitBuffer].size(), alignment);

	mCommandMemory[mCurSubmitBuffer].resize(mCommandMemoryOffset + size);

	return &mCommandMemory[mCurSubmitBuffer][mCommandMemoryOffset];
}

//*****************************************************************************
void VuGfxSort::resizeCommandMemory(void *p, int newSize)
{
	VUASSERT(p == &mCommandMemory[mCurSubmitBuffer][mCommandMemoryOffset], "VuGfxSort::resizeCommandMemory() not most recent allocation");

	mCommandMemory[mCurSubmitBuffer].resize(mCommandMemoryOffset + newSize);
}

//*****************************************************************************
VUUINT32 VuGfxSort::translateCommandMemoryPointerToOffset(void *p)
{
	return (VUUINT32)((VUBYTE *)p - &mCommandMemory[mCurSubmitBuffer].begin());
}

//*****************************************************************************
void *VuGfxSort::translateCommandMemoryOffsetToPointer(VUUINT32 offset)
{
	return &mCommandMemory[mCurSubmitBuffer].begin() + offset;
}
