//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Dynamic Bounding Rectangle Tree
// 
//*****************************************************************************

#include "VuDbrt.h"
#include "VuEngine/Containers/VuStack.h"


// inline functions

//*****************************************************************************
inline int IndexOf(const VuDbrtNode *pNode)
{
	return pNode->mpParent->mpChildren[1] == pNode;
}

//*****************************************************************************
inline float Proximity(const VuDbrtBounds &a, const VuDbrtBounds &b)
{
	const VuVector2 d = (a.mMin + a.mMax) - (b.mMin + b.mMax);
	return VuAbs(d.mX) + VuAbs(d.mY);
}

//*****************************************************************************
inline int Select(const VuDbrtBounds &o, const VuDbrtBounds &a, const VuDbrtBounds &b)
{
	return Proximity(o,a) < Proximity(o,b) ? 0 : 1;
}

//*****************************************************************************
inline void Merge(const VuDbrtBounds &a, const VuDbrtBounds &b, VuDbrtBounds &r)
{
	r.mMin = VuMin(a.mMin, b.mMin);
	r.mMax = VuMax(a.mMax, b.mMax);
}

//*****************************************************************************
inline bool NotEqual(const VuDbrtBounds &a, const VuDbrtBounds &b)
{
	return a.mMin != b.mMin || a.mMax != b.mMax;
}


//*****************************************************************************
VuDbrt::VuDbrt(VUUINT initialNodeCount, VUUINT allocNodeCount):
	mpRoot(VUNULL),
	mpFree(VUNULL),
	mAllocNodeCount(allocNodeCount),
	mLeafCount(0),
	mNodeCount(0),
	mFreeCount(0)
{
	allocNodes(initialNodeCount);
}

//*****************************************************************************
VuDbrt::~VuDbrt()
{
	clear();

	for ( Allocations::iterator iter = mAllocations.begin(); iter != mAllocations.end(); iter++ )
		delete[] *iter;
}

//*****************************************************************************
void VuDbrt::clear()
{
	if ( mpRoot)
		deleteNodeRecursive(mpRoot);
}

//*****************************************************************************
VuDbrtNode *VuDbrt::insert(void *pData, const VuDbrtBounds &bounds)
{
	VuDbrtNode *pLeaf = createNode(VUNULL, bounds, pData);
	insertLeaf(mpRoot, pLeaf);
	mLeafCount++;
	return pLeaf;
}

//*****************************************************************************
void VuDbrt::remove(VuDbrtNode *pLeaf)
{
	removeLeaf(pLeaf);
	deleteNode(pLeaf);
	mLeafCount--;
}

//*****************************************************************************
void VuDbrt::update(VuDbrtNode *pLeaf, const VuDbrtBounds &bounds)
{
	removeLeaf(pLeaf);
	pLeaf->mBounds = bounds;
	insertLeaf(mpRoot, pLeaf);
}

//*****************************************************************************
void VuDbrt::allocNodes(VUUINT count)
{
	VuDbrtNode *pNodes = new VuDbrtNode[count];
	memset(pNodes, 0, count*sizeof(pNodes[0]));
	mAllocations.push_back(pNodes);

	for ( VUUINT i = 0; i < count; i++ )
		pNodes[i].mpNextFree = &pNodes[i+1];

	pNodes[count-1].mpNextFree = mpFree;
	mpFree = pNodes;

	mFreeCount += count;
}

//*****************************************************************************
VuDbrtNode *VuDbrt::createNode(VuDbrtNode *pParent, void *pData)
{
	if ( mpFree == VUNULL )
		allocNodes(mAllocNodeCount);

	VuDbrtNode *pNode = mpFree;
	mpFree = pNode->mpNextFree;

	pNode->mpParent = pParent;
	pNode->mpData = pData;
	pNode->mpChildren[1] = VUNULL;
	pNode->mpExtraData = VUNULL;

	mNodeCount++;
	mFreeCount--;

	return pNode;
}

//*****************************************************************************
VuDbrtNode *VuDbrt::createNode(VuDbrtNode *pParent, const VuDbrtBounds &bounds, void *pData)
{
	VuDbrtNode *pNode = createNode(pParent, pData);
	pNode->mBounds = bounds;
	return pNode;
}

//*****************************************************************************
VuDbrtNode *VuDbrt::createNode(VuDbrtNode *pParent, const VuDbrtBounds &bounds0, const VuDbrtBounds &bounds1, void *pData)
{
	VuDbrtNode *pNode = createNode(pParent, pData);
	Merge(bounds0, bounds1, pNode->mBounds);
	return pNode;
}

//*****************************************************************************
void VuDbrt::deleteNode(VuDbrtNode *pNode)
{
	pNode->mpNextFree = mpFree;
	mpFree = pNode;

	mNodeCount--;
	mFreeCount++;
}

//*****************************************************************************
void VuDbrt::deleteNodeRecursive(VuDbrtNode *pNode)
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
void VuDbrt::insertLeaf(VuDbrtNode *pRoot, VuDbrtNode *pLeaf)
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

		VuDbrtNode *pPrev = pRoot->mpParent;
		VuDbrtNode *pNode = createNode(pPrev, pLeaf->mBounds, pRoot->mBounds, VUNULL);
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
VuDbrtNode *VuDbrt::removeLeaf(VuDbrtNode *pLeaf)
{
	if ( pLeaf == mpRoot )
	{
		mpRoot = VUNULL;
		return VUNULL;
	}
	else
	{
		VuDbrtNode *pParent = pLeaf->mpParent;
		VuDbrtNode *pPrev = pParent->mpParent;
		VuDbrtNode *pSibling = pParent->mpChildren[1-IndexOf(pLeaf)];
		if ( pPrev)
		{
			pPrev->mpChildren[IndexOf(pParent)] = pSibling;
			pSibling->mpParent = pPrev;
			deleteNode(pParent);
			while ( pPrev )
			{
				const VuDbrtBounds bounds = pPrev->mBounds;
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
