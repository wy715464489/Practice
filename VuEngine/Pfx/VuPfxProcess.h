//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Pfx Process
// 
//*****************************************************************************

#pragma once

#include "VuPfxNode.h"
#include "VuEngine/Containers/VuList.h"

class VuJsonContainer;
class VuPfxSystemInstance;
class VuPfxPatternInstance;


class VuPfxProcess : public VuPfxNode
{
	DECLARE_RTTI

protected:
	~VuPfxProcess() {}

public:
	virtual int		instanceSize() const = 0;
	virtual void	constructInstance(void *p) const = 0;
};

class VuPfxProcessInstance : public VuListElement<VuPfxProcessInstance>
{
public:
	void					setParams(VuPfxProcess *pParams)	{ mpParams = pParams; }
	virtual void			start() {}
	virtual void			tick(float fdt, bool ui) = 0;

	VuPfxPatternInstance	*mpPatternInstance;
	VuPfxProcess			*mpParams;
};


//*****************************************************************************
// Macro used by the various process implementations to declare their creation
// functions.
#define IMPLEMENT_PFX_PROCESS_REGISTRATION(type)							\
	VuPfxProcess *Create##type() { return new type; }						\
	int type::instanceSize() const { return sizeof(type##Instance); }		\
	void type::constructInstance(void *p) const { new (p) type##Instance; }


//*****************************************************************************
// Macro used to declare instance functionality in the class definition.
#define DECLARE_PFX_PROCESS						\
public:											\
	int		instanceSize() const;				\
	void	constructInstance(void *p) const;	\
private:

