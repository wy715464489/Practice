//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Dev Menu
// 
//*****************************************************************************

#include "VuDevMenu.h"
#include "VuDev.h"
#include "VuEngine/Math/VuRect.h"
#include "VuEngine/HAL/Gfx/VuGfx.h"
#include "VuEngine/HAL/Keyboard/VuKeyboard.h"
#include "VuEngine/Gfx/VuGfxUtil.h"
#include "VuEngine/Gfx/Font/VuFontDraw.h"
#include "VuEngine/Gfx/GfxSort/VuGfxSort.h"
#include "VuEngine/Util/VuFileUtil.h"
#include "VuEngine/Managers/VuDrawManager.h"


#if VU_DISABLE_DEV_MENU

IMPLEMENT_SYSTEM_COMPONENT(VuDevMenu, VuDevMenu);

#else

class VuDevMenuNode
{
public:
	VuDevMenuNode(const std::string &strName) : mstrName(strName), mDepth(-1), mpParent(VUNULL), mpPrevSibling(VUNULL), mpNextSibling(VUNULL) {}
	virtual ~VuDevMenuNode() {}

	virtual void			print(char *buf, int size) = 0;
	virtual void			modify(int step) = 0;

	virtual VuDevMenuNode	*nextVisible();

	virtual void			insert(std::string path, VuDevMenuNode *pNode) { VUASSERT(0, "VuDevMenuNode::insert() attempt to insert into a non-folder."); }
	virtual void			remove(std::string path, const std::string &name) { VUASSERT(0, "VuDevMenuNode::insert() attempt to insert into a non-folder."); }

	static std::string		getRoot(const std::string &path);
	static std::string		subtractRoot(const std::string &path);

	std::string				mstrName;
	int						mDepth;
	VuDevMenuNode			*mpParent;
	VuDevMenuNode			*mpPrevSibling;
	VuDevMenuNode			*mpNextSibling;
};

class VuDevMenuFolderNode : public VuDevMenuNode
{
public:
	VuDevMenuFolderNode(const std::string &strName) : VuDevMenuNode(strName), mExpanded(false), mpFirstChild(VUNULL), mpLastChild(VUNULL) {}
	~VuDevMenuFolderNode();

	virtual void			print(char *buf, int size)	{ VU_SNPRINTF(buf, size, size - 1, "%s%s", mExpanded ? "-" : "+", mstrName.c_str()); }
	virtual void			modify(int step)			{ mExpanded = step > 0; }

	virtual VuDevMenuNode	*nextVisible();

	virtual void			insert(std::string path, VuDevMenuNode *pNode);
	virtual void			remove(std::string path, const std::string &name);

private:
	VuDevMenuNode	*findChild(const std::string &strName);

	bool			mExpanded;
	VuDevMenuNode	*mpFirstChild;
	VuDevMenuNode	*mpLastChild;
};

class VuDevMenuBoolNode : public VuDevMenuNode
{
public:
	VuDevMenuBoolNode(const std::string &strName, bool &value) : VuDevMenuNode(strName), mValue(value) {}

	virtual void	print(char *buf, int size)	{ VU_SNPRINTF(buf, size, size - 1, "%s = %s", mstrName.c_str(), mValue ? "true" : "false"); }
	virtual void	modify(int step)			{ mValue = step > 0; }

private:
	bool			&mValue;
};

class VuDevMenuIntNode : public VuDevMenuNode
{
public:
	VuDevMenuIntNode(const std::string &strName, int &value, int valStep, int valMin, int valMax) : VuDevMenuNode(strName), mValue(value), mStep(valStep), mMin(valMin), mMax(valMax) {}

	virtual void	print(char *buf, int size)	{ VU_SNPRINTF(buf, size, size - 1, "%s = %d", mstrName.c_str(), mValue); }
	virtual void	modify(int step)			{ mValue = VuClamp(mValue + step*mStep, mMin, mMax); }

private:
	int				&mValue;
	int				mStep;
	int				mMin;
	int				mMax;
};

class VuDevMenuFloatNode : public VuDevMenuNode
{
public:
	VuDevMenuFloatNode(const std::string &strName, float &value, float valStep, float valMin, float valMax) : VuDevMenuNode(strName), mValue(value), mStep(valStep), mMin(valMin), mMax(valMax) {}

	virtual void	print(char *buf, int size)	{ VU_SNPRINTF(buf, size, size - 1, "%s = %.7g", mstrName.c_str(), mValue); }
	virtual void	modify(int step)	{ mValue = VuClamp(mValue + step*mStep, mMin, mMax); }

private:
	float			&mValue;
	float			mStep;
	float			mMin;
	float			mMax;
};

class VuDevMenuUInt8Node : public VuDevMenuNode
{
public:
	VuDevMenuUInt8Node(const std::string &strName, VUUINT8 &value, int valStep, int valMin, int valMax) : VuDevMenuNode(strName), mValue(value), mStep(valStep), mMin(valMin), mMax(valMax) {}

	virtual void	print(char *buf, int size)	{ VU_SNPRINTF(buf, size, size - 1, "%s = %d", mstrName.c_str(), (int)mValue); }
	virtual void	modify(int step)			{ mValue = (VUUINT8)VuClamp(mValue + step*mStep, mMin, mMax); }

private:
	VUUINT8			&mValue;
	int				mStep;
	int				mMin;
	int				mMax;
};

class VuDevMenuIntEnumNode : public VuDevMenuNode
{
public:
	VuDevMenuIntEnumNode(const std::string &strName, int &value, const VuDevMenu::IntEnumChoice *choices) : VuDevMenuNode(strName), mValue(value), mpChoices(choices) {}

	virtual void	print(char *buf, int size)	{ VU_SNPRINTF(buf, size, size - 1, "%s = %s", mstrName.c_str(), mpChoices[choiceIndex()].mpName); }
	virtual void	modify(int step)			{ mValue = (VUUINT8)VuClamp(mValue + step, 0, choiceCount() - 1); }

private:
	int choiceCount()
	{
		int count = 0;
		while ( mpChoices[count].mpName )
			count++;
		return count;
	}
	int choiceIndex()
	{
		for ( int index = 0; index < choiceCount(); index++ )
			if ( mpChoices[index].mValue == mValue )
				return index;
		return 0;
	}
	int								&mValue;
	const VuDevMenu::IntEnumChoice	*mpChoices;
};

class VuDevMenuCallbackNode : public VuDevMenuNode
{
public:
	VuDevMenuCallbackNode(const std::string &strName, VuDevMenu::Callback *pCB, int param) : VuDevMenuNode(strName), mpCB(pCB), mParam(param) {}

	virtual void	print(char *buf, int size)	{ VU_SNPRINTF(buf, size, size - 1, "%s", mstrName.c_str()); }
	virtual void	modify(int step)			{ mpCB->onDevMenu(mParam); }

private:
	VuDevMenu::Callback	*mpCB;
	int					mParam;
};


class VuDevMenuImpl : public VuDevMenu, public VuKeyboard::Callback
{
public:
	VuDevMenuImpl();
	~VuDevMenuImpl();

	virtual bool init() { return true; }

	virtual void		addBool(const char *strPath, bool &value);
	virtual void		addInt(const char *strPath, int &value, int valStep, int valMin, int valMax);
	virtual void		addFloat(const char *strPath, float &value, float valStep, float valMin, float valMax);
	virtual void		addUInt8(const char *strPath, VUUINT8 &value, int valStep, int valMin, int valMax);
	virtual void		addColor3(const char *strPath, VuColor &value);
	virtual void		addColor4(const char *strPath, VuColor &value);
	virtual void		addIntEnum(const char *strPath, int &value, const IntEnumChoice *choices);
	virtual void		addCallback(const char *strPath, VuDevMenu::Callback *pCB, int param);

	virtual void		removeNode(const char *strPath);

private:
	void				draw();

	// keyboard callback
	virtual void		onKeyDown(VUUINT32 key);

	bool				mbVisible;
	VuDevMenuFolderNode	mRoot;
	VuDevMenuNode		*mpCurNode;
};


// the interface
IMPLEMENT_SYSTEM_COMPONENT(VuDevMenu, VuDevMenuImpl);


//*****************************************************************************
VuDevMenuImpl::VuDevMenuImpl():
	mbVisible(false),
	mRoot("Root"),
	mpCurNode(VUNULL)
{
	VuKeyboard::IF()->addCallback(this);

	// expand root folder
	mRoot.modify(1);

	// start rendering
	VuDrawManager::IF()->registerHandler(this, &VuDevMenuImpl::draw);
}

//*****************************************************************************
VuDevMenuImpl::~VuDevMenuImpl()
{
	// stop rendering
	VuDrawManager::IF()->unregisterHandler(this);

	VuKeyboard::IF()->removeCallback(this);
}

//*****************************************************************************
void VuDevMenuImpl::addBool(const char *strPath, bool &value)
{
	mRoot.insert(VuFileUtil::getPath(strPath), new VuDevMenuBoolNode(VuFileUtil::getNameExt(strPath), value));
}

//*****************************************************************************
void VuDevMenuImpl::addInt(const char *strPath, int &value, int valStep, int valMin, int valMax)
{
	mRoot.insert(VuFileUtil::getPath(strPath), new VuDevMenuIntNode(VuFileUtil::getNameExt(strPath), value, valStep, valMin, valMax));
}

//*****************************************************************************
void VuDevMenuImpl::addFloat(const char *strPath, float &value, float valStep, float valMin, float valMax)
{
	mRoot.insert(VuFileUtil::getPath(strPath), new VuDevMenuFloatNode(VuFileUtil::getNameExt(strPath), value, valStep, valMin, valMax));
}

//*****************************************************************************
void VuDevMenuImpl::addUInt8(const char *strPath, VUUINT8 &value, int valStep, int valMin, int valMax)
{
	mRoot.insert(VuFileUtil::getPath(strPath), new VuDevMenuUInt8Node(VuFileUtil::getNameExt(strPath), value, valStep, valMin, valMax));
}

//*****************************************************************************
void VuDevMenuImpl::addColor3(const char *strPath, VuColor &value)
{
	VuDevMenu::IF()->addUInt8((std::string(strPath) + "/R").c_str(), value.mR);
	VuDevMenu::IF()->addUInt8((std::string(strPath) + "/G").c_str(), value.mG);
	VuDevMenu::IF()->addUInt8((std::string(strPath) + "/B").c_str(), value.mB);
}

//*****************************************************************************
void VuDevMenuImpl::addColor4(const char *strPath, VuColor &value)
{
	addColor3(strPath, value);
	VuDevMenu::IF()->addUInt8((std::string(strPath) + "/A").c_str(), value.mA);
}

//*****************************************************************************
void VuDevMenuImpl::addIntEnum(const char *strPath, int &value, const IntEnumChoice *choices)
{
	mRoot.insert(VuFileUtil::getPath(strPath), new VuDevMenuIntEnumNode(VuFileUtil::getNameExt(strPath), value, choices));
}

//*****************************************************************************
void VuDevMenuImpl::addCallback(const char *strPath, VuDevMenu::Callback *pCB, int param)
{
	mRoot.insert(VuFileUtil::getPath(strPath), new VuDevMenuCallbackNode(VuFileUtil::getNameExt(strPath), pCB, param));
}

//*****************************************************************************
void VuDevMenuImpl::removeNode(const char *strPath)
{
	mRoot.remove(VuFileUtil::getPath(strPath), VuFileUtil::getNameExt(strPath));
	mpCurNode = VUNULL;
}

//*****************************************************************************
void VuDevMenuImpl::draw()
{
	if ( !mbVisible )
		return;

	VuGfxSort::IF()->setFullScreenLayer(VuGfxSort::FSL_DEV_MENU);

	if ( !mpCurNode )
		mpCurNode = mRoot.nextVisible();

	VuGfxUtil *pGfxUtil = VuGfxUtil::IF();
	VuFontDraw *pFontDraw = pGfxUtil->fontDraw();
	VuFontDrawParams params;
	float textHeight = params.mSize/FONT_DRAW_SCALE_Y;

	// draw background
	pGfxUtil->drawFilledRectangle2d(GFX_SORT_DEPTH_STEP, VuColor(0, 0, 0, 192));

	// draw nodes
	char buf[256];
	memset(buf, 0, sizeof(buf));
	float fPosY = 0.05f;
	for ( VuDevMenuNode *pNode = mRoot.nextVisible(); pNode && fPosY < 1.0f; pNode = pNode->nextVisible() )
	{
		pNode->print(buf, sizeof(buf));

		params.mColor = pNode == mpCurNode ? VuColor(255,255,64) : VuColor(224,224,224);

		float fPosX = 0.05f + pNode->mDepth*textHeight;
		pFontDraw->drawString(0, VuDev::IF()->getFont(), buf, params, VuRect(fPosX, fPosY, 0.0f, 0.0f), 0);

		fPosY += textHeight;
	}
}

//*****************************************************************************
void VuDevMenuImpl::onKeyDown(VUUINT32 key)
{
	if ( mbVisible && mpCurNode )
	{
		int step = 1;
		if ( VuKeyboard::IF()->isKeyDown(VUKEY_SHIFT) )
			step *= 10;
		if ( VuKeyboard::IF()->isKeyDown(VUKEY_ALT) )
			step *= 10;

		if ( key == VUKEY_LEFT )
		{
			mpCurNode->modify(-step);
		}
		else if ( key == VUKEY_RIGHT )
		{
			mpCurNode->modify(+step);
		}
		else if ( key == VUKEY_UP )
		{
			for ( VuDevMenuNode *pNode = &mRoot; pNode; pNode = pNode->nextVisible() )
			{
				if ( pNode->nextVisible() == mpCurNode )
				{
					mpCurNode = pNode;
					break;
				}
			}
			if ( mpCurNode == &mRoot )
			{
				while ( mpCurNode->nextVisible() )
					mpCurNode = mpCurNode->nextVisible();
			}
		}
		else if ( key == VUKEY_DOWN )
		{
			if ( VuDevMenuNode *pNode = mpCurNode->nextVisible() )
				mpCurNode = pNode;
			else
				mpCurNode = mRoot.nextVisible();
		}
	}

	if ( key == VUKEY_SLASH )
	{
		mbVisible = !mbVisible;
		VuKeyboard::IF()->setCallbackPriority(this, mbVisible ? 65535 : 0);
	}
}

//*****************************************************************************
VuDevMenuNode *VuDevMenuNode::nextVisible()
{
	if ( mpNextSibling )
		return mpNextSibling;

	for ( VuDevMenuNode *pNode = mpParent; pNode; pNode = pNode->mpParent )
		if ( pNode->mpNextSibling )
			return pNode->mpNextSibling;

	return VUNULL;
}

//*****************************************************************************
std::string VuDevMenuNode::getRoot(const std::string &path)
{
	size_t slash = (int)path.find_first_of('/');
	if ( slash != std::string::npos )
		return path.substr(0, slash);
	return path;
}

//*****************************************************************************
std::string VuDevMenuNode::subtractRoot(const std::string &path)
{
	size_t slash = (int)path.find_first_of('/');
	if ( slash != std::string::npos )
		return path.substr(slash + 1);
	return "";
}

//*****************************************************************************
VuDevMenuFolderNode::~VuDevMenuFolderNode()
{
	VuDevMenuNode *pNode = mpFirstChild;
	while ( pNode )
	{
		VuDevMenuNode *pNextNode =  pNode->mpNextSibling;
		delete pNode;
		pNode = pNextNode;
	}
}

//*****************************************************************************
VuDevMenuNode *VuDevMenuFolderNode::nextVisible()
{
	if ( mExpanded && mpFirstChild )
		return mpFirstChild;

	if ( mpNextSibling )
		return mpNextSibling;

	for ( VuDevMenuNode *pNode = mpParent; pNode; pNode = pNode->mpParent )
		if ( pNode->mpNextSibling )
			return pNode->mpNextSibling;

	return VUNULL;
}

//*****************************************************************************
void VuDevMenuFolderNode::insert(std::string path, VuDevMenuNode *pNode)
{
	if ( path.length() )
	{
		std::string child = getRoot(path);
		path = subtractRoot(path);

		// find or create/insert child
		VuDevMenuNode *pChild = findChild(child);
		if ( !pChild )
		{
			pChild = new VuDevMenuFolderNode(child.c_str());
			insert("", pChild);
		}

		pChild->insert(path, pNode);
	}
	else
	{
		VUASSERT(findChild(pNode->mstrName) == VUNULL, "VuDevMenuFolderNode::insert() duplicate entry");

		// insert here
		pNode->mDepth = mDepth + 1;
		pNode->mpParent = this;
		pNode->mpPrevSibling = mpLastChild;
		pNode->mpNextSibling = VUNULL;

		if ( !mpFirstChild )
			mpFirstChild = pNode;

		if ( mpLastChild )
			mpLastChild->mpNextSibling = pNode;
		mpLastChild = pNode;
	}
}

//*****************************************************************************
void VuDevMenuFolderNode::remove(std::string path, const std::string &name)
{
	if ( path.length() )
	{
		std::string child = getRoot(path);
		path = subtractRoot(path);

		// find or create/insert child
		VuDevMenuNode *pChild = findChild(child);
		if ( pChild )
			pChild->remove(path, name);
	}
	else
	{
		VuDevMenuNode *pNode = findChild(name);
		if ( pNode )
		{
			// remove from list
			if ( pNode->mpPrevSibling )
				pNode->mpPrevSibling->mpNextSibling = pNode->mpNextSibling;
			if ( pNode->mpNextSibling )
				pNode->mpNextSibling->mpPrevSibling = pNode->mpPrevSibling;
			if ( pNode == mpFirstChild )
				mpFirstChild = pNode->mpNextSibling;
			if ( pNode == mpLastChild )
				mpLastChild = pNode->mpPrevSibling;

			delete pNode;
		}
	}
}

//*****************************************************************************
VuDevMenuNode *VuDevMenuFolderNode::findChild(const std::string &strName)
{
	for ( VuDevMenuNode *pNode = mpFirstChild; pNode; pNode = pNode->mpNextSibling )
		if ( pNode->mstrName == strName )
			return pNode;

	return VUNULL;
}

#endif // VU_DISABLE_DEV_MENU
