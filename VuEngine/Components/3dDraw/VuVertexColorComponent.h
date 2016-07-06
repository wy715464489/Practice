//*****************************************************************************
//
//  Copyright (c) 2012-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  VertexColorComponent class
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Components/VuComponent.h"
#include "VuEngine/Containers/VuArray.h"

class VuStaticModelInstance;
class VuGfxSceneNode;
class VuVertexBuffer;
class VuMatrix;
class VuAabb;
class VuVector3;
namespace VuLightUtil { class VuLightInfo; }


class VuVertexColorComponent : public VuComponent
{
	DECLARE_SHORT_COMPONENT_TYPE(VertexColor)
	DECLARE_RTTI

public:
	VuVertexColorComponent(VuEntity *pOwner);

	virtual void	onLoad(const VuJsonContainer &data);
	virtual void	onPostLoad();
	virtual void	onSave(VuJsonContainer &data) const;

	virtual void	onBake();
	virtual void	onClearBaked();

protected:
	typedef std::vector<std::vector<VUUINT32>> RawVertexColors;
	struct VertexColors
	{
		VertexColors() : mppVertexBuffers(VUNULL), mChunkCount(0) {}
		~VertexColors() { clear(); }
		void			clear();
		void			load(const VuJsonContainer &data);
		void			save(VuJsonContainer &data) const;
		void			setRaw(RawVertexColors &rawVertexColors);
		VuVertexBuffer	**mppVertexBuffers;
		int				mChunkCount;
	};

	void		clear();
	void		apply();

	void		recalculate(VuStaticModelInstance &modelInstance, VertexColors &vertexColors);
	void		recalculateRecursive(VuGfxSceneNode *pNode, const VuMatrix &transform, const VuLightUtil::VuLightInfo &lightInfo, RawVertexColors &vertexColors);
	bool		needsVertexColors(VuStaticModelInstance &modelInstance);

	VertexColors	mModelVertexColors;
	VertexColors	mLod1VertexColors;
	VertexColors	mLod2VertexColors;
	VertexColors	mReflectionVertexColors;
	VertexColors	mUltraVertexColors;
};
