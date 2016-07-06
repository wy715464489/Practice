//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Trigger entity
// 
//*****************************************************************************

#include "VuTriggerEntity.h"
#include "VuEngine/Components/Transform/VuTransformComponent.h"
#include "VuEngine/Components/Instigator/VuInstigatorComponent.h"
#include "VuEngine/Components/Script/VuScriptComponent.h"
#include "VuEngine/Components/3dLayout/Vu3dLayoutComponent.h"
#include "VuEngine/Managers/VuTriggerManager.h"
#include "VuEngine/Gfx/VuGfxUtil.h"
#include "VuEngine/Gfx/Camera/VuCamera.h"
#include "VuEngine/Util/VuColor.h"
#include "VuEngine/Dev/VuDev.h"


class VuTriggerBoxEntity : public VuTriggerEntity
{
	DECLARE_RTTI

private:
	virtual void		update();
	virtual void		draw();
	virtual void		drawLayout(const Vu3dLayoutDrawParams &params);

	inline float		dist(const VuVector3 &vPos, float fRadius);
};

IMPLEMENT_RTTI(VuTriggerBoxEntity, VuTriggerEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuTriggerBoxEntity);


//*****************************************************************************
void VuTriggerBoxEntity::update()
{
	const VuTriggerManager::Instigators &instigators = VuTriggerManager::IF()->getInstigators();
	for ( const VuTriggerManager::VuInstigatorEntry *pi = &instigators.begin(); pi != &instigators.end(); pi++ )
	{
		// this may be optimized by tracking which insigators affect which trigger entities
		if ( pi->mMask & mTriggerMask )
		{
			// quick test for boundary crossing
			float fDistPrev = dist(pi->mPrevPos, pi->mPrevRadius);
			float fDistCur = dist(pi->mCurPos, pi->mCurRadius);
			if ( fDistPrev*fDistCur <= 0.0f )
			{
				// more detailed test, taking into account edge case of one point being exactly on the boundary
				bool bInsidePrev = fDistPrev < 0.0f;
				bool bInsideCur = fDistCur < 0.0f;
				if ( bInsidePrev != bInsideCur )
				{
					doTrigger(pi->mpInstigatorComponent->getOwnerEntity(), bInsideCur);
				}
			}
		}
	}
}

//*****************************************************************************
void VuTriggerBoxEntity::draw()
{
	VuMatrix mat = mpTransformComponent->getWorldTransform();
	mat.scaleLocal(2.0f*mpTransformComponent->getWorldScale());

	VuDev::IF()->drawBox(mat, VuColor(255,255,0));
}

//*****************************************************************************
void VuTriggerBoxEntity::drawLayout(const Vu3dLayoutDrawParams &params)
{
	if ( params.mbSelected )
	{
		VuMatrix mat = mpTransformComponent->getWorldTransform();
		mat.scaleLocal(mpTransformComponent->getWorldScale());

		VuGfxUtil::IF()->drawAabbSolid(VuColor(128,255,128,128), mp3dLayoutComponent->getLocalBounds(), mat, params.mCamera.getViewProjMatrix());
	}
}

//*****************************************************************************
inline float VuTriggerBoxEntity::dist(const VuVector3 &vPos, float fRadius)
{
	const VuMatrix &mat = mpTransformComponent->getWorldTransform();
	const VuVector3 &extents = mpTransformComponent->getWorldScale();

	VuVector3 relPos = vPos - mat.getTrans();
	float maxDist = VuAbs(VuDot(relPos, mat.getAxisX())) - extents.mX;
	maxDist = VuMax(maxDist, VuAbs(VuDot(relPos, mat.getAxisY())) - extents.mY);
	maxDist = VuMax(maxDist, VuAbs(VuDot(relPos, mat.getAxisZ())) - extents.mZ);

	return maxDist - fRadius;
}
