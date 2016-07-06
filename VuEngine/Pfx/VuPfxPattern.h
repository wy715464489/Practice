//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Pfx Pattern
// 
//*****************************************************************************

#pragma once

#include "VuPfxNode.h"
#include "VuEngine/Containers/VuList.h"
#include "VuEngine/Math/VuAabb.h"

class VuPfxSystemInstance;
class VuPfxProcess;
class VuPfxProcessInstance;
class VuPfxParticle;
class VuGfxDrawParams;
class VuGfxDrawShadowParams;


class VuPfxPattern : public VuPfxNode
{
	DECLARE_RTTI

protected:
	~VuPfxPattern() {}

public:
	VuPfxPattern();

	virtual int			instanceSize() const = 0;
	virtual void		constructInstance(void *p) const = 0;
	virtual int			particleSize() const = 0;
	virtual void		constructParticle(void *p) const = 0;

	enum eSpace { WORLD_SPACE, OBJECT_SPACE };
	int					mMaxParticleCount;
	int					mSpace;
	float				mStartDelay;
};

class VuPfxPatternInstance : public VuListElement<VuPfxPatternInstance>
{
public:
	VuPfxPatternInstance();

	bool					create();
	virtual void			destroy();
	virtual void			destroyParticles();
	virtual void			start() = 0;
	virtual void			tick(float fdt, bool ui) = 0;
	virtual void			draw(const VuGfxDrawParams &params) = 0;
	virtual void			drawShadow(const VuGfxDrawShadowParams &params) {};
	virtual VuPfxParticle	*createParticle();
	virtual const VuMatrix	&getSpawnTransform() const;
	virtual const VuMatrix	&getDrawTransform() const;

	typedef VuList<VuPfxProcessInstance> Processes;
	typedef VuList<VuPfxParticle> Particles;

	VuPfxSystemInstance		*mpSystemInstance;
	VuPfxPattern			*mpParams;
	Processes				mProcesses;
	Particles				mParticles;
	VuAabb					mAabb;
};


//*****************************************************************************
// Macro used by the various pattern implementations to declare their creation
// functions.
#define IMPLEMENT_PFX_PATTERN_REGISTRATION(type, particleType)				\
	VuPfxPattern *Create##type() { return new type; }						\
	int type::instanceSize() const { return sizeof(type##Instance); }		\
	void type::constructInstance(void *p) const { new (p) type##Instance; }	\
	int type::particleSize() const { return sizeof(particleType); }			\
	void type::constructParticle(void *p) const { new (p) particleType; }


//*****************************************************************************
// Macro used to declare instance functionality in the class definition.
#define DECLARE_PFX_PATTERN						\
public:											\
	int		instanceSize() const;				\
	void	constructInstance(void *p) const;	\
	int		particleSize() const;				\
	void	constructParticle(void *p) const;	\
private:

