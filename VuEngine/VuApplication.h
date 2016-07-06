//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Interface class to the game.
// 
//*****************************************************************************

#pragma once

class VuApplication
{
public:
	//*****************************************************************************
	// P U B L I C   M E T H O D S
	//*****************************************************************************

	// static methods used by the editors/game
	static bool			initGame();
	static bool			initEditor();
	static bool			initLauncher();
	static void			release();
	static const char	*getName();
	static VUUINT32		getTitleID();
};
