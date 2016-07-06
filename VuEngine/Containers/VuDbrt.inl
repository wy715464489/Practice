//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Dynamic Bounding Rectangle Tree inline implementation
// 
//*****************************************************************************

#include "VuEngine/Containers/VuStack.h"


//*****************************************************************************
template <typename T>
void VuDbrt::enumNodes(const VuDbrtNode *pRoot, T &policy)
{
	policy.process(pRoot);
	if ( !pRoot->isLeaf() )
	{
		enumNodes(pRoot->mpChildren[0], policy);
		enumNodes(pRoot->mpChildren[1], policy);
	}
}

//*****************************************************************************
template <typename T>
void VuDbrt::enumLeaves(const VuDbrtNode *pRoot, T &policy)
{
	if ( !pRoot->isLeaf() )
	{
		enumLeaves(pRoot->mpChildren[0], policy);
		enumLeaves(pRoot->mpChildren[1], policy);
	}
	else
	{
		policy.process(pRoot);
	}
}

//*****************************************************************************
template <typename T>
void VuDbrt::collideBounds(const VuDbrtNode *pRoot, const VuDbrtBounds &bounds, T &policy)
{
	if ( pRoot )
	{
		VuStaticStack<const VuDbrtNode *, STACK_SIZE> nodeStack;

		nodeStack.push(pRoot);

		do
		{
			const VuDbrtNode *pNode = nodeStack.top();
			nodeStack.pop();
			if ( pNode->mBounds.intersects(bounds) )
			{
				if ( pNode->isLeaf() )
				{
					policy.process(pNode);
				}
				else
				{
					nodeStack.push(pNode->mpChildren[0]);
					nodeStack.push(pNode->mpChildren[1]);
				}
			}
		}
		while( nodeStack.size() );
	}
}

//*****************************************************************************
template <typename T>
void VuDbrt::collidePoint(const VuDbrtNode *pRoot, const VuVector2 &point, T &policy)
{
	if ( pRoot )
	{
		VuStaticStack<const VuDbrtNode *, STACK_SIZE> nodeStack;

		nodeStack.push(pRoot);

		do
		{
			const VuDbrtNode *pNode = nodeStack.top();
			nodeStack.pop();
			if ( pNode->mBounds.contains(point) )
			{
				if ( pNode->isLeaf() )
				{
					policy.process(pNode);
				}
				else
				{
					nodeStack.push(pNode->mpChildren[0]);
					nodeStack.push(pNode->mpChildren[1]);
				}
			}
		}
		while( nodeStack.size() );
	}
}

//*****************************************************************************
bool VuDbrtBounds::contains(const VuDbrtBounds &bounds) const
{
	return mMin.mX <= bounds.mMin.mX && mMin.mY <= bounds.mMin.mY &&
		   mMax.mX >= bounds.mMax.mX && mMax.mY >= bounds.mMax.mY;
}

//*****************************************************************************
bool VuDbrtBounds::contains(const VuVector2 &point) const
{
	return mMin.mX <= point.mX && mMin.mY <= point.mY &&
		   mMax.mX >= point.mX && mMax.mY >= point.mY;
}

//*****************************************************************************
bool VuDbrtBounds::intersects(const VuDbrtBounds &bounds) const
{
	return mMin.mX <= bounds.mMax.mX && mMax.mX >= bounds.mMin.mX &&
		   mMin.mY <= bounds.mMax.mY && mMax.mY >= bounds.mMin.mY;
}


