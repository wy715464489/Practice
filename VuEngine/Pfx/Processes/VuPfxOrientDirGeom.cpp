//*****************************************************************************
//
//  Copyright (c) 2014-2014 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Pfx Orient Dir Geom Process
// 
//*****************************************************************************

#include "VuEngine/Pfx/VuPfxProcess.h"
#include "VuEngine/Pfx/VuPfxPattern.h"
#include "VuEngine/Pfx/Particles/VuPfxGeomParticle.h"
#include "VuEngine/Math/VuMathUtil.h"
#include "VuEngine/Math/VuMatrix.h"


class VuPfxOrientDirGeom : public VuPfxProcess
{
	DECLARE_RTTI
	DECLARE_PFX_PROCESS

public:
	VuPfxOrientDirGeom();
};

class VuPfxOrientDirGeomInstance : public VuPfxProcessInstance
{
public:
	virtual void		tick(float fdt, bool ui);
};

IMPLEMENT_RTTI(VuPfxOrientDirGeom, VuPfxProcess);
IMPLEMENT_PFX_PROCESS_REGISTRATION(VuPfxOrientDirGeom);


//*****************************************************************************
VuPfxOrientDirGeom::VuPfxOrientDirGeom()
{
}

//*****************************************************************************
void VuPfxOrientDirGeomInstance::tick(float fdt, bool ui)
{
	const VuPfxOrientDirGeom *pParams = static_cast<const VuPfxOrientDirGeom *>(mpParams);

	for ( VuPfxParticle *p = mpPatternInstance->mParticles.front(); p; p = p->next() )
	{
		VuPfxGeomParticle *pgp = static_cast<VuPfxGeomParticle *>(p);

		VuMatrix transform;
		transform.setEulerAngles(pgp->mRotation);
		VuMathUtil::buildOrientationMatrix(pgp->mLinearVelocity, transform.getAxisZ(), transform);

		pgp->mRotation = transform.getEulerAngles();
		pgp->mAngularVelocity.set(0,0,0);
	}
}
