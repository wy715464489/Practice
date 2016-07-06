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
#include "VuEngine/Math/VuMathUtil.h"
#include "VuEngine/Dev/VuDev.h"


class VuTriggerPlaneEntity : public VuTriggerEntity
{
	DECLARE_RTTI

public:
	VuTriggerPlaneEntity();

private:
	virtual void		update();
	virtual void		draw();
	virtual void		drawLayout(const Vu3dLayoutDrawParams &params);
};

IMPLEMENT_RTTI(VuTriggerPlaneEntity, VuTriggerEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuTriggerPlaneEntity);


//*****************************************************************************
VuTriggerPlaneEntity::VuTriggerPlaneEntity()
{
	mpTransformComponent->setMask(VuTransformComponent::TRANS|VuTransformComponent::ROT|VuTransformComponent::SCALE_X|VuTransformComponent::SCALE_Z);
	mp3dLayoutComponent->setLocalBounds(VuAabb(VuVector3(-1, 0, -1), VuVector3(1, 0, 1)));
}

//*****************************************************************************
void VuTriggerPlaneEntity::update()
{
	const VuMatrix &mat = mpTransformComponent->getWorldTransform();
	VuVector4 plane = VuMathUtil::planeFromNormalPoint(mat.getAxisY(), mat.getTrans());
	const VuVector3 extents = mpTransformComponent->getWorldScale();

	const VuTriggerManager::Instigators &instigators = VuTriggerManager::IF()->getInstigators();
	for ( const VuTriggerManager::VuInstigatorEntry *pi = &instigators.begin(); pi != &instigators.end(); pi++ )
	{
		// this may be optimized by tracking which insigators affect which trigger entities
		if ( pi->mMask & mTriggerMask )
		{
			// quick test for plane crossing
			float fDistPrev = VuMathUtil::distPointPlane(pi->mPrevPos, plane);
			float fDistCur = VuMathUtil::distPointPlane(pi->mCurPos, plane);
			if ( fDistPrev*fDistCur <= 0.0f )
			{
				// more detailed test, taking into account edge case of one point being exactly on the plane
				bool bFrontPrev = fDistPrev > 0.0f;
				bool bFrontCur = fDistCur > 0.0f;
				if ( bFrontPrev != bFrontCur )
				{
					// determine intersection
					float num = VuDot(mat.getAxisY(), mat.getTrans() - pi->mPrevPos);
					float den = VuDot(mat.getAxisY(), pi->mCurPos - pi->mPrevPos);
					float u = num/den;
					VuVector3 inter = pi->mPrevPos + u*(pi->mCurPos - pi->mPrevPos);
					float radius = pi->mPrevRadius + u*pi->mCurRadius;

					// calculate lateral distance of intersection
					VuVector3 relPos = inter - mat.getTrans();
					float maxDist = VuAbs(VuDot(relPos, mat.getAxisX())) - extents.mX;
					maxDist = VuMax(maxDist, VuAbs(VuDot(relPos, mat.getAxisZ())) - extents.mZ);
					if ( maxDist < radius )
					{
						doTrigger(pi->mpInstigatorComponent->getOwnerEntity(), bFrontPrev);
					}
				}
			}
		}
	}
}

//*****************************************************************************
void VuTriggerPlaneEntity::draw()
{
	VuMatrix mat = mpTransformComponent->getWorldTransform();
	mat.scaleLocal(mpTransformComponent->getWorldScale());

	VuVector3 v0 = mat.getTrans() - mat.getAxisX() - mat.getAxisZ();
	VuVector3 v1 = mat.getTrans() + mat.getAxisX() - mat.getAxisZ();
	VuVector3 v2 = mat.getTrans() - mat.getAxisX() + mat.getAxisZ();
	VuVector3 v3 = mat.getTrans() + mat.getAxisX() + mat.getAxisZ();
	VuVector3 v5 = mat.getTrans() + mat.getAxisY();
	VuColor color(255,255,0);

	VuDev::IF()->drawLine(v0, v1, color);
	VuDev::IF()->drawLine(v0, v2, color);
	VuDev::IF()->drawLine(v1, v3, color);
	VuDev::IF()->drawLine(v2, v3, color);
	VuDev::IF()->drawLine(v0, v2, color);
	VuDev::IF()->drawLine(v1, v3, color);
	VuDev::IF()->drawLine(mat.getTrans(), v5, color);
}

//*****************************************************************************
void VuTriggerPlaneEntity::drawLayout(const Vu3dLayoutDrawParams &params)
{
	if ( params.mbSelected )
	{
		VuMatrix mat = mpTransformComponent->getWorldTransform();
		VuGfxUtil::IF()->drawArrowLines(VuColor(128, 255, 128), 2.0f, 1.0f, 1.0f, mat*params.mCamera.getViewProjMatrix());

		mat.scaleLocal(mpTransformComponent->getWorldScale());
		VuGfxUtil::IF()->drawAabbSolid(VuColor(128,255,128,128), mp3dLayoutComponent->getLocalBounds(), mat, params.mCamera.getViewProjMatrix());
	}
}
