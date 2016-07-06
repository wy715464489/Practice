//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Dynamic Bounding Volume Tree inline implementation
// 
//*****************************************************************************

#include "VuEngine/Containers/VuStack.h"
#include "VuEngine/Math/VuMathUtil.h"


//*****************************************************************************
template <typename T>
void VuDbvt::enumNodes(const VuDbvtNode *pRoot, T &policy)
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
void VuDbvt::enumLeaves(const VuDbvtNode *pRoot, T &policy)
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
template <int STACK_SIZE, typename T>
void VuDbvt::collideBounds(const VuDbvtNode *pRoot, const VuAabb &bounds, T &policy)
{
	if ( pRoot )
	{
		VuStaticStack<const VuDbvtNode *, STACK_SIZE> nodeStack;

		nodeStack.push(pRoot);

		do
		{
			const VuDbvtNode *pNode = nodeStack.top();
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
template <int STACK_SIZE, typename T>
void VuDbvt::collidePoint(const VuDbvtNode *pRoot, const VuVector3 &point, T &policy)
{
	if ( pRoot )
	{
		VuStaticStack<const VuDbvtNode *, STACK_SIZE> nodeStack;

		nodeStack.push(pRoot);

		do
		{
			const VuDbvtNode *pNode = nodeStack.top();
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
template <int STACK_SIZE, typename T>
inline void VuDbvt::collideKDOP(const VuDbvtNode *pRoot, const VuVector4 *planes, int planeCount, T &policy)
{
	if ( pRoot )
	{
		VuStaticStack<NodePtr, STACK_SIZE> nodeStack;

		const int						inside=(1<<planeCount)-1;
		int								signs[sizeof(unsigned)*8];
		VUASSERT(planeCount<int (sizeof(signs)/sizeof(signs[0])), "VuDbvt::collideKDOP() too many plan planes");
		for(int i=0;i<planeCount;++i)
		{
			signs[i]=((planes[i].mX>=0)?1:0)+((planes[i].mY>=0)?2:0)+((planes[i].mZ>=0)?4:0);
		}
		nodeStack.push(NodePtr(pRoot,0));
		do	{
			NodePtr	se=nodeStack.top();
			bool	out=false;
			nodeStack.pop();
			for(int i=0,j=1;(!out)&&(i<planeCount);++i,j<<=1)
			{
				if(0==(se.mMask&j))
				{
					const int	side=classify(se.mpNode->mBounds.mMin, se.mpNode->mBounds.mMax, planes[i], signs[i]);
					switch(side)
					{
					case	-1:	out=true;break;
					case	+1:	se.mMask|=j;break;
					}
				}
			}
			if(!out)
			{
				if((se.mMask!=inside)&&(se.mpNode->isInternal()))
				{
					nodeStack.push(NodePtr(se.mpNode->mpChildren[0],se.mMask));
					nodeStack.push(NodePtr(se.mpNode->mpChildren[1],se.mMask));
				}
				else
				{
					enumLeaves(se.mpNode, policy);
				}
			}
		} while(nodeStack.size());
	}
}

//*****************************************************************************
inline int VuDbvt::classify(const VuVector3 &mi, const VuVector3 &mx, const VuVector4 &plane, int sign)
{
	VuVector3 pi, px;
	switch ( sign )
	{
		case (0+0+0): px=VuVector3(mi.mX,mi.mY,mi.mZ); pi=VuVector3(mx.mX,mx.mY,mx.mZ); break;
		case (1+0+0): px=VuVector3(mx.mX,mi.mY,mi.mZ); pi=VuVector3(mi.mX,mx.mY,mx.mZ); break;
		case (0+2+0): px=VuVector3(mi.mX,mx.mY,mi.mZ); pi=VuVector3(mx.mX,mi.mY,mx.mZ); break;
		case (1+2+0): px=VuVector3(mx.mX,mx.mY,mi.mZ); pi=VuVector3(mi.mX,mi.mY,mx.mZ); break;
		case (0+0+4): px=VuVector3(mi.mX,mi.mY,mx.mZ); pi=VuVector3(mx.mX,mx.mY,mi.mZ); break;
		case (1+0+4): px=VuVector3(mx.mX,mi.mY,mx.mZ); pi=VuVector3(mi.mX,mx.mY,mi.mZ); break;
		case (0+2+4): px=VuVector3(mi.mX,mx.mY,mx.mZ); pi=VuVector3(mx.mX,mi.mY,mi.mZ); break;
		case (1+2+4): px=VuVector3(mx.mX,mx.mY,mx.mZ); pi=VuVector3(mi.mX,mi.mY,mi.mZ); break;
	}

	if ( VuMathUtil::distPointPlane(px,plane) <  0 ) return -1;
	if ( VuMathUtil::distPointPlane(pi,plane) >= 0 ) return  1;

	return 0;
}
