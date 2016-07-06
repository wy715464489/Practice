//*****************************************************************************
//
//  Copyright (c) 2012-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Timer Service
// 
//*****************************************************************************

#include "VuService.h"


class VuTimerService : public VuService
{
public:
	VuTimerService();
	~VuTimerService() {}

	class Callback
	{
		public:
			virtual void onTimerServiceTick(VUUINT32 id, float fdt) {}
			virtual void onTimerServiceExpired(VUUINT32 id) {}
	};
	void			init(Callback *pCB, VUUINT32 id, float duration);
	virtual bool	tick(float fdt);

	float			getTimeRemaining() { return mTimeRemaining; }

private:
	Callback			*mpCB;
	VUUINT32			mID;
	float				mTimeRemaining;
};
