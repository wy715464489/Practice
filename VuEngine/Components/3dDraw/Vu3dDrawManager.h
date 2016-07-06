//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  3dDrawManager class
// 
//*****************************************************************************

#pragma once

#include "VuEngine/VuSystemComponent.h"
#include "VuEngine/Gfx/VuGfxDrawParams.h"
#include "VuEngine/Containers/VuArray.h"
#include "VuEngine/Method/VuMethod.h"

class VuEngine;
class Vu3dDrawComponent;
class VuDbvt;


class Vu3dDrawManager : public VuSystemComponent
{
	DECLARE_SYSTEM_COMPONENT(Vu3dDrawManager)

protected:
	// called by engine
	friend class VuEngine;
	virtual bool	init()	{ return true; }

public:
	Vu3dDrawManager();
	~Vu3dDrawManager();

	virtual void	draw(const VuGfxDrawParams &params);
	virtual void	drawShadow(const VuGfxDrawShadowParams &params);

	typedef VuMethodInterface0<void> PrefetchMethod;
	virtual void	addPrefetchMethod(PrefetchMethod *pMethod);
	virtual void	removePrefetchMethod(PrefetchMethod *pMethod);

	// zones
	VUUINT32		calcZoneMask(const VuVector3 &pos);

	class VuZoneMaskIF { public: virtual VUUINT32 calcMask(const VuVector3 &pos) const = 0; };
	void			addZoneMask(const VuZoneMaskIF *pIF) { mZoneMasks.push_back(pIF); }
	void			removeZoneMask(const VuZoneMaskIF *pIF) { mZoneMasks.removeSwap(pIF); }

protected:
	friend class Vu3dDrawComponent;

	virtual void		add(Vu3dDrawComponent *p3dDrawComponent);
	virtual void		remove(Vu3dDrawComponent *p3dDrawComponent);
	virtual void		updateVisibility(Vu3dDrawComponent *p3dDrawComponent);

private:
	void				drawPrefetch();
	void				updateDevStats(float fdt);

	VuDbvt				*mpDbvt;

	// stats
	int					mDrawPassed;
	int					mDrawRejected;
	int					mShadowPassed;
	int					mShadowRejected;

	// prefetch
	typedef VuArray<PrefetchMethod *> PrefetchQueue;
	PrefetchQueue		mPrefetchQueue;

	// zones
	typedef VuArray<const VuZoneMaskIF *> ZoneMasks;
	ZoneMasks			mZoneMasks;

	struct VuDrawPolicy;
	struct VuDrawShadowPolicy;
};
