//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Static Model Instance
// 
//*****************************************************************************

#pragma once

#include "VuModelInstance.h"

class VuStaticModelAsset;
class VuGfxStaticScene;
class VuGfxSceneNode;
class VuVertexBuffer;

class VuStaticModelInstance : public VuModelInstance
{
public:
	VuStaticModelInstance();
	~VuStaticModelInstance();

	void				reset();

	// notifications for derived classes
	virtual void		onSetModel() {};
	virtual void		onReset() {};

	// the container can be set either by specifying the model asset or directly
	void				setModelAsset(const std::string &assetName);
	void				setModel(VuGfxStaticScene *pGfxStaticScene);
	void				setRejectionScaleModifier(float modifier)	{ mRejectionScaleModifier = modifier; }

	VuStaticModelAsset	*getModelAsset() const		{ return mpModelAsset; }
	VuGfxStaticScene	*getGfxStaticScene() const	{ return mpGfxStaticScene; }
	const VuAabb		&getAabb() const;

	// draw
	void				draw(const VuMatrix &modelMat, const VuGfxDrawParams &params) const;
	void				drawShadow(const VuMatrix &modelMat, const VuGfxDrawShadowParams &params) const;
	void				drawDropShadow(const VuMatrix &modelMat, const VuGfxDrawShadowParams &params) const;
	void				drawPrefetch() const;
	void				drawInfo(const VuMatrix &modelMat, const VuGfxDrawInfoParams &params) const;

	// collide (used by tools)
	bool				collideRay(const VuMatrix &modelMat, const VuVector3 &v0, VuVector3 &v1, bool shadowing = false) const;
	bool				collideSphere(const VuMatrix &modelMat, const VuVector3 &pos, float radius) const;

	// vertex colors
	bool				setVertexColors(VuVertexBuffer **ppBuffers, int count);
	void				clearVertexColors() { mppVertexBuffers = VUNULL; mVertexBufferCount = 0; }

protected:
	void				drawRecursive(VuGfxSceneNode *pNode, const VuMatrix &transform, const VuGfxDrawParams &params) const;
	void				drawShadowRecursive(VuGfxSceneNode *pNode, const VuMatrix &transform, const VuGfxDrawShadowParams &params) const;
	void				drawDropShadowRecursive(VuGfxSceneNode *pNode, const VuMatrix &transform, const VuGfxDrawShadowParams &params) const;
	void				drawInfoRecursive(VuGfxSceneNode *pNode, const VuMatrix &transform, const VuGfxDrawInfoParams &params) const;

	template <bool SHADOWING>
	bool				collideRayRecursive(VuGfxSceneNode *pNode, const VuMatrix &transform, const VuVector3 &v0, VuVector3 &v1) const;
	template <bool SHADOWING>
	bool				collideRayMesh(VuGfxSceneMesh *pMesh, const VuVector3 &v0, VuVector3 &v1) const;
	bool				testAabbRayCollision(const VuAabb &aabb, const VuMatrix &transform, const VuVector3 &v0, const VuVector3 &v1) const;
	bool				collideSphereRecursive(VuGfxSceneNode *pNode, const VuMatrix &transform, const VuVector3 &pos, float radius) const;
	bool				testAabbSphereCollision(const VuAabb &aabb, const VuMatrix &transform, const VuVector3 &pos, float radius) const;
	bool				collideSphereMesh(VuGfxSceneMesh *pMesh, const VuMatrix &transform, const VuVector3 &pos, float radius) const;

	VuStaticModelAsset	*mpModelAsset;
	VuGfxStaticScene	*mpGfxStaticScene;
	float				mRejectionScaleModifier;

	VuVertexBuffer		**mppVertexBuffers;
	int					mVertexBufferCount;
};
