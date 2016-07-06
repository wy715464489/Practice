//*****************************************************************************
//
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Rect Packer
// 
//*****************************************************************************

#pragma once

class VuRectPackerNode;

class VuRectPacker
{
public:
	VuRectPacker(int totalWidth, int totalHeight);
	~VuRectPacker();

	bool	insert(int width, int height, int &x, int &y);

private:
	VuRectPackerNode	*mpRootNode;
};