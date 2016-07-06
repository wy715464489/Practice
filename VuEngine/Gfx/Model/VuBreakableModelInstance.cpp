//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Breakable Model Instance
// 
//*****************************************************************************

#include "VuBreakableModelInstance.h"
#include "VuEngine/Gfx/GfxScene/VuGfxStaticScene.h"
#include "VuEngine/Gfx/GfxScene/VuGfxSceneNode.h"
#include "VuEngine/Math/VuRand.h"
#include "VuEngine/Math/VuMathUtil.h"


//*****************************************************************************
VuBreakableModelInstance::VuBreakableModelInstance():
	mMinPieceLinVel(-5.0f, -5.0f,  5.0f),
	mMaxPieceLinVel( 5.0f,  5.0f, 10.0f),
	mMinPieceAngVel(-VU_PI, -VU_PI, -VU_PI),
	mMaxPieceAngVel( VU_PI,  VU_PI,  VU_PI),
	mMinVelocityDamping(0.0f),
	mMaxVelocityDamping(0.0f),
	mGravity(9.806f),
	mPieceCount(0),
	mpPieces(VUNULL)
{
}

//*****************************************************************************
VuBreakableModelInstance::~VuBreakableModelInstance()
{
	delete[] mpPieces;
}

//*****************************************************************************
void VuBreakableModelInstance::onSetModel()
{
	if ( VuGfxStaticScene *pScene = getGfxStaticScene() )
	{
		mPieceCount = (int)pScene->mNodes.size();
		if ( mPieceCount )
		{
			mpPieces = new VuPiece[mPieceCount];
			memset(mpPieces, 0, sizeof(mpPieces[0])*mPieceCount);
			VuPiece *pPiece = mpPieces;
			for ( VuGfxStaticScene::Nodes::iterator iter = pScene->mNodes.begin(); iter != pScene->mNodes.end(); iter++ )
			{
				pPiece->mpGfxSceneNode = *iter;
				pPiece->mNodeInvTransform = (*iter)->mTransform;
				pPiece->mNodeInvTransform.invert();
				pPiece->mNodePivot = (*iter)->mAabb.getCenter();
				pPiece++;
			}
		}
	}
}

//*****************************************************************************
void VuBreakableModelInstance::onReset()
{
	delete[] mpPieces;
	mpPieces = VUNULL;
	mPieceCount = 0;
}

//*****************************************************************************
void VuBreakableModelInstance::initializePieces(const VuMatrix &modelMat, const VuVector3 &vLinVel)
{
	VuMatrix impactMat;
	VuMathUtil::buildOrientationMatrix(vLinVel, VuVector3(0,0,1), impactMat);

	// pieces don't affect gameplay, so we're not worried about synchronized random seeds
	for ( int i = 0; i < mPieceCount; i++ )
	{
		VuPiece *pPiece = &mpPieces[i];

		VuMatrix mat = pPiece->mpGfxSceneNode->mTransform*modelMat;

		pPiece->mWorldPos = mat.transform(pPiece->mNodePivot);
		pPiece->mWorldRot = mat.getEulerAngles();

		VuVector3 localLinVel;
		localLinVel.mX = VuLerp(mMinPieceLinVel.mX, mMaxPieceLinVel.mX, VuRand::global().rand());
		localLinVel.mY = VuLerp(mMinPieceLinVel.mY, mMaxPieceLinVel.mY, VuRand::global().rand());
		localLinVel.mZ = VuLerp(mMinPieceLinVel.mZ, mMaxPieceLinVel.mZ, VuRand::global().rand());
		pPiece->mWorldLinVel = impactMat.transformNormal(localLinVel);

		VuVector3 localAngVel;
		localAngVel.mX = VuLerp(mMinPieceAngVel.mX, mMaxPieceAngVel.mX, VuRand::global().rand());
		localAngVel.mY = VuLerp(mMinPieceAngVel.mY, mMaxPieceAngVel.mY, VuRand::global().rand());
		localAngVel.mZ = VuLerp(mMinPieceAngVel.mZ, mMaxPieceAngVel.mZ, VuRand::global().rand());
		pPiece->mWorldAngVel = modelMat.transformNormal(localAngVel);

		pPiece->mVelocityDamping = VuLerp(mMinVelocityDamping, mMaxVelocityDamping, VuRand::global().rand());

		pPiece->mWorldLinVel += vLinVel;

		pPiece->mTransform.setEulerAngles(pPiece->mWorldRot);
		pPiece->mTransform.setTrans(pPiece->mWorldPos);
		pPiece->mTransform.translateLocal(-pPiece->mNodePivot);
		pPiece->mTransform = pPiece->mNodeInvTransform*pPiece->mTransform;
	}
}

//*****************************************************************************
void VuBreakableModelInstance::updatePieces(float fdt, VuAabb &aabb)
{
	VuVector3 vGravity(0, 0, -mGravity);
	for ( int i = 0; i < mPieceCount; i++ )
	{
		VuPiece *pPiece = &mpPieces[i];

		float damping = VuMin(pPiece->mVelocityDamping*fdt, 1.0f);

		pPiece->mWorldPos += pPiece->mWorldLinVel*fdt + (0.5f*fdt*fdt)*vGravity;
		pPiece->mWorldLinVel += vGravity*fdt;
		pPiece->mWorldLinVel *= (1.0f - damping);
		pPiece->mWorldRot += pPiece->mWorldAngVel*fdt;

		pPiece->mTransform.setEulerAngles(pPiece->mWorldRot);
		pPiece->mTransform.setTrans(pPiece->mWorldPos);
		pPiece->mTransform.translateLocal(-pPiece->mNodePivot);
		pPiece->mTransform = pPiece->mNodeInvTransform*pPiece->mTransform;

		aabb.addAabb(pPiece->mpGfxSceneNode->mAabb, pPiece->mTransform);
	}
}

//*****************************************************************************
void VuBreakableModelInstance::drawPieces(const VuGfxDrawParams &params) const
{
	for ( int i = 0; i < mPieceCount; i++ )
		drawRecursive(mpPieces[i].mpGfxSceneNode, mpPieces[i].mTransform, params);
}
