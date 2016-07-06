//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Allows objects to be owned by multiple owners.  Also manages weak
//  references to the object.
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Objects/VuBaseObj.h"


//*****************************************************************************
// Referenced Object
//*****************************************************************************
class VuRefObj : public VuBaseObj
{
public:
	class VuRefListElement
	{
	public:
		VuRefListElement() : mpPrev(VUNULL), mpNext(VUNULL) {}

		virtual void release() = 0;

		VuRefListElement *mpPrev;
		VuRefListElement *mpNext;
	};

	VuRefObj()						: mRefCount(1), mpWeakRefRoot(VUNULL) 	{}
	VuRefObj(const VuRefObj &other)	: mRefCount(1), mpWeakRefRoot(VUNULL)	{}
protected:
	inline virtual ~VuRefObj();
public:

	inline int	addRef();
	inline int	removeRef();
	inline int	refCount();

	inline void addWeakRef(VuRefListElement *pWeakRef);
	inline void removeWeakRef(VuRefListElement *pWeakRef);

private:
	// reference count
	VUINT32				mRefCount;

	// references
	VuRefListElement	*mpWeakRefRoot;
};

// safe release macro
#define VU_SAFE_RELEASE(pRefObj)\
{								\
	if ( pRefObj )				\
	{							\
		pRefObj->removeRef();	\
		pRefObj = VUNULL;		\
	}							\
}


//*****************************************************************************
// Weak Reference
//*****************************************************************************
template<class T>
class VuWeakRef : protected VuRefObj::VuRefListElement
{
public:
	VuWeakRef()					: mpObj(VUNULL) {}
	~VuWeakRef()								{ release(); }
	explicit VuWeakRef(T *pObj)	: mpObj(VUNULL) { internalAssign(pObj); }

	// accessors
	T				&operator *() const		{ VUASSERT(mpObj, "VuWeakRef::*() null pointer"); return *mpObj; }
	T				*operator ->() const	{ VUASSERT(mpObj, "VuWeakRef::->() null pointer"); return &*(mpObj); }
	T				*get() const			{ return mpObj; }
	operator bool	() const				{ return mpObj ? true : false; }

	// construct from a weak reference
	template<class T_Other>
	VuWeakRef(const VuWeakRef<T_Other> &other)	: mpObj(VUNULL) { internalAssign(other.get()); }
	VuWeakRef(const VuWeakRef<T> &other)		: mpObj(VUNULL) { internalAssign(other.get()); }

	// assign a weak reference to a weak reference
	template<class T_Other>
	void operator = (const VuWeakRef<T_Other> &other)	{ internalAssign(other.get()); }
	void operator = (const VuWeakRef<T> &other)			{ internalAssign(other.get()); }

	// assign a strong reference to a weak reference
	template<class T_Other>
	void operator = (T_Other *pObj)	{ internalAssign(pObj); }
	void operator = (T *pObj)		{ internalAssign(pObj); }

	// equality
	template<class T_Other>
	bool operator == (const VuWeakRef<T_Other> &other)	{ return mpObj == other.get(); }
	bool operator == (const VuWeakRef<T> &other)		{ return mpObj == other.get(); }
	template<class T_Other>
	bool operator == (T_Other *pObj)					{ return mpObj == pObj; }
	bool operator == (T *pObj)							{ return mpObj == pObj; }

	// release this weak pointer
	inline void release();

private:

	// used by assignment operators
	template<class T_Other>
	inline void internalAssign(T_Other *pObj);

	T			*mpObj;
};


//*****************************************************************************
// VuRefObj implementation
//*****************************************************************************

//*****************************************************************************
// release all remaining weak references
//*****************************************************************************
VuRefObj::~VuRefObj()
{
	VUASSERT(mRefCount == 0, "VuRefObj::~VuRefObj() dangling ref count");

	while( mpWeakRefRoot )
		mpWeakRefRoot->release();
}

//*****************************************************************************
int VuRefObj::addRef()
{
	mRefCount++;

	return mRefCount;
}

//*****************************************************************************
int VuRefObj::removeRef()
{
	if ( --mRefCount == 0 )
	{
		delete this; 
		return 0;
	}

	return mRefCount;
}

//*****************************************************************************
int VuRefObj::refCount()
{
	return mRefCount;
}

//*****************************************************************************
// add a weak reference
//*****************************************************************************
void VuRefObj::addWeakRef(VuRefListElement *pElement)
{
	VUASSERT(pElement->mpPrev == VUNULL, "VuRefObj::addWeakRef() invalid prev pointer");
	VUASSERT(pElement->mpNext == VUNULL, "VuRefObj::addWeakRef() invalid next pointer");

	if ( mpWeakRefRoot )
		mpWeakRefRoot->mpPrev = pElement;
	pElement->mpNext = mpWeakRefRoot;
	mpWeakRefRoot = pElement;
}

//*****************************************************************************
// remove a weak reference
//*****************************************************************************
void VuRefObj::removeWeakRef(VuRefListElement *pElement)
{
	if ( mpWeakRefRoot == pElement )
		mpWeakRefRoot = mpWeakRefRoot->mpNext;
	if ( pElement->mpPrev )
		pElement->mpPrev->mpNext = pElement->mpNext;
	if ( pElement->mpNext )
		pElement->mpNext->mpPrev = pElement->mpPrev;

	pElement->mpPrev = VUNULL;
	pElement->mpNext = VUNULL;
}


//*****************************************************************************
// VuWeakRef implementation
//*****************************************************************************

//*****************************************************************************
// release this weak reference
// (simply remove from refed object weak list)
//*****************************************************************************
template<class T>
void VuWeakRef<T>::release()
{
	if ( mpObj )
	{
		mpObj->removeWeakRef(this);
		mpObj = VUNULL;
	}
}

//*****************************************************************************
// used by assignment operators
//*****************************************************************************
template<class T>
template<class T_Other>
void VuWeakRef<T>::internalAssign(T_Other *pObj)
{
	release();
	if ( pObj )
	{
		mpObj = pObj;
		mpObj->addWeakRef(this);
	}
}
