//*****************************************************************************
//
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Rect Packer
// 
//*****************************************************************************

#include "VuRectPacker.h"


class VuRectPackerNode
{
public:
	VuRectPackerNode(int x, int y, int width, int height);
	~VuRectPackerNode();

	VuRectPackerNode	*insert(int width, int height);

	int					mX;
	int					mY;
	int					mWidth;
	int					mHeight;
	bool				mOccupied;
	VuRectPackerNode	*mpChild0;
	VuRectPackerNode	*mpChild1;
};


//*****************************************************************************
VuRectPacker::VuRectPacker(int totalWidth, int totalHeight)
{
	mpRootNode = new VuRectPackerNode(0, 0, totalWidth, totalHeight);
}

//*****************************************************************************
VuRectPacker::~VuRectPacker()
{
	delete mpRootNode;
}

//*****************************************************************************
bool VuRectPacker::insert(int width, int height, int &x, int &y)
{
	VuRectPackerNode *pNode = mpRootNode->insert(width, height);
	if ( pNode == VUNULL )
		return false;

	x = pNode->mX;
	y = pNode->mY;

	return true;
}

//*****************************************************************************
VuRectPackerNode::VuRectPackerNode(int x, int y, int width, int height):
	mX(x),
	mY(y),
	mWidth(width),
	mHeight(height),
	mOccupied(false),
	mpChild0(VUNULL),
	mpChild1(VUNULL)
{
}

//*****************************************************************************
VuRectPackerNode::~VuRectPackerNode()
{
	delete mpChild0;
	delete mpChild1;
}

//*****************************************************************************
VuRectPackerNode *VuRectPackerNode::insert(int width, int height)
{
	// if we're not a leaf then
	if ( mpChild0 )
	{
		// try inserting into first child
		if ( VuRectPackerNode *pNewNode = mpChild0->insert(width, height) )
			return pNewNode;

		// no room, insert into second
		return mpChild1->insert(width, height);
	}
	else
	{
		// if we're already occupied, return
		if ( mOccupied )
			return VUNULL;

		// if we're too small, return
		if ( mWidth < width || mHeight < height )
			return VUNULL;

		// if we're just right, accept
		if ( mWidth == width && mHeight == height )
		{
			mOccupied = true;
			return this;
		}

		// otherwise, decide which way to split
		int dw = mWidth - width;
		int dh = mHeight - height;

		if ( dw > dh )
		{
			//  horizontal split
			mpChild0 = new VuRectPackerNode(mX, mY, width, mHeight);
			mpChild1 = new VuRectPackerNode(mX + width, mY, mWidth - width, mHeight);
		}
		else
		{
			// vertical split
			mpChild0 = new VuRectPackerNode(mX, mY, mWidth, height);
			mpChild1 = new VuRectPackerNode(mX, mY + height, mWidth, mHeight - height);
		}

		// insert into first child we created
		return mpChild0->insert(width, height);
	}
}
