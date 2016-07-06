//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Dynamic Bounding Volume Tree
// 
//*****************************************************************************

#include "VuDbvt.h"


// inline functions

//*****************************************************************************
inline int IndexOf(const VuDbvtNode *pNode)
{
	return pNode->mpParent->mpChildren[1] == pNode;
}

//*****************************************************************************
inline float Proximity(const VuAabb &a, const VuAabb &b)
{
	const VuVector3 d = (a.mMin + a.mMax) - (b.mMin + b.mMax);
	return VuAbs(d.mX) + VuAbs(d.mY) + VuAbs(d.mZ);
}

//*****************************************************************************
inline int Select(const VuAabb &o, const VuAabb &a, const VuAabb &b)
{
	return Proximity(o,a) < Proximity(o,b) ? 0 : 1;
}

//*****************************************************************************
inline void Merge(const VuAabb &a, const VuAabb &b, VuAabb &r)
{
	r.mMin = VuMin(a.mMin, b.mMin);
	r.mMax = VuMax(a.mMax, b.mMax);
}

//*****************************************************************************
inline bool NotEqual(const VuAabb &a, const VuAabb &b)
{
	return a.mMin != b.mMin || a.mMax != b.mMax;
}


//*****************************************************************************
VuDbvt::VuDbvt():
	mpRoot(VUNULL),
	mpFree(VUNULL),
	mLeafCount(0)
{
}

//*****************************************************************************
VuDbvt::~VuDbvt()
{
	clear();
}

//*****************************************************************************
void VuDbvt::clear()
{
	if ( mpRoot)
	{
		deleteNodeRecursive(mpRoot);
		mpRoot = VUNULL;
	}
	if ( mpFree )
	{
		VU_ALIGNED_FREE(mpFree);
		mpFree = VUNULL;
	}
}

//*****************************************************************************
VuDbvtNode *VuDbvt::insert(void *pData, const VuAabb &bounds)
{
	VuDbvtNode *pLeaf = createNode(VUNULL, bounds, pData);
	insertLeaf(mpRoot, pLeaf);
	mLeafCount++;
	return pLeaf;
}

//*****************************************************************************
void VuDbvt::remove(VuDbvtNode *pLeaf)
{
	removeLeaf(pLeaf);
	deleteNode(pLeaf);
	mLeafCount--;
}

//*****************************************************************************
void VuDbvt::update(VuDbvtNode *pLeaf, const VuAabb &bounds)
{
	removeLeaf(pLeaf);
	pLeaf->mBounds = bounds;
	insertLeaf(mpRoot, pLeaf);
}

//*****************************************************************************
VuDbvtNode *VuDbvt::createNode(VuDbvtNode *pParent, void *pData)
{
	VuDbvtNode *pNode;

	if ( mpFree )
	{
		pNode = mpFree;
		mpFree = 0;
	}
	else
	{
		pNode = new (VU_ALIGNED_ALLOC(sizeof(VuDbvtNode),16)) VuDbvtNode();
	}
	pNode->mpParent = pParent;
	pNode->mpData = pData;
	pNode->mpChildren[1] = 0;

	return pNode;
}

//*****************************************************************************
VuDbvtNode *VuDbvt::createNode(VuDbvtNode *pParent, const VuAabb &bounds, void *pData)
{
	VuDbvtNode *pNode = createNode(pParent, pData);
	pNode->mBounds = bounds;

	return pNode;
}

//*****************************************************************************
VuDbvtNode *VuDbvt::createNode(VuDbvtNode *pParent, const VuAabb &bounds0, const VuAabb &bounds1, void *pData)
{
	VuDbvtNode *pNode = createNode(pParent, pData);
	Merge(bounds0, bounds1, pNode->mBounds);

	return pNode;
}

//*****************************************************************************
void VuDbvt::deleteNode(VuDbvtNode *pNode)
{
	if ( mpFree )
	{
		VU_ALIGNED_FREE(mpFree);
	}
	mpFree = pNode;
}

//*****************************************************************************
void VuDbvt::deleteNodeRecursive(VuDbvtNode *pNode)
{
	if ( !pNode->isLeaf() )
	{
		deleteNodeRecursive(pNode->mpChildren[0]);
		deleteNodeRecursive(pNode->mpChildren[1]);
	}

	if ( pNode == mpRoot )
		mpRoot = VUNULL;

	deleteNode(pNode);
}

//*****************************************************************************
void VuDbvt::insertLeaf(VuDbvtNode *pRoot, VuDbvtNode *pLeaf)
{
	if ( mpRoot == VUNULL )
	{
		mpRoot = pLeaf;
		pLeaf->mpParent = VUNULL;
	}
	else
	{
		while ( !pRoot->isLeaf() )
			pRoot = pRoot->mpChildren[Select(pLeaf->mBounds, pRoot->mpChildren[0]->mBounds, pRoot->mpChildren[1]->mBounds)];

		VuDbvtNode *pPrev = pRoot->mpParent;
		VuDbvtNode *pNode = createNode(pPrev, pLeaf->mBounds, pRoot->mBounds, VUNULL);
		if ( pPrev )
		{
			pPrev->mpChildren[IndexOf(pRoot)] = pNode;
			pNode->mpChildren[0] = pRoot; pRoot->mpParent = pNode;
			pNode->mpChildren[1] = pLeaf; pLeaf->mpParent = pNode;
			do
			{
				if ( !pPrev->mBounds.contains(pNode->mBounds) )
					Merge(pPrev->mpChildren[0]->mBounds, pPrev->mpChildren[1]->mBounds, pPrev->mBounds);
				else
					break;
				pNode = pPrev;
			}
			while ( (pPrev = pNode->mpParent) != VUNULL );
		}
		else
		{
			pNode->mpChildren[0] = pRoot; pRoot->mpParent = pNode;
			pNode->mpChildren[1] = pLeaf; pLeaf->mpParent = pNode;
			mpRoot = pNode;
		}
	}
}

//*****************************************************************************
VuDbvtNode *VuDbvt::removeLeaf(VuDbvtNode *pLeaf)
{
	if ( pLeaf == mpRoot )
	{
		mpRoot = VUNULL;
		return VUNULL;
	}
	else
	{
		VuDbvtNode *pParent = pLeaf->mpParent;
		VuDbvtNode *pPrev = pParent->mpParent;
		VuDbvtNode *pSibling = pParent->mpChildren[1-IndexOf(pLeaf)];
		if ( pPrev)
		{
			pPrev->mpChildren[IndexOf(pParent)] = pSibling;
			pSibling->mpParent = pPrev;
			deleteNode(pParent);
			while ( pPrev )
			{
				const VuAabb bounds = pPrev->mBounds;
				Merge(pPrev->mpChildren[0]->mBounds, pPrev->mpChildren[1]->mBounds, pPrev->mBounds);
				if ( NotEqual(bounds, pPrev->mBounds) )
					pPrev = pPrev->mpParent;
				else
					break;
			}
			return pPrev ? pPrev : mpRoot;
		}
		else
		{
			mpRoot = pSibling;
			pSibling->mpParent = VUNULL;
			deleteNode(pParent);
			return mpRoot;
		}
	}
}
