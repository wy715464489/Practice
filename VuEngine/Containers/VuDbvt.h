//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Dynamic Bounding Volume Tree
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Math/VuAabb.h"

class VuDbvtNode;
class VuVector4;


class VuDbvt
{
public:
	VuDbvt();
	~VuDbvt();

	void				clear();

	VuDbvtNode*			insert(void *pData, const VuAabb &bounds);
	void				remove(VuDbvtNode *pLeaf);
	void				update(VuDbvtNode *pLeaf, const VuAabb &bounds);

	template <typename T>
	static void			enumNodes(const VuDbvtNode *pRoot, T &policy);
	template <typename T>
	static void			enumLeaves(const VuDbvtNode *pRoot, T &policy);
	template <int STACK_SIZE, typename T>
	static void			collideBounds(const VuDbvtNode *pRoot, const VuAabb &bounds, T &policy);
	template <int STACK_SIZE, typename T>
	static void			collidePoint(const VuDbvtNode *pRoot, const VuVector3 &point, T &policy);
	template <int STACK_SIZE, typename T>
	static void			collideKDOP(const VuDbvtNode *pRoot, const VuVector4 *planes, int planeCount, T &policy);

	const VuDbvtNode	*getRoot() const { return mpRoot; }

	VUUINT				getLeafCount() const { return mLeafCount; }

private:
	VuDbvtNode			*createNode(VuDbvtNode *pParent, void *pData);
	VuDbvtNode			*createNode(VuDbvtNode *pParent, const VuAabb &bounds, void *pData);
	VuDbvtNode			*createNode(VuDbvtNode *pParent, const VuAabb &bounds0, const VuAabb &bounds1, void *pData);
	void				deleteNode(VuDbvtNode *pNode);
	void				deleteNodeRecursive(VuDbvtNode *pNode);

	void				insertLeaf(VuDbvtNode *pRoot, VuDbvtNode *pLeaf);
	VuDbvtNode			*removeLeaf(VuDbvtNode *pLeaf);

	inline static int	classify(const VuVector3 &mi, const VuVector3 &mx, const VuVector4 &plane, int sign);

	VuDbvtNode			*mpRoot;
	VuDbvtNode			*mpFree;
	VUUINT				mLeafCount;

	struct NodePtr
	{
		NodePtr() {}
		NodePtr(const VuDbvtNode *pNode, int mask) : mpNode(pNode), mMask(mask) {}
		const VuDbvtNode	*mpNode;
		int					mMask;
	};
};

class VuDbvtNode
{
public:
	VuAabb			mBounds;
	VuDbvtNode		*mpParent;
	union
	{
		VuDbvtNode	*mpChildren[2];
		void		*mpData;
		VuDbvtNode	*mpNextFree;
	};

	inline bool	isLeaf() const	{ return mpChildren[1] == VUNULL; }
	inline bool isInternal() const { return(!isLeaf()); }
};


#include "VuDbvt.inl"
