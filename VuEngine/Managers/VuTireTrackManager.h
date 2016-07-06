//*****************************************************************************
//
//  Copyright (c) 2012-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Tire Track Manager
// 
//*****************************************************************************

#pragma once

#include "VuEngine/VuSystemComponent.h"
#include "VuEngine/Objects/VuRefObj.h"
#include "VuEngine/Containers/VuArray.h"
#include "VuEngine/Containers/VuList.h"
#include "VuEngine/Math/VuAabb.h"

class VuTireTrack;
class VuTireTrackParams;
class VuTireTrackType;
class VuTireTrackMaterial;
class VuTireTrackSegment;
class VuTireTrackNode;
class VuTireTrackVertex;
class VuVertexDeclaration;
class VuCompiledShaderAsset;
class VuGfxSortMaterial;
class Vu3dDrawComponent;
class VuGfxDrawParams;


class VuTireTrackManager : public VuSystemComponent
{
	DECLARE_SYSTEM_COMPONENT(VuTireTrackManager)

protected:
	// called by engine
	friend class VuEngine;
	virtual bool	init();
	virtual void	release();

public:
	VuTireTrackManager();
	~VuTireTrackManager();

	void			configure(int maxSegmentCount, int maxNodeCount);

	void			reset();

	VuTireTrack		*createTireTrack(const VuTireTrackParams &params);
	void			releaseTireTrack(VuTireTrack *pTireTrack);
	void			updateTireTrack(VuTireTrack *pTireTrack, bool contact, bool skid, VUUINT8 surfaceType, const VuVector3 &pos, const VuVector3 &nor, const VuVector3 &vel, float shadowValue, float scaleX);

protected:
	VuTireTrackSegment		*createSegment();
	void					freeSegment(VuTireTrackSegment *pSegment);
	VuTireTrackNode			*createNode(VuTireTrackSegment *pSegment);
	void					freeNode(VuTireTrackSegment *pSegment, VuTireTrackNode *pNode);
	void					copyNode(VuTireTrackNode *pSrcNode, VuTireTrackNode *pDstNode);
	void					setNode(VuTireTrackNode *pNode, const VuVector3 &pos, const VuVector3 &nor, const VuVector3 &axisX, float width, float dist, float shadowValue);

	void					tick(float fdt);
	void					updateDevStats();

	void					draw(const VuGfxDrawParams &params);
	static void				staticDrawCallback(void *data);
	void					drawCallback(void *data);

	typedef std::list<VuTireTrack *> TireTracks;
	typedef std::map<std::string, VuTireTrackType> TireTrackTypes;
	typedef VuList<VuTireTrackSegment> TireTrackSegments;
	typedef VuList<VuTireTrackNode> TireTrackNodes;

	int						mMaxSegmentCount;
	int						mMaxNodeCount;

	TireTrackTypes			mTireTrackTypes;

	TireTracks				mTireTracks;

	VuTireTrackSegment		*mpSegments;
	TireTrackSegments		mFreeSegments;
	TireTrackSegments		mActiveSegments;

	VuTireTrackNode			*mpNodes;
	TireTrackNodes			mFreeNodes;

	Vu3dDrawComponent		*mp3dDrawComponent;
};

class VuTireTrackParams
{
public:
	VuTireTrackParams() : mWidth(0.25f), mScaleV(1.0f), mMinDist(1.0f), mMinVel(5.0f), mOffsetX(0.0f), mOffsetZ(0.05f), mFadeTime(0.25f), mDrawDist(100.0f), mFadeDist(75.0f), mType("Default") {}

	float	mWidth;
	float	mScaleV;
	float	mMinDist;
	float	mMinVel;
	float	mOffsetX;
	float	mOffsetZ;
	float	mFadeTime;
	float	mDrawDist;
	float	mFadeDist;
	std::string	mType;
};