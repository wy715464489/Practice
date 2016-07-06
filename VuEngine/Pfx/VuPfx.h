//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Low-level Pfx class
// 
//*****************************************************************************

#pragma once

#include "VuEngine/VuSystemComponent.h"
#include "VuPfxSystem.h"

class VuEngine;
class VuJsonContainer;
class VuFastContainer;
class VuPfxConfig;
class VuPfxSystemInstance;
class VuPfxRegistry;
class VuPfxResources;
class VuPfxQuadShader;
class VuPfxTrailShader;
class VuPfxNode;
class VuPfxGroup;
class VuPfxSystem;
class VuPfxPattern;
class VuPfxProcess;
class VuProperties;


class VuPfx : public VuSystemComponent
{
	DECLARE_SYSTEM_COMPONENT(VuPfx)

public:
	VuPfx();
	~VuPfx();

	virtual bool	init();
	virtual void	release();

	// public interface

	// configuration
	void			configure(const VuPfxConfig &config);

	// project management
	bool			addProject(const char *strName, const VuFastContainer &data);
	void			removeProject(const char *strName);

	// system instance management
	VuPfxSystemInstance	*createSystemInstance(const char *strPath) const;
	VuPfxSystemInstance	*createSystemInstance(const char *strProject, const char *strPath) const;
	void				releaseSystemInstance(VuPfxSystemInstance *pSystemInstance) const;

	// debug
	void			checkForLeaks() const;


	// internal interface

	// parameter access
	VuPfxGroup			*getProject(const char *strName) const;
	VuPfxNode			*getNode(const char *strProject, const char *strPath) const;
	VuPfxGroup			*getGroup(const char *strProject, const char *strPath) const;
	VuPfxSystem			*getSystem(const char *strProject, const char *strPath) const;
	VuPfxPattern		*getPattern(const char *strProject, const char *strPath) const;
	VuPfxProcess		*getProcess(const char *strProject, const char *strPath) const;

	// properties access
	void				getNamespace(VuJsonContainer &data) const;
	VuProperties		*getProperties(const char *strPath) const;

	// registry access
	VuPfxRegistry		*registry()		{ return mpRegistry; }
	VuPfxResources		*resources()	{ return mpResources; }

	// shader access
	VuPfxQuadShader		*quadShader()	{ return mpQuadShader; }
	VuPfxTrailShader	*trailShader()	{ return mpTrailShader; }

	// dev
	bool				isDrawEnabled()	{ return mbDraw; }

private:
	typedef std::map<std::string, VuPfxGroup *> Projects;

	VuPfxNode			*getNode(VuPfxNode *pNode, const char *strPath) const;

	VuPfxRegistry		*mpRegistry;
	VuPfxResources		*mpResources;
	VuPfxQuadShader		*mpQuadShader;
	VuPfxTrailShader	*mpTrailShader;
	Projects			mProjects;
	bool				mbDraw;
	bool				mbDrawDebug;

	void				getNamespaceRecursive(const VuPfxNode *pNode, VuJsonContainer &data) const;
	void				tick(float fdt);
	void				updateDevStats();
	void				drawDebug();
};


class VuPfxConfig
{
public:
	VuPfxConfig();

	int	mMaxSystemCount;
	int	mMaxSystemSize;
	int	mMaxPatternCount;
	int	mMaxPatternSize;
	int	mMaxProcessCount;
	int	mMaxProcessSize;
	int mMaxParticleCount;
	int mMaxParticleSize;
};
