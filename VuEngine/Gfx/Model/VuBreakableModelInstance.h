//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Breakable Model Instance
// 
//*****************************************************************************

#pragma once

#include "VuStaticModelInstance.h"
#include "VuEngine/Math/VuMatrix.h"


class VuBreakableModelInstance : public VuStaticModelInstance
{
public:
	VuBreakableModelInstance();
	~VuBreakableModelInstance();

	virtual void	onSetModel();
	virtual void	onReset();

	void			initializePieces(const VuMatrix &modelMat, const VuVector3 &vLinVel);
	void			updatePieces(float fdt, VuAabb &aabb);
	void			drawPieces(const VuGfxDrawParams &params) const;

	VuVector3		mMinPieceLinVel;
	VuVector3		mMaxPieceLinVel;
	VuVector3		mMinPieceAngVel;
	VuVector3		mMaxPieceAngVel;
	float			mMinVelocityDamping;
	float			mMaxVelocityDamping;
	float			mGravity;

private:
	// pieces
	class VuPiece
	{
	public:
		VuGfxSceneNode	*mpGfxSceneNode;
		VuMatrix		mNodeInvTransform;
		VuVector3		mNodePivot;
		VuVector3		mWorldPos;
		VuVector3		mWorldRot;
		VuVector3		mWorldLinVel;
		VuVector3		mWorldAngVel;
		VuMatrix		mTransform;
		float			mVelocityDamping;
	};
	int			mPieceCount;
	VuPiece		*mpPieces;
};
