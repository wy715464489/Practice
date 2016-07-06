//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Engine-defined collision types
// 
//*****************************************************************************

#pragma once


enum VuEngineCollisionTypes
{
	COL_NOTHING                 =    0,
	COL_EVERYTHING              =   -1,

	COL_ENGINE_STATIC_PROP      = 1<<0,
	COL_ENGINE_DYNAMIC_PROP     = 1<<1,
	COL_ENGINE_CORONA_OCCLUDER  = 1<<2,
	COL_ENGINE_RAGDOLL          = 1<<3,

	COL_GAME_BEGIN              = 1<<4,
};

enum VuEngineExtendedCollisionFlags
{
	EXT_COL_ENGINE_BREAKABLE           = 1<<0,
	EXT_COL_ENGINE_EXPLODABLE          = 1<<1,
	EXT_COL_ENGINE_NOT_CORONA          = 1<<2,
	EXT_COL_ENGINE_DETECT_EXPLOSIONS   = 1<<3,
	EXT_COL_ENGINE_REACT_TO_EXPLOSIONS = 1<<4,

	EXT_COL_GAME_BEGIN                 = 1<<5,
};

enum VuWaterSurfaceFlags
{
	WATER_SURFACE_WATER = 1<<0,

	WATER_GAME_BEGIN    = 1<<1,
};
