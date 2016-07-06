//*****************************************************************************
//
//  Copyright (c) 2012-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Pfx Attach Service
// 
//*****************************************************************************

#include "VuPfxAttachService.h"
#include "VuEngine/Components/Transform/VuTransformComponent.h"
#include "VuEngine/Gfx/Model/VuAnimatedModelInstance.h"
#include "VuEngine/Pfx/VuPfxManager.h"
#include "VuEngine/Pfx/VuPfxEntity.h"
#include "VuEngine/Pfx/VuPfx.h"


//*****************************************************************************
VuPfxAttachService::VuPfxAttachService():
	mhPfx(0)
{
}

//*****************************************************************************
void VuPfxAttachService::init(VuEntity *pParent, VUUINT32 hPfx, const VuMatrix &transform, VuAnimatedModelInstance *pModelInstance, int boneIndex)
{
	mParent = pParent;
	mhPfx = hPfx;
	mTransform = transform;
	mpModelInstance = pModelInstance;
	mBoneIndex = boneIndex;

	tick(0.0f);
}

//*****************************************************************************
bool VuPfxAttachService::tick(float fdt)
{
	VuPfxEntity *pPfxEntity = VuPfxManager::IF()->getEntity(mhPfx);

	if ( mParent.get() && pPfxEntity )
	{
		VuMatrix transform = mTransform;

		if (mBoneIndex >= 0)
			transform *= mpModelInstance->getModelMatrices()[mBoneIndex];

		transform *= mParent.get()->getTransformComponent()->getWorldTransform();

		pPfxEntity->getSystemInstance()->setMatrix(transform);

		return true;
	}

	return false;
}
