//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  GfxComposer class
// 
//*****************************************************************************

#pragma once

#include "VuEngine/VuSystemComponent.h"
#include "VuEngine/Math/VuVector2.h"

class VuEngine;
class VuTexture;
class VuShaderProgram;
class VuVertexDeclaration;
class VuGfxSortMaterial;
class VuGfxSortMaterialDesc;
class VuGfxSortMatExt;
class VuGfxDrawParams;


class VuGfxComposer : public VuSystemComponent
{
	DECLARE_SYSTEM_COMPONENT(VuGfxComposer)

protected:
	// called by engine
	friend class VuEngine;
	virtual bool	init() { return true; }

public:
	VuGfxComposer() : mpGameInterface(VUNULL) {}

	class GameInterface
	{
	public:
		virtual VuTexture		*getWaterReflectionTexture(int viewport)	{ return VUNULL; }
		virtual VuTexture		*getDepthTexture()							{ return VUNULL; }
		virtual VuVector2		getWaterReflectionMapOffset()				{ return VuVector2(0.5f, 0.5f); }
		virtual VuVector2		getWaterReflectionMapScale()				{ return VuVector2(0.5f, 0.5f); }
		virtual VuGfxSortMatExt	*createMaterialExt(VuGfxSortMaterial *pMat) { return VUNULL; }
		virtual void			setGlobalConstants(const VuGfxSortMatExt *pGfxSortMatExt, VuShaderProgram *pSP) {}
		virtual void			gatherInfo(const VuGfxDrawParams &params) {}
		virtual void			synchronizeGfx() {}
	};
	void		setGameInterface(GameInterface *pGIF)	{ mpGameInterface = pGIF; }

	// textures
	VuTexture			*getWaterReflectionTexture(int viewport)	{ return mpGameInterface->getWaterReflectionTexture(viewport); }
	VuTexture			*getDepthTexture()							{ return mpGameInterface->getDepthTexture(); }
	VuVector2			getWaterReflectionMapOffset()				{ return mpGameInterface->getWaterReflectionMapOffset(); }
	VuVector2			getWaterReflectionMapScale()				{ return mpGameInterface->getWaterReflectionMapScale(); }

	VuGfxSortMatExt		*createMaterialExt(VuGfxSortMaterial *pMat)	{ return mpGameInterface ? mpGameInterface->createMaterialExt(pMat) : VUNULL; }
	void				setGlobalConstants(const VuGfxSortMatExt *pGfxSortMatExt, VuShaderProgram *pSP) { mpGameInterface->setGlobalConstants(pGfxSortMatExt, pSP); }
	void				gatherInfo(const VuGfxDrawParams &params) { mpGameInterface->gatherInfo(params); }
	void				synchronizeGfx() { mpGameInterface->synchronizeGfx(); }

private:
	GameInterface		*mpGameInterface;
};
