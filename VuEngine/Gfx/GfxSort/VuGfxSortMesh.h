//*****************************************************************************
//
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  VuGfxSortMesh class
// 
//*****************************************************************************

#pragma once

class VuVertexBuffer;
class VuIndexBuffer;


class VuGfxSortMeshDesc
{
public:
	VuGfxSortMeshDesc();

	bool			operator == (const VuGfxSortMeshDesc &other) const;
	bool			operator < (const VuGfxSortMeshDesc &other) const;

	VuVertexBuffer		*mpVertexBuffer;
	VuIndexBuffer		*mpIndexBuffer;
};

class VuGfxSortMesh
{
protected:
	friend class VuGfxSort;
	VuGfxSortMesh(const VuGfxSortMeshDesc &desc);
	~VuGfxSortMesh();

	void				addRef()	{ mRefCount++; }
	void				removeRef()	{ mRefCount--; }
	VUINT32				refCount()	{ return mRefCount; }

public:
	VuGfxSortMeshDesc	mDesc;
	VUUINT32			mSortKey;
private:
	int					mRefCount;
};
