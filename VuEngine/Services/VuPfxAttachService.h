//*****************************************************************************
//
//  Copyright (c) 2012-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Pfx Attach Service
// 
//*****************************************************************************

#include "VuService.h"
#include "VuEngine/Math/VuMatrix.h"
#include "VuEngine/Objects/VuRefObj.h"

class VuEntity;
class VuAnimatedModelInstance;


class VuPfxAttachService : public VuService
{
public:
	VuPfxAttachService();
	~VuPfxAttachService() {}

	void			init(VuEntity *pParent, VUUINT32 hPfx, const VuMatrix &transform, VuAnimatedModelInstance *pModelInstance = VUNULL, int boneIndex = -1);
	virtual bool	tick(float fdt);

private:
	VuMatrix				mTransform;
	VuWeakRef<VuEntity>		mParent;
	VUUINT32				mhPfx;
	VuAnimatedModelInstance	*mpModelInstance;
	int						mBoneIndex;
};
