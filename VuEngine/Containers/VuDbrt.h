//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Dynamic Bounding Rectangle Tree
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Math/VuAabb.h"
#include "VuEngine/Math/VuVector2.h"

class VuDbrtNode;
class VuDbrtBounds;


class VuDbrt
{
public:
	VuDbrt(VUUINT initialNodeCount, VUUINT allocNodeCount);
	~VuDbrt();

	void				clear();

	VuDbrtNode			*insert(void *pData, const VuDbrtBounds &bounds);
	void				remove(VuDbrtNode *pLeaf);
	void				update(VuDbrtNode *pLeaf, const VuDbrtBounds &bounds);

	template <typename T>
	static void			enumNodes(const VuDbrtNode *pRoot, T &policy);
	template <typename T>
	static void			enumLeaves(const VuDbrtNode *pRoot, T &policy);
	template <typename T>
	static void			collideBounds(const VuDbrtNode *pRoot, const VuDbrtBounds &bounds, T &policy);
	template <typename T>
	static void			collidePoint(const VuDbrtNode *pRoot, const VuVector2 &point, T &policy);

	const VuDbrtNode	*getRoot() const { return mpRoot; }

	VUUINT				getLeafCount() const { return mLeafCount; }
	VUUINT				getNodeCount() const { return mNodeCount; }
	VUUINT				getFreeCount() const { return mFreeCount; }

private:
	enum { STACK_SIZE = 256 };
	typedef std::list<VuDbrtNode *> Allocations;

	void				allocNodes(VUUINT count);

	VuDbrtNode			*createNode(VuDbrtNode *pParent, void *pData);
	VuDbrtNode			*createNode(VuDbrtNode *pParent, const VuDbrtBounds &bounds, void *pData);
	VuDbrtNode			*createNode(VuDbrtNode *pParent, const VuDbrtBounds &bounds0, const VuDbrtBounds &bounds1, void *pData);
	void				deleteNode(VuDbrtNode *pNode);
	void				deleteNodeRecursive(VuDbrtNode *pNode);

	void				insertLeaf(VuDbrtNode *pRoot, VuDbrtNode *pLeaf);
	VuDbrtNode			*removeLeaf(VuDbrtNode *pLeaf);

	VuDbrtNode			*mpRoot;
	VuDbrtNode			*mpFree;
	VUUINT				mAllocNodeCount;
	Allocations			mAllocations;
	VUUINT				mLeafCount;
	VUUINT				mNodeCount;
	VUUINT				mFreeCount;
};


class VuDbrtBounds
{
public:
	VuDbrtBounds()	{}
	VuDbrtBounds(const VuVector2 &vMin, const VuVector2 &vMax) : mMin(vMin), mMax(vMax)	{}
	VuDbrtBounds(const VuAabb &aabb) : mMin(aabb.mMin.mX, aabb.mMin.mY), mMax(aabb.mMax.mX, aabb.mMax.mY)	{}

	inline bool	contains(const VuDbrtBounds &bounds) const;
	inline bool	contains(const VuVector2 &point) const;
	inline bool	intersects(const VuDbrtBounds &bounds) const;

	VuVector2	mMin;
	VuVector2	mMax;
};

class VuDbrtNode
{
public:
	VuDbrtBounds	mBounds;
	VuDbrtNode		*mpParent;
	union
	{
		VuDbrtNode	*mpChildren[2];
		void		*mpData;
		VuDbrtNode	*mpNextFree;
	};
	void			*mpExtraData;

	inline bool	isLeaf() const	{ return mpChildren[1] == VUNULL; }
};


#include "VuDbrt.inl"
